//
// main.cpp
// Created by Ashwin Paudel on 2022-03-20.
//
// =============================================================================
//
// Copyright (c) 2022, Drast Programming Language Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.
//
// =============================================================================
//
// Contributed by:
//  - Ashwin Paudel <ashwonixer123@gmail.com>
//
// =============================================================================

#include "compiler/Lexer/Lexer.h"
#include "compiler/Parser/Parser.h"
#include "compiler/SemanticAnalyzer/SemanticAnalyzer.h"
#include <iostream>

int main(int argc, const char *argv[]) {
    std::string file_name = argv[1];

    Error error = {.file_name = argv[1], .buffer = file_name};
    drast::lexer::Lexer lexer(file_name, error);
    //    lexer.lex();

    std::cout << "----------------------------------" << std::endl;

    drast::parser::Parser parser(lexer, error);
    auto ast_tree = parser.parse();

    std::cout << "----------------------------------" << std::endl;

    drast::semanticAnalyzer::SemanticAnalyzer semantic_analyzer(ast_tree,
                                                                error);
    semantic_analyzer.analyze();

    std::cout << "----------------------------------" << std::endl;

    return 0;
}
