#pragma once

#include <string>

inline int audit_label_score(const std::string& label, int base) {
    return base + static_cast<int>(label.size());
}
