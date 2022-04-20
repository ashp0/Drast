//
// Created by Ashwin Paudel on 2022-04-17.
//

#include "Error.h"

static std::string getNthLine(const std::string &str, size_t lines) {
    std::stringstream ss;
    ss << str;
    std::string line;
    for (size_t i = 0; i < lines; i++) {
        std::getline(ss, line);
    }
    return line;
}

static void print_line(const std::string &str, const Location &location) {
    std::cout << getNthLine(str, location.line) << '\n';

    for (int i = 0; i < location.column - 1; i++) {
        std::cout << "-";
    }
    std::cout << "^" << '\n';
}

void Error::addError(const char *msg, Location location) {
    auto pair = std::make_pair(msg, location);

    this->error_messages.emplace_back(pair);
}

void Error::addError(const std::string &msg, Location location) {
    auto pair = std::make_pair(msg, location);

    this->error_messages.emplace_back(pair);
}

void Error::addWarning(const char *msg, Location location) {
    auto pair = std::make_pair(msg, location);

    this->warning_messages.emplace_back(pair);
}

void Error::addWarning(const std::string &msg, Location location) {
    auto pair = std::make_pair(msg, location);

    this->warning_messages.emplace_back(pair);
}

void Error::displayMessages() {
    for (auto const &warning_message : warning_messages) {
        displayMessage(warning_message, true);
        std::cout << '\n';
    }

    for (auto const &error_message : error_messages) {
        displayMessage(error_message, false);
        std::cout << '\n';
    }

    exit(-1);
}

void Error::displayMessage(const std::pair<std::string, Location> &message,
                           bool is_warning) {
    if (is_warning) {
        std::cout << "Warning: ";
    } else {
        std::cout << "Error: ";
    }
    std::cout << file_name << ":" << message.second.line << ":"
              << message.second.column << ": " << message.first << '\n';

    print_line(buffer, message.second);
}