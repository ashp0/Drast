//
// Created by Ashwin Paudel on 2022-06-02.
//

#ifndef DRAST_REPORT_H
#define DRAST_REPORT_H

#include <string>
#include "Location.h"
#include <exception>
#include <optional>
#include "../Config/Config.h"

class Report final : public std::exception {
protected:
    std::string error_message;
    std::optional<Location> location = std::nullopt;
public:
    explicit Report(const char *message, Location location)
            : location(location) {
        error_message += Config::shared()->filename;
        error_message += ":";
        error_message += std::to_string(location.line);
        error_message += ":";
        error_message += std::to_string(location.column);
        error_message += ": ";
        error_message += message;
    }

    explicit Report(const char *message) {
        error_message = message;
    }

    explicit Report(const std::string &message, Location location)
            : location(location) {
        error_message += location.toString();
        error_message += ": ";
        error_message += message;
    }

    ~Report() noexcept override = default;

    [[nodiscard]] const char *what() const noexcept final {
        return error_message.c_str();
    }
};

#endif //DRAST_REPORT_H
