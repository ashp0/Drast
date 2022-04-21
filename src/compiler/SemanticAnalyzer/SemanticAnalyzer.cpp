//
// Created by Ashwin Paudel on 2022-04-10.
//

#include "SemanticAnalyzer.h"

namespace drast::semanticAnalyzer {

void SemanticAnalyzer::analyze() {
    analyzeCompoundStatement(dynamic_cast<AST::Compound *>(root));
}

void SemanticAnalyzer::analyzeCompoundStatement(AST::Compound *compound) {
    for (auto &statement : compound->statements) {
        analyzeStatement(statement);
    }

    auto dup = compound->searchForDuplicateVariables();
    if (dup) {
        std::cout << "Duplicate variable found: " << dup->first << std::endl;
        exit(1);
    }
}

void SemanticAnalyzer::analyzeStatement(AST::Node *statement) {
    // TODO: Check if there are some variables that are not allowed to be top
    // level
    switch (statement->type) {
    case AST::ASTType::VARIABLE_DECLARATION:
        analyzeVariableDeclaration(
            dynamic_cast<AST::VariableDeclaration *>(statement));
        break;
    default:
        std::cout << "Unable to analyze statement: " << std::endl;
        exit(1);
    }
}

void SemanticAnalyzer::analyzeFunctionDeclaration(
    AST::FunctionDeclaration *function) {}

void SemanticAnalyzer::analyzeVariableDeclaration(
    AST::VariableDeclaration *variable) {
    std::cout << "Analyzing variable declaration" << '\n';

    currentCompound()->variables.emplace_back(variable->name, variable);
}

lexer::TokenType SemanticAnalyzer::analyzeExpression(AST::Node *expression) {
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

lexer::TokenType
SemanticAnalyzer::analyzeBinaryExpression(AST::BinaryExpression *expression) {
    auto left_type = analyzeExpression(expression->left);
    auto right_type = analyzeExpression(expression->right);
    if (left_type == right_type) {
        return left_type;
    } else {
        std::cout << "Invalid Expression" << '\n';
        exit(-1);
    }
}

lexer::TokenType SemanticAnalyzer::analyzeGroupingExpression(
    AST::GroupingExpression *expression) {
    return analyzeExpression(expression->expr);
}

lexer::TokenType
SemanticAnalyzer::analyzeUnaryExpression(AST::UnaryExpression *expression) {
    auto expr_type = analyzeExpression(expression->expr);
    if (expr_type != lexer::TokenType::V_INT) {
        std::cout << "Operand must be of type integer" << '\n';
        exit(-1);
    }

    return expr_type;
}

lexer::TokenType
SemanticAnalyzer::analyzeLiteralExpression(AST::LiteralExpression *expression) {
    return expression->literal_type;
}

} // namespace drast::semanticAnalyzer
