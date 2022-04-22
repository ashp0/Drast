//
// SemanticAnalyzer.cpp
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

#include "SemanticAnalyzer.h"

namespace drast::semanticAnalyzer {

void SemanticAnalyzer::analyze() {
    analyzeCompoundStatement(dynamic_cast<AST::Compound *>(root));

    if (encountered_error) {
        error.displayMessages();
    }
}

void SemanticAnalyzer::analyzeCompoundStatement(AST::Compound *compound) {
    current_compound_index += 1;
    compound_stmts.push_back(compound);
    for (auto &statement : compound->statements) {
        analyzeStatement(statement);
    }

    auto dup = compound->searchForDuplicateVariables();
    if (dup) {
        throwError("Duplicate Variable Found: " + std::string(dup->first),
                   dup->second->location);
    }
}

void SemanticAnalyzer::analyzeStatement(AST::Node *statement) {
    // TODO: Check if there are some tokens that are not allowed to be top level
    this->current_statement = statement;
    switch (statement->type) {
    case AST::ASTType::VARIABLE_DECLARATION:
        analyzeVariableDeclaration(
            dynamic_cast<AST::VariableDeclaration *>(statement));
        break;
    case AST::ASTType::BINARY_EXPRESSION:
        analyzeBinaryExpression(
            dynamic_cast<AST::BinaryExpression *>(statement));
        break;
    default:
        //        std::cout << "Unable to analyze statement: " <<
        //        statement->toString() << std::endl;
        break;
    }
}

void SemanticAnalyzer::analyzeFunctionDeclaration(
    AST::FunctionDeclaration *function) {}

void SemanticAnalyzer::analyzeVariableDeclaration(
    AST::VariableDeclaration *variable) {

    auto type = analyzeExpression(variable->value.value());
    variable->sema_type = type;

    currentCompound()->variables.emplace_back(variable->name, variable);
}

AST::SemaTypes SemanticAnalyzer::analyzeExpression(AST::Node *expression) {
    switch (expression->type) {
    case AST::ASTType::BINARY_EXPRESSION:
        return analyzeBinaryExpression(
            dynamic_cast<AST::BinaryExpression *>(expression));
    case AST::ASTType::UNARY_EXPRESSION:
        return analyzeUnaryExpression(
            dynamic_cast<AST::UnaryExpression *>(expression));
    case AST::ASTType::GROUPING_EXPRESSION:
        return analyzeGroupingExpression(
            dynamic_cast<AST::GroupingExpression *>(expression));
    case AST::ASTType::LITERAL_EXPRESSION:
        return analyzeLiteralExpression(
            dynamic_cast<AST::LiteralExpression *>(expression));
    default:
        std::cout << "Error in the compiler :(" << '\n';
        exit(-1);
    }
}

AST::SemaTypes
SemanticAnalyzer::analyzeBinaryExpression(AST::BinaryExpression *expression) {
    auto left_type = analyzeExpression(expression->left);
    auto right_type = analyzeExpression(expression->right);
    if (left_type.type == right_type.type) {
        return left_type;
    } else {
        throwError("Invalid Expression, types are not equal");
        ;
        return left_type;
    }
}

AST::SemaTypes SemanticAnalyzer::analyzeGroupingExpression(
    AST::GroupingExpression *expression) {
    return analyzeExpression(expression->expr);
}

AST::SemaTypes
SemanticAnalyzer::analyzeUnaryExpression(AST::UnaryExpression *expression) {
    auto expr_type = analyzeExpression(expression->expr);
    if (expr_type.type != AST::SemaTypes::INT) {
        throwError("Operand must be of type integer");
    }

    return expr_type;
}

AST::SemaTypes
SemanticAnalyzer::analyzeLiteralExpression(AST::LiteralExpression *expression) {
    if (expression->literal_type == lexer::TokenType::IDENTIFIER) {
        auto variable = findVariable(expression->value);
        return variable->sema_type.value();
    }
    return {AST::TokenTypeToSemaType(expression->literal_type)};
}

AST::VariableDeclaration *
SemanticAnalyzer::findVariable(std::string_view name) {
    for (auto i = current_compound_index; i > 0; i--) {
        auto compound = compound_stmts[i - 1];
        auto var = compound->findVariable(name);
        if (var) {
            return var.value();
        }
    }

    throwError("Undefined Variable Definition: " + std::string(name) + "\n");
    return nullptr;
}

void SemanticAnalyzer::throwError(const std::string &message) {
    encountered_error = true;
    error.addError(message, this->current_statement->location);
}

void SemanticAnalyzer::throwError(const std::string &message,
                                  Location &location) {
    encountered_error = true;
    error.addError(message, location);
}

} // namespace drast::semanticAnalyzer
