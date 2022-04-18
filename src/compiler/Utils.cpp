//
// Created by Ashwin Paudel on 2022-04-09.
//

#include "Utils.h"

bool isHexadecimal(char c) {
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') ||
           (c >= 'A' && c <= 'F') || c == '_';
}

bool isOctal(char c) {
    return c == '0' || c == '1' || c == '2' || c == '3' || c == '4' ||
           c == '5' || c == '6' || c == '7' || c == '_';
}

bool isBinary(char c) { return (c == '0' || c == '1') || c == '_'; }

bool isNumber(char c) { return (c >= '0' && c <= '9') || c == '_' || c == '.'; }

bool isAlphaNumeric(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
           (c >= '0' && c <= '9') || c == '_';
}

void readFile(std::string &file_name, std::string &file_buffer) {
    FILE *fp;
    char *line = nullptr;
    size_t length = 0;
    ssize_t read;

    fp = fopen(file_name.c_str(), "r");
    if (fp == nullptr) {
        fprintf(stderr, "Error: failed to open source file: %s\n",
                file_name.c_str());
        exit(1);
    }

    while ((read = getline(&line, &length, fp)) != -1) {
        file_buffer += line;
    }

    fclose(fp);
    if (line)
        free(line);

    if (file_buffer.empty()) {
        fprintf(stderr, "Error: encountered empty file: %s\n",
                file_buffer.c_str());
    }
}