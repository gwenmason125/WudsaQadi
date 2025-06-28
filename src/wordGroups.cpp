#include "wudsaqadi/wordGroups.hpp"

namespace {

using namespace wudsaqadi;

void findPillarGroupsRecursion(
    std::array<PillarGroupTrie, PILLAR_GROUP_MAX_WIDTH> &pillarGroupTries,
    const std::unordered_set<hash_t> &widthwiseWords,
    std::array<hash_t, PILLAR_GROUP_LENGTH> &hashes,
    PillarGroupSearch &left,
    PillarGroupSearch &middle,
    PillarGroupSearch &right,
    size_t idx,
    int verboseDepth
) {
    bool verbose = idx < verboseDepth; 
    for (const auto &i : left.groups[idx]->children) {
        for (const auto &j : middle.groups[idx]->children) {
            for (const auto &k : right.groups[idx]->children) {
                hash_t currentHash = concatHash(i.first, j.first, left.width);
                currentHash = concatHash(currentHash, k.first, left.width + middle.width);
                if (widthwiseWords.count(currentHash)) {
                    if (verbose) {
                        std::string currentWord = unhashWord(currentHash);
                        std::cout << ANSI_CURSOR_GOTO(idx + 1, 1)
                            << ANSI_BLUE << currentWord.substr(0, left.width)
                            << ANSI_RED << currentWord.substr(left.width, middle.width)
                            << ANSI_BLUE << currentWord.substr(left.width + middle.width, right.width)
                            << std::flush;
                    }

                    hashes[idx] = currentHash;
                    if (idx == PILLAR_GROUP_LENGTH - 1) {
                        pillarGroupTries[left.width + middle.width + right.width - 1].insert(hashes);
                    } else {
                        left.groups[idx + 1] = i.second.get();
                        middle.groups[idx + 1] = j.second.get();
                        right.groups[idx + 1] = k.second.get();
                        findPillarGroupsRecursion(pillarGroupTries, widthwiseWords, hashes, left, middle, right, idx + 1, verboseDepth);
                    }
                }
            }
        }
    }

    if (verbose) {
        std::cout << ANSI_CURSOR_GOTO(idx + 1, 0);
        printColoredDots(left.width, right.width);
    }
}

void findPillarGroups(
    std::array<PillarGroupTrie, PILLAR_GROUP_MAX_WIDTH> &pillarGroupTries,
    const std::unordered_set<hash_t> &widthwiseWords,
    const PillarGroupTrieNode *leftGroup,
    const PillarGroupTrieNode *rightGroup,
    size_t leftWidth,
    size_t rightWidth,
    int verboseDepth
) {
    if (verboseDepth > 0) {
        std::cout << ANSI_CLEAR_SCREEN << ANSI_HIDE_CURSOR << ANSI_CURSOR_GOTO(1, 1);
        for (int i = 0; i < PILLAR_GROUP_LENGTH; ++i) {
            printColoredDots(leftWidth, rightWidth);
            std::cout << "\n";
        }
    }

    PillarGroupSearch left(leftGroup, leftWidth);
    PillarGroupSearch middle(&pillarGroupTries[0].getRoot(), 1);
    PillarGroupSearch right(rightGroup, rightWidth);

    std::array<hash_t, PILLAR_GROUP_LENGTH> hashes;

    findPillarGroupsRecursion(pillarGroupTries, widthwiseWords, hashes, left, middle, right, 0, verboseDepth);
}
}

namespace wudsaqadi {

// Sentinel definition
const PillarGroupTrieNode PILLAR_GROUP_SENTINEL = []() {
    PillarGroupTrieNode sentinel;
    auto *iter = &sentinel;
    for (int i = 0; i < PILLAR_GROUP_LENGTH; ++i) {
        iter->children[0] = std::make_unique<PillarGroupTrieNode>();
        iter = iter->children[0].get();
    }
    return std::move(sentinel);
}();

PillarGroupTrie::PillarGroupTrie(): root(std::make_unique<PillarGroupTrieNode>()) {}

void PillarGroupTrie::insert(const std::vector<std::string> &words) {
    PillarGroupTrieNode *currentNode = root.get();
    for (int i = 0; i < PILLAR_GROUP_LENGTH; ++i) {
        hash_t currentHash = 0;
        for (int j = words.size() - 1; j >= 0; --j) {
            currentHash = concatHash(hashChar(words[j][i]), currentHash, 1);
        }

        currentNode->children.try_emplace(currentHash, std::make_unique<PillarGroupTrieNode>());
        currentNode = currentNode->children[currentHash].get();
    }
}

void PillarGroupTrie::insert(const std::array<hash_t, PILLAR_GROUP_LENGTH> hashes) {
    PillarGroupTrieNode *iter = root.get();
    for (const auto &hash : hashes) {
        iter->children.try_emplace(hash, std::make_unique<PillarGroupTrieNode>());
        iter = iter->children[hash].get();
    }
}

void PillarGroupTrie::writeToFile(std::ofstream &fileOut) const {
    std::array<hash_t, PILLAR_GROUP_LENGTH> hashes = {};
    writeToFileRecursion(fileOut, root.get(), hashes, 0);
}

void PillarGroupTrie::writeToFileRecursion(
    std::ofstream &fileOut,
    const PillarGroupTrieNode *iter,
    std::array<hash_t, PILLAR_GROUP_LENGTH> &hashes,
    size_t idx
) const {
    if (idx == PILLAR_GROUP_LENGTH) {
        for (const std::string &word : unhashPillarGroup(hashes)) {
            fileOut << word << "\n";
        }
        fileOut << "\n";
    } else {
        std::map<hash_t, const PillarGroupTrieNode *> orderedMap;
        for (const auto &child : iter->children) {
            orderedMap[child.first] = child.second.get();
        }
        for (const auto &child : orderedMap) {
            hashes[idx] = child.first;
            writeToFileRecursion(fileOut, child.second, hashes, idx + 1);
        }
    }
}

PillarGroupTrieNode &PillarGroupTrie::getRoot() const { return *root; }

PillarGroupSearch::PillarGroupSearch(const PillarGroupTrieNode *root, size_t width)
    : width(width) {
    groups[0] = root;
}

void findAllPillarGroups(
    std::array<PillarGroupTrie, PILLAR_GROUP_MAX_WIDTH> &pillarGroupTries,
    const std::array<std::unordered_set<hash_t>, PILLAR_GROUP_MAX_WIDTH> &widthwiseWords,
    int verboseDepth
) {
    std::array<std::array<const PillarGroupTrieNode *, PILLAR_GROUP_LENGTH>, 3> wordIters = {};
    const PillarGroupTrieNode *leftGroup = &pillarGroupTries[0].getRoot();
    const PillarGroupTrieNode *rightGroup = &PILLAR_GROUP_SENTINEL;
    findPillarGroups(pillarGroupTries, widthwiseWords[1], leftGroup, rightGroup, 1, 0, verboseDepth);

    leftGroup = &pillarGroupTries[0].getRoot();
    rightGroup = &pillarGroupTries[0].getRoot();
    findPillarGroups(pillarGroupTries, widthwiseWords[2], leftGroup, rightGroup, 1, 1, verboseDepth);

    for (int i = 3; i < PILLAR_GROUP_MAX_WIDTH; ++i) {
        auto &currentWidthwiseWords = widthwiseWords[i];

        leftGroup = &PILLAR_GROUP_SENTINEL;
        rightGroup = &pillarGroupTries[i - 1].getRoot();
        findPillarGroups(pillarGroupTries, currentWidthwiseWords, leftGroup, rightGroup, 0, i, verboseDepth);

        for (int j = 1; j < i; ++j) {
            leftGroup = &pillarGroupTries[j - 1].getRoot();
            rightGroup = &pillarGroupTries[i - j - 1].getRoot();
            findPillarGroups(pillarGroupTries, currentWidthwiseWords, leftGroup, rightGroup, j, i - j, verboseDepth);
        }

        leftGroup = &pillarGroupTries[i - 1].getRoot();
        rightGroup = &PILLAR_GROUP_SENTINEL;
        findPillarGroups(pillarGroupTries, currentWidthwiseWords, leftGroup, rightGroup, i, 0, verboseDepth);
    }
}

std::vector<std::string> unhashPillarGroup(const std::array<hash_t, PILLAR_GROUP_LENGTH> &hashes) {
    std::string firstUnhash = unhashWord(hashes[0]);
    std::vector<std::string> pillarGroups;
    pillarGroups.resize(firstUnhash.length());
    for (int i = 0; i < firstUnhash.length(); ++i) {
        std::string s;
        s.resize(PILLAR_GROUP_LENGTH);
        s[0] = firstUnhash[i];
        pillarGroups[i] = s;
    }
    for (int i = 1; i < PILLAR_GROUP_LENGTH; ++i) {
        std::string s = unhashWord(hashes[i]);
        for (int j = 0; j < firstUnhash.length(); ++j) {
            pillarGroups[j][i] = s[j];
        }
    }

    return pillarGroups;
}

}