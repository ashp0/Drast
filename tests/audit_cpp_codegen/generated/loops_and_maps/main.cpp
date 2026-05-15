#include "runtime/drast_runtime.hpp"

int countWords(const std::vector<std::string>& words);

int countWords(const std::vector<std::string>& words) {
    std::unordered_map<std::string, int> counts;
    for (auto& word : words) {
        if (drast::contains(counts, word)) {
            auto current = drast::map_get(counts, word, 0);
            (counts[word] = current) + 1;
        } else {
            (counts[word] = 1);
        }
    }
    std::vector<std::string> keys = drast::map_keys(counts);
    int total = 0;
    for (auto& key : keys) {
        total += drast::map_get(counts, key, 0);
    }
    return total;
}

int main(int argc, char **argv) {
    drast::setArgs(argc, argv);
    std::vector<std::string> words = {"one", "two", "one", "three"};
    int total = countWords(words);
    for (auto index = 0; index < 3; index += 1) {
        total += index;
    }
    return total;
}

