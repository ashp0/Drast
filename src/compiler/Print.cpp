//
// Created by Ashwin Paudel on 2022-03-24.
//

#include "Print.h"

static std::string getNthLine(std::string &str, size_t lines) {
    std::stringstream ss;
    ss << str;
    std::string line;
    for (size_t i = 0; i < lines; i++) {
        std::getline(ss, line);
    }
    return line;
}

static void print_line(std::string &str, Location &location) {
    std::cout << getNthLine(str, location.line) << std::endl;

    for (int i = 0; i < location.column - 1; i++) {
        std::cout << "-";
    }
    std::cout << "^" << std::endl;
}

int Print::error(const std::string &msg, Location location) {
    // main.drast:1:1: error:
    std::cout << c_error << file_name << ":" << location.line << ":"
              << location.column << ": error: " << c_reset << msg << std::endl;

    print_line(source, location);
    exit(-1);
}

int Print::warning(const std::string &msg, Location location) {
    std::cout << c_warning << file_name << ":" << location.line << ":"
              << location.column << ": warning: " << c_reset << msg
              << std::endl;

    print_line(source, location);
    return 0;
}
