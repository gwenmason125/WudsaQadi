#pragma once

#include <array>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "wudsaqadi/hash.hpp"
#include "wudsaqadi/util.hpp"

namespace wudsaqadi {

constexpr size_t PILLAR_GROUP_MAX_WIDTH = 6;
constexpr size_t PILLAR_GROUP_LENGTH = 8;

class PillarGroupTrieNode {
public:
    std::unordered_map<hash_t, std::unique_ptr<PillarGroupTrieNode>> children;
};

extern const PillarGroupTrieNode PILLAR_GROUP_SENTINEL;

class PillarGroupTrie {
private:
    std::unique_ptr<PillarGroupTrieNode> root;
    void writeToFileRecursion(
        std::ofstream &fileOut,
        const PillarGroupTrieNode *iter,
        std::array<hash_t, PILLAR_GROUP_LENGTH> &hashes,
        size_t idx
    ) const;

public:
    PillarGroupTrie();
    void insert(const std::vector<std::string> &words);
    void insert(const std::array<hash_t, PILLAR_GROUP_LENGTH> hashes);
    void writeToFile(std::ofstream &fileOut) const;
    PillarGroupTrieNode &getRoot() const;
};

struct PillarGroupSearch {
    std::array<const PillarGroupTrieNode *, PILLAR_GROUP_LENGTH> groups;
    const size_t width;

    PillarGroupSearch(const PillarGroupTrieNode *root, size_t width);
};

void findAllPillarGroups(
    std::array<PillarGroupTrie, PILLAR_GROUP_MAX_WIDTH> &pillarGroupTries,
    const std::array<std::unordered_set<hash_t>, PILLAR_GROUP_MAX_WIDTH> &widthwiseWords,
    int verboseDepth
);

std::vector<std::string> unhashPillarGroup(const std::array<hash_t, PILLAR_GROUP_LENGTH> &hashes);

}