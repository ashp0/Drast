//
// Created by Ashwin Paudel on 2022-04-10.
//

#ifndef DRAST_SEMANTIC_ANALYZER_H
#define DRAST_SEMANTIC_ANALYZER_H

#include "AST.h"
#include "Error.h"
#include "LookupTable.h"
#include "Types.h"
#include <iostream>
#include <utility>
#include <vector>

class SemanticAnalyzer {
  private:
    CompoundStatement *main;
    std::vector<CompoundStatement *> compounds = {};
    std::vector<SemanticAnalyzerExpressionTypes> compound_declarations = {};
    size_t compound_index = 0; // the current compound statement

    AST *current_statement;

    Error error;

  public:
    SemanticAnalyzer(CompoundStatement *ast, Error error)
        : main(ast), error(std::move(error)) {}

    void analyze();

  private:
    bool compoundStatement(CompoundStatement *ast);

    bool statement(AST *statement);

    bool importStatement(ImportStatement *ast);

    bool variableDeclaration(VariableDeclaration *ast);

    SemanticAnalyzerExpressionTypes analyzeExpression(AST *expression);

    SemanticAnalyzerExpressionTypes
    analyzeBinaryExpression(BinaryExpression *expression);

    SemanticAnalyzerExpressionTypes
    analyzeGroupingExpression(GroupingExpression *expression);

    SemanticAnalyzerExpressionTypes
    analyzeUnaryExpression(UnaryExpression *expression);

    SemanticAnalyzerExpressionTypes
    analyzeLiteralExpression(LiteralExpression *expression);

    VariableDeclaration *locateVariable(std::string_view name);

    void checkIfTypesMatch(SemanticAnalyzerExpressionTypes lhs,
                           SemanticAnalyzerExpressionTypes rhs);

    void warning(const std::string &message);

    void addError(const std::string &message);

    void addError(const std::string &message, AST *ast);

    CompoundStatement *currentCompound();
};

#endif // DRAST_SEMANTIC_ANALYZER_H
