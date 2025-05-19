#include "wudsaqadi/util.hpp"

namespace wudsaqadi {

std::vector<std::string> readLinesFromFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + path);
    }

    std::vector<std::string> lines;
    std::string line;

    while (std::getline(file, line)) {
        if (!line.empty()) {
            if (line.back() == '\r') {
                line.pop_back();
            }
            lines.push_back(line);
        }
    }

    return lines;
}

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

}