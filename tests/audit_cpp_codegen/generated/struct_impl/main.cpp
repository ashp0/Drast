#include <string>
#include <utility>
#include <vector>

struct Ledger;

struct Ledger {
    std::string title;
    std::vector<int> entries;
  private:
    int cached;
  public:
    Ledger(std::string title, std::vector<int> entries);
    int total() const;
    void rename(const std::string& next);
};

Ledger::Ledger(std::string title, std::vector<int> entries)
    : title(std::move(title)),
      entries(std::move(entries)),
      cached(0) {}

int Ledger::total() const {
    int sum = 0;
    for (const auto& entry : this->entries) {
        sum += entry;
    }
    return sum;
}

void Ledger::rename(const std::string& next) {
    this->title = next;
}


int main() {
    std::vector<int> entries = {5, 7, 11};
    Ledger ledger = Ledger("q2", std::move(entries));
    ledger.rename("q3");
    return ledger.total();
}

