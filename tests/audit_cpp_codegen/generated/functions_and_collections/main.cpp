#include "runtime/drast_runtime.hpp"

struct Sample;

struct Sample {
    Sample() = default;
    Sample(std::string name, std::vector<int> values) : name(std::move(name)), values(std::move(values)) {}
    std::string name;
    std::vector<int> values;
};

int scoreName(const std::string& name);
int sumValues(const std::vector<int>& values);
std::string combine(const Sample& sample, const std::string& suffix);

int scoreName(const std::string& name) {
    return name.size();
}

int sumValues(const std::vector<int>& values) {
    int total = 0;
    for (auto& value : values) {
        total += value;
    }
    return total;
}

std::string combine(const Sample& sample, const std::string& suffix) {
    std::string text = sample.name;
    text += suffix;
    return text;
}

int main(int argc, char **argv) {
    drast::setArgs(argc, argv);
    std::vector<int> values = {1, 2, 3, 4};
    Sample sample = Sample("alpha", values);
    int total = sumValues(values);
    total += scoreName(sample.name);
    std::string label = combine(sample, "-done");
    return total + label.size();
}

