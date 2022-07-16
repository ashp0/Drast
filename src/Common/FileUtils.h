//
// Created by Ashwin Paudel on 2022-07-15.
//

#ifndef DRAST_FILEUTILS_H
#define DRAST_FILEUTILS_H

#include <string>
#include <fstream>

inline bool fileExists(const char *name) {
    std::ifstream infile(name);
    return infile.good();
}

#endif //DRAST_FILEUTILS_H
