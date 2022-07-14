//
// Created by Ashwin Paudel on 2022-06-03.
//

#include "Config.h"

Config *instance;

Config *Config::shared() {
    if (instance == nullptr) {
        instance = new Config();
    }
    return instance;
}