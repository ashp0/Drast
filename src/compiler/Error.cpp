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
    std::cout << getNthLine(str, location.line) << std::endl;

    for (int i = 0; i < location.column - 1; i++) {
        std::cout << "-";
    }
    std::cout << "^" << std::endl;
}

void Error::append(const char *msg, Location location) {
    auto pair = std::make_pair(msg, location);

    this->error_messages.emplace_back(pair);
}

void Error::append(const std::string &msg, Location location) {
    auto pair = std::make_pair(msg, location);

    this->error_messages.emplace_back(pair);
}

void Error::displayErrors() {
    for (auto const &error_message : error_messages) {
        displayError(error_message);
        std::cout << std::endl;
    }

    exit(-1);
}

void Error::displayError(
    const std::pair<std::string, Location> &error_message) {
    std::cout << "error:" << file_name << ":" << error_message.second.line
              << ":" << error_message.second.column << ": "
              << error_message.first << std::endl;

    print_line(buffer, error_message.second);
}