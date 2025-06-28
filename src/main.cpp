#include <chrono>
#include "wudsaqadi/util.hpp"
#include "wudsaqadi/hash.hpp"
#include "wudsaqadi/wordGroups.hpp"

using namespace wudsaqadi;

namespace {
    void doPillarGroupRoutine() {
        std::vector<std::string> words = readLinesFromFile("resources/dictionaries/NWL23.txt");

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
        findAllPillarGroups(pillarGroupTries, widthwiseWords, 0);
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = end - start;

        auto hours = std::chrono::duration_cast<std::chrono::hours>(duration);
        auto minutes = std::chrono::duration_cast<std::chrono::minutes>(duration - hours);
        auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration - hours - minutes);

        std::cout << "Completed search in " << hours.count() << "h" << minutes.count() << "m" << seconds.count() << "s\n";

        std::ofstream file("output/out.txt");
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open output file");
        }

        for (int i = 1; i < PILLAR_GROUP_MAX_WIDTH; ++i) {
            pillarGroupTries[i].writeToFile(file);
        }
}

constexpr size_t BOARD_HEIGHT = 15;
constexpr size_t BOARD_WIDTH = 15;

struct Tile {

};

class Board {
public:
    std::array<std::array<Tile, BOARD_WIDTH>, BOARD_HEIGHT> board;
    Board() {

    }
};

int main() {
    return 0;
}