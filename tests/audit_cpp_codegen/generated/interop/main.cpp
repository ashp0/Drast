#include "runtime/drast_runtime.hpp"
#include "header/native_audit.hpp"


int main(int argc, char **argv) {
    drast::setArgs(argc, argv);
    std::string label = "native";
    int base = 10;
    return audit_label_score(label, base);
}

