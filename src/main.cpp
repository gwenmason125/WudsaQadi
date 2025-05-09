#include <unordered_map>
#include <map>
#include <unordered_set>
#include <memory>
#include <string>
#include <stdexcept>
#include <fstream>
#include <vector>
#include <array>
#include <iostream>
#include <cmath>
#include <limits>
#include <cstdint>
#include <chrono>
#include <algorithm>

#define ANSI_CLEAR_SCREEN           "\033[2J"
#define ANSI_HIDE_CURSOR            "\033[?25l"
#define ANSI_CURSOR_GOTO(row, col)  "\033[" << row << ";" << col << "H"
#define ANSI_BLUE                   "\033[34m"
#define ANSI_RED                    "\033[31m"

constexpr size_t WORD_MIN_LENGTH = 2;
constexpr size_t WORD_MAX_LENGTH = 15;

constexpr size_t PILLAR_GROUP_MAX_WIDTH = 6;
constexpr size_t PILLAR_GROUP_LENGTH = 8;

constexpr size_t HASH_ALPHABET_SIZE = 26;
constexpr char HASH_ALPHABET_START = 'A';

using hash_t = uint32_t;

constexpr size_t HASH_MAX_LENGTH = []() {
    hash_t remaining = std::numeric_limits<hash_t>::max();
    hash_t current_letter_value = 1;
    size_t length = 0;

    while (remaining / HASH_ALPHABET_SIZE >= current_letter_value) {
        if (length != 0) {
            remaining -= current_letter_value;
        }
        ++length;
        current_letter_value *= HASH_ALPHABET_SIZE;
    }

    return length;
}();

constexpr std::array<hash_t, HASH_MAX_LENGTH + 1> HASH_BASE_POWERS = []() {
    std::array<hash_t, HASH_MAX_LENGTH + 1> powers = {};
    powers[0] = 1;
    for (int i = 1; i <= HASH_MAX_LENGTH; ++i) {
        powers[i] = powers[i - 1] * HASH_ALPHABET_SIZE;
    }
    return powers;
}();

hash_t hashWord(const std::string &s) {
    hash_t hash = 0;
    for (int i = s.length() - 1; i >= 0; --i) {
        hash = hash * HASH_ALPHABET_SIZE + (s[i] - HASH_ALPHABET_START + 1);
    }
    return hash;
}

std::string unhashWord(hash_t hash) {
    std::string s;
    while (hash > 0) {
        hash -= 1;
        size_t rem = hash % HASH_ALPHABET_SIZE;
        char c = static_cast<char>(HASH_ALPHABET_START + rem);
        s.push_back(c);
        hash /= HASH_ALPHABET_SIZE;
    }
    return s;
}

constexpr hash_t hashChar(char c) {
    return c - HASH_ALPHABET_START + 1;
}

constexpr hash_t concatHash(hash_t left, hash_t right, int l_length) {
    return left + right * HASH_BASE_POWERS[l_length];
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

class PillarGroupTrieNode {
public:
    std::unordered_map<hash_t, std::unique_ptr<PillarGroupTrieNode>> children;
};

static const PillarGroupTrieNode PILLAR_GROUP_SENTINEL = []() {
    PillarGroupTrieNode sentinel;
    auto *iter = &sentinel;
    for (int i = 0; i < PILLAR_GROUP_LENGTH; ++i) {
        iter->children[0] = std::make_unique<PillarGroupTrieNode>();
        iter = iter->children[0].get();
    }
    return std::move(sentinel);
}();

class PillarGroupTrie {
private:
    std::unique_ptr<PillarGroupTrieNode> root;

    void writeToFileRecursion(
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
        }
        else {
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

public:
    PillarGroupTrie(size_t length = 0, size_t width = 0):
        root(std::make_unique<PillarGroupTrieNode>())
    {}

    void insert(const std::vector<std::string> &words) {
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

    void insert(const std::array<hash_t, PILLAR_GROUP_LENGTH> hashes) {
        PillarGroupTrieNode *iter = root.get();
        for (const auto &hash : hashes) {
            iter->children.try_emplace(hash, std::make_unique<PillarGroupTrieNode>());
            iter = iter->children[hash].get();
        }
    }

    void writeToFile(std::ofstream &fileOut) const {
        std::array<hash_t, PILLAR_GROUP_LENGTH> hashes = {};
        writeToFileRecursion(fileOut, root.get(), hashes, 0);
    }

    PillarGroupTrieNode &getRoot() const { return *root; }
};

std::vector<std::string> readLinesFromFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + path);
    }

    std::vector<std::string> lines;
    std::string line;

    while (std::getline(file, line)) {
        if (!line.empty()) {
            lines.push_back(line);
        }
    }

    return lines;
}

struct PillarGroupSearch {
    std::array<const PillarGroupTrieNode *, PILLAR_GROUP_LENGTH> groups;
    const size_t width;
    PillarGroupSearch(const PillarGroupTrieNode *root, size_t width): width(width) {groups[0] = root;}
};

void printColoredDots(size_t leftWidth, size_t rightWidth) {
    std::cout << ANSI_BLUE;
    for (int i = 0; i < leftWidth; ++i) {
        std::cout << ".";
    }
    std::cout << ANSI_RED << "." << ANSI_BLUE;
    for (int i = 0; i < rightWidth; ++i) {
        std::cout << ".";
    }
}

void findPillarGroupsRecursion(
    std::array<PillarGroupTrie, PILLAR_GROUP_MAX_WIDTH> &pillarGroupTries,
    const std::unordered_set<hash_t> &widthwiseWords,
    std::array<hash_t, PILLAR_GROUP_LENGTH> &hashes,
    PillarGroupSearch &left,
    PillarGroupSearch &middle,
    PillarGroupSearch &right,
    size_t idx,
    bool verbose
) {
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
                        findPillarGroupsRecursion(pillarGroupTries, widthwiseWords, hashes, left, middle, right, idx + 1, verbose);
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
    bool verbose
) {
    if (verbose) {
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

    findPillarGroupsRecursion(pillarGroupTries, widthwiseWords, hashes, left, middle, right, 0, verbose);
}

void findAllPillarGroups(
    std::array<PillarGroupTrie, PILLAR_GROUP_MAX_WIDTH> &pillarGroupTries,
    const std::array<std::unordered_set<hash_t>, PILLAR_GROUP_MAX_WIDTH> &widthwiseWords,
    bool verbose = true
) {
    std::array<std::array<const PillarGroupTrieNode *, PILLAR_GROUP_LENGTH>, 3> wordIters = {};
    const PillarGroupTrieNode *leftGroup = &pillarGroupTries[0].getRoot();
    const PillarGroupTrieNode *rightGroup = &PILLAR_GROUP_SENTINEL;
    findPillarGroups(pillarGroupTries, widthwiseWords[1], leftGroup, rightGroup, 1, 0, verbose);

    leftGroup = &pillarGroupTries[0].getRoot();
    rightGroup = &pillarGroupTries[0].getRoot();
    findPillarGroups(pillarGroupTries, widthwiseWords[2], leftGroup, rightGroup, 1, 1, verbose);

    for (int i = 3; i < PILLAR_GROUP_MAX_WIDTH; ++i) {
        auto &currentWidthwiseWords = widthwiseWords[i];

        leftGroup = &PILLAR_GROUP_SENTINEL;
        rightGroup = &pillarGroupTries[i - 1].getRoot();
        findPillarGroups(pillarGroupTries, currentWidthwiseWords, leftGroup, rightGroup, 0, i, verbose);

        for (int j = 1; j < i; ++j) {
            leftGroup = &pillarGroupTries[j - 1].getRoot();
            rightGroup = &pillarGroupTries[i - j - 1].getRoot();
            findPillarGroups(pillarGroupTries, currentWidthwiseWords, leftGroup, rightGroup, j, i - j, verbose);
        }

        leftGroup = &pillarGroupTries[i - 1].getRoot();
        rightGroup = &PILLAR_GROUP_SENTINEL;
        findPillarGroups(pillarGroupTries, currentWidthwiseWords, leftGroup, rightGroup, i, 0, verbose);
    }
}

int main() {
    std::vector<std::string> words = readLinesFromFile("../data/dictionaries/NWL23.txt");

    std::array<PillarGroupTrie, PILLAR_GROUP_MAX_WIDTH> pillarGroupTries = {};

    std::array<std::unordered_set<hash_t>, PILLAR_GROUP_MAX_WIDTH> widthwiseWords;

    for (const auto &word : words) {
        size_t currentLength = word.length();
        if (currentLength == 8) {
            pillarGroupTries[0].insert({word});
        }
        if (currentLength <= PILLAR_GROUP_MAX_WIDTH) {
            widthwiseWords[currentLength - 1].insert(hashWord(word));
        }
    }

    auto start = std::chrono::high_resolution_clock::now();
    findAllPillarGroups(pillarGroupTries, widthwiseWords);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = end - start;

    auto hours = std::chrono::duration_cast<std::chrono::hours>(duration);
    auto minutes = std::chrono::duration_cast<std::chrono::minutes>(duration - hours);
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration - hours - minutes);

    std::cout << "Completed search in " << hours.count() << "h" << minutes.count() << "m" << seconds.count() << "s\n";

    std::ofstream file("./out.txt");
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open output file");
    }

    for (int i = 1; i < PILLAR_GROUP_MAX_WIDTH; ++i) {
        pillarGroupTries[i].writeToFile(file);
    }

    return 0;
}