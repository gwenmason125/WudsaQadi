#pragma once

#include <array>
#include <string>
#include <vector>
#include <limits>
#include <cstdint>

namespace wudsaqadi {

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

hash_t hashWord(const std::string &s);
std::string unhashWord(hash_t hash);
constexpr hash_t hashChar(char c);
constexpr hash_t concatHash(hash_t left, hash_t right, int l_length);

inline constexpr hash_t hashChar(char c) {
    return c - HASH_ALPHABET_START + 1;
}

inline constexpr hash_t concatHash(hash_t left, hash_t right, int l_length) {
    return left + right * HASH_BASE_POWERS[l_length];
}

} // namespace wudsaqadi