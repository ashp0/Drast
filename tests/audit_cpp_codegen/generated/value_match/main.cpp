#include "runtime/drast_runtime.hpp"

int rankStatus(const std::string& status);

int rankStatus(const std::string& status) {
    {
        const auto& _match = status;
        if (_match == "new") {
            return 1;
        }
        else if (_match == "active") {
            return 2;
        }
        else {
            return 0;
        }
    }
}

int main(int argc, char **argv) {
    drast::setArgs(argc, argv);
    std::string status = "active";
    return rankStatus(status);
}

