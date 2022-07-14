//
// Created by Ashwin Paudel on 2022-06-02.
//

#include "ArgsParser.h"

std::unordered_map<std::string, std::string> key_descriptions = {
        {"--help (-h)", "Show a list of all available commands"},
        {"--file (-f)", "The file that will be compiled"},
        {"--lexOutput", "Show a list of tokens"},
};

void ArgsParser::parseArguments() {
    for (int i = 0; i < argc; ++i) {
        if (argv[i][0] == '-') {
            if (argv[i][1] == '-') {
                if (i + 1 < argc && argv[i + 1][0] != '-') {
                    parseValuedLongArgument(argv[i], argv[i + 1]);
                } else {
                    parseLongArgument(argv[i]);
                }
            } else {
                if (strlen(argv[i]) != 2) {
                    std::cout << "Invalid argument: " << argv[i] << std::endl;
                    exit(EXIT_FAILURE);
                }
                if (i + 1 < argc && argv[i + 1][0] != '-') {
                    parseValuedShortArgument(argv[i][1], argv[i + 1]);
                } else {
                    parseShortArgument(argv[i][1]);
                }
            }
        }
    }
}


void ArgsParser::parseValuedShortArgument(const char &c, const std::string &value) {
    if (c == 'f') {
        Config::shared()->filename = value;
    } else {
        std::cout << "Unknown argument: -" << c << std::endl;
        exit(EXIT_FAILURE);
    }
}

void ArgsParser::parseShortArgument(const char &c) {
    switch (c) {
        case 'h':
            showHelp();
            exit(EXIT_SUCCESS);
        case 'f':
            throw Report("Expected file path");
        default:
            std::cout << "Unknown argument: -" << c << std::endl;
            exit(EXIT_FAILURE);
    }
}

void ArgsParser::parseValuedLongArgument(const std::string &key, const std::string &value) {
    if (key == "--file") {
        Config::shared()->filename = value;
    } else {
        std::cout << "Unknown argument: " << key << std::endl;
        exit(EXIT_FAILURE);
    }
}

void ArgsParser::parseLongArgument(const std::string &key) {
    if (key == "--help") {
        showHelp();
        exit(EXIT_SUCCESS);
    } else if (key == "--lexOutput") {
        Config::shared()->showsLexerOutput = true;
    } else {
        std::cout << "Unknown argument: " << key << std::endl;
        exit(EXIT_FAILURE);
    }
}

void ArgsParser::showHelp() {
    std::cout << "Available commands:" << std::endl;
    for (const auto &desc: key_descriptions) {
        std::cout << '\t' << desc.first << ": " << desc.second << std::endl;
    }
}