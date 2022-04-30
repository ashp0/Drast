//
// SemanticAnalyzer.h
// Created by Ashwin Paudel on 2022-04-10.
//
// =============================================================================
///
/// \file
/// This file contains the declaration of the SemanticAnalyzer class. Which is
/// used to check the correctness of the syntax tree.
///
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

#ifndef DRAST_SEMANTIC_ANALYZER_H
#define DRAST_SEMANTIC_ANALYZER_H

#include "../AST/AST.h"
#include "../Common/Error.h"
#include "../Common/LookupTable.h"
#include "../Common/Types.h"
#include <iostream>
#include <utility>
#include <vector>

namespace drast::semanticAnalyzer {

class SemanticAnalyzer {
  private:
    // The base compound statement
    AST::Node *root;

    // All the other compound and the current compound index
    // This will be used to check for variable definitions, it will start from
    // the current index, and then loop until it reaches the root statement
    std::vector<AST::Compound *> compound_stmts;
    size_t current_compound_index = 0;

    // The parent declaration ( struct )
    AST::Node *parent_declaration = nullptr;

    // The current function it is currently on, use std::optional
    std::optional<AST::FunctionDeclaration *> current_function;

    // The current statement it is currently parsing
    AST::Node *current_statement = nullptr;

    // The error module
    Error error;
    bool encountered_error = false;

  public:
    SemanticAnalyzer(AST::Node *root, Error error)
        : root(root), error(std::move(error)) {}

    void analyze();

    void analyzeCompoundStatement(AST::Compound *compound);

    void analyzeStatement(AST::Node *statement);

    void analyzeFunctionDeclaration(AST::FunctionDeclaration *function);

    void analyzeVariableDeclaration(AST::VariableDeclaration *variable);

    AST::SemaTypes analyzeExpression(AST::Node *expression);

    AST::SemaTypes analyzeBinaryExpression(AST::BinaryExpression *expression);

    AST::SemaTypes
    analyzeGroupingExpression(AST::GroupingExpression *expression);

    AST::SemaTypes analyzeUnaryExpression(AST::UnaryExpression *expression);

    AST::SemaTypes analyzeLiteralExpression(AST::LiteralExpression *expression);

    AST::VariableDeclaration *findVariable(std::string_view name);

    // Checks what type of expression it should be
    // If it encounters an error, it will show an error
    lexer::TokenType determineType(lexer::TokenType type,
                                   lexer::TokenType type2);

    constexpr AST::Compound *currentCompound() const {
        return compound_stmts[current_compound_index - 1];
    }

    void throwError(const std::string &message);

    void throwError(const std::string &message, Location &location);
};

} // namespace drast::semanticAnalyzer

#endif // DRAST_SEMANTIC_ANALYZER_H
