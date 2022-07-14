//
// Created by Ashwin Paudel on 2022-06-03.
//

#ifndef DRAST_CONFIG_H
#define DRAST_CONFIG_H

#include <utility>
#include <string>

struct Config {
    std::string filename;
    bool showsLexerOutput = false;

    static Config *shared();
};

#endif //DRAST_CONFIG_H