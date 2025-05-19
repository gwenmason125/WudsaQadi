#include "wudsaqadi/hash.hpp"

namespace wudsaqadi {

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

} // namespace wudsaqadi