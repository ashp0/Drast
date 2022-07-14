//
// Created by Ashwin Paudel on 2022-06-02.
//

#ifndef DRAST_ARGSPARSER_H
#define DRAST_ARGSPARSER_H

#include <unordered_map>
#include <string>
#include <iostream>
#include "../Common/Report.h"
#include "../Config/Config.h"

class ArgsParser {
private:
    int argc;
    char **argv;
public:
    ArgsParser(int argc, char **argv) : argc(argc), argv(argv) {};

    void parseArguments();

    static void parseValuedShortArgument(const char &c, const std::string &value);

    static void parseShortArgument(const char &c);

    static void parseValuedLongArgument(const std::string &key, const std::string &value);

    static void parseLongArgument(const std::string &key);

    static void showHelp();
};


#endif //DRAST_ARGSPARSER_H
