//
// Created by Ashwin Paudel on 2022-04-10.
//

#ifndef DRAST_SEMANTIC_ANALYZER_H
#define DRAST_SEMANTIC_ANALYZER_H

#include "AST.h"
#include "Print.h"
#include <iostream>
#include <vector>

class SemanticAnalyzer {
  private:
    // Everything is just 1 compound statement with a bunch of other compound
    // statements inside
    AST *compound;

  private:
    void check_expression();
};

#endif // DRAST_SEMANTIC_ANALYZER_H
