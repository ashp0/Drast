//
// Created by Ashwin Paudel on 2022-04-10.
//

#ifndef DRAST_SEMANTIC_ANALYZER_H
#define DRAST_SEMANTIC_ANALYZER_H

#include "AST.h"
#include "Error.h"
#include "LookupTable.h"
#include <iostream>
#include <utility>
#include <vector>

class ExpressionType {
  public:
    enum Type {
        NUMBER,
        BOOL,
        STRING,
        NIL,
        TYPE, // a user defined struct
    } type;

    std::string_view type_literal; // if it's a user defined type

    ExpressionType(std::string_view type_literal)
        : type_literal(type_literal), type(TYPE) {}
    ExpressionType(Type type) : type(type) {}
};

class SemanticAnalyzer {
  private:
    CompoundStatement *main;
    std::vector<CompoundStatement *> compounds = {};
    size_t compound_index = 0; // the current compound statement

    AST *current_statement;

    Error error;

  public:
    SemanticAnalyzer(CompoundStatement *ast, Error error)
        : main(ast), error(std::move(error)), current_statement(ast) {}

    void analyze();

  private:
    bool compoundStatement(CompoundStatement *ast);

    bool statement(AST *statement);

    bool importStatement(ImportStatement *ast);

    bool variableDeclaration(VariableDeclaration *ast);

    ExpressionType analyzeExpression(AST *expression);

    ExpressionType analyzeBinaryExpression(BinaryExpression *expression);

    ExpressionType analyzeGroupingExpression(GroupingExpression *expression);

    ExpressionType analyzeUnaryExpression(UnaryExpression *expression);

    ExpressionType analyzeLiteralExpression(LiteralExpression *expression);

    void checkIfTypesMatch(ExpressionType lhs, ExpressionType rhs);

    void warning(const std::string &message);

    void addError(const std::string &message);

    void addError(const std::string &message, AST *ast);

    VariableDeclaration *findVariable(std::string_view name);

    CompoundStatement *currentCompound();
};

#endif // DRAST_SEMANTIC_ANALYZER_H
