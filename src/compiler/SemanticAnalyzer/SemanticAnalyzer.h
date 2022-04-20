//
// Created by Ashwin Paudel on 2022-04-10.
//

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
    std::vector<AST::Node *> compound_stmts;
    size_t current_compound_index = 0;

    // The parent declaration ( struct )
    AST::Node *parent_declaration = nullptr;

    // The current function it is currently on, use std::optional
    std::optional<AST::FunctionDeclaration *> current_function;

    // The current statement it is currently parsing
    AST::Node *current_statement = nullptr;

    // The error module
    Error error;

  public:
    SemanticAnalyzer(AST::CompoundStatement *root, Error error)
        : root(root), error(std::move(error)) {}

    void analyze();

    void analyzeCompoundStatement(AST::CompoundStatement *compound);

    void analyzeStatement(AST::Node *statement);

    void analyzeFunctionDeclaration(AST::FunctionDeclaration *function);

    void analyzeVariableDeclaration(AST::VariableDeclaration *variable);

    lexer::TokenType analyzeExpression(AST::Node *expression);

    lexer::TokenType analyzeBinaryExpression(AST::BinaryExpression *expression);

    lexer::TokenType
    analyzeGroupingExpression(AST::GroupingExpression *expression);

    lexer::TokenType analyzeUnaryExpression(AST::UnaryExpression *expression);

    lexer::TokenType
    analyzeLiteralExpression(AST::LiteralExpression *expression);

    // Checks what type of expression it should be
    // If it encounters an error, it will show an error
    lexer::TokenType determineType(lexer::TokenType type,
                                   lexer::TokenType type2);
};

} // namespace drast::semanticAnalyzer

#endif // DRAST_SEMANTIC_ANALYZER_H
