#include <string>
#include <utility>
#include "header/native_audit.hpp"


int main() {
    std::string label = "native";
    int base = 10;
    return audit_label_score(std::move(label), base);
}

