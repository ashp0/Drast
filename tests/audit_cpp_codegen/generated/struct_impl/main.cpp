#include "runtime/drast_runtime.hpp"

struct Ledger;

struct Ledger {
    std::string title;
    std::vector<int> entries;
  private:
    int cached;
  public:
    Ledger(const std::string& title, const std::vector<int>& entries);
    int total();
    void rename(const std::string& next);
};

Ledger::Ledger(const std::string& title, const std::vector<int>& entries) {
    this->title = title;
    this->entries = entries;
    this->cached = 0;
}

int Ledger::total() {
    int sum = 0;
    for (auto& entry : this->entries) {
        sum += entry;
    }
    return sum;
}

void Ledger::rename(const std::string& next) {
    this->title = next;
}


int main(int argc, char **argv) {
    drast::setArgs(argc, argv);
    std::vector<int> entries = {5, 7, 11};
    Ledger ledger = Ledger("q2", entries);
    ledger.rename("q3");
    return ledger.total();
}

