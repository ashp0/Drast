//
// Created by Ashwin Paudel on 2022-04-10.
//

#include "SemanticAnalyzer.h"

void SemanticAnalyzer::analyze() {
    compoundStatement(this->main);

    error.displayMessages();
}

bool SemanticAnalyzer::compoundStatement(CompoundStatement *ast) {
    this->compounds.push_back(ast);

    for (auto &statement : ast->statements) {
        this->statement(statement);
    }

    auto duplicate = ast->searchForDuplicates();
    if (duplicate) {
        addError("Duplicate declaration: " +
                     std::string(duplicate.value().first),
                 duplicate.value().second);
    }

    return false;
}

bool SemanticAnalyzer::statement(AST *statement) {
    current_statement = statement;
    switch (statement->type) {
    case ASTType::COMPOUND:
        compound_index += 1;
        return compoundStatement(dynamic_cast<CompoundStatement *>(statement));
    case ASTType::IMPORT_STATEMENT:
        return importStatement(dynamic_cast<ImportStatement *>(statement));
    case ASTType::BINARY_EXPRESSION:
        analyzeExpression(statement);
        return true;
    case ASTType::VARIABLE_DECLARATION:
        return variableDeclaration(
            dynamic_cast<VariableDeclaration *>(statement));
    default:
        return false;
    }
}

bool SemanticAnalyzer::importStatement(ImportStatement *ast) {
    //    warning("Import statement cannot be checked");
    return true;
}

bool SemanticAnalyzer::variableDeclaration(VariableDeclaration *ast) {
    if (ast->value) {
        auto value = analyzeExpression(ast->value.value());
    }

    currentCompound()->declarations.emplace_back(ast->name, ast);
    return false;
}

SemanticAnalyzerExpressionTypes
SemanticAnalyzer::analyzeExpression(AST *expression) {
    switch (expression->type) {
    case ASTType::BINARY_EXPRESSION:
        return analyzeBinaryExpression(
            dynamic_cast<BinaryExpression *>(expression));
    case ASTType::UNARY_EXPRESSION:
        return analyzeUnaryExpression(
            dynamic_cast<UnaryExpression *>(expression));
    case ASTType::GROUPING_EXPRESSION:
        return analyzeGroupingExpression(
            dynamic_cast<GroupingExpression *>(expression));
    case ASTType::LITERAL_EXPRESSION:
        return analyzeLiteralExpression(
            dynamic_cast<LiteralExpression *>(expression));
    default:
        std::cout << "Invalid AST type" << std::endl;
        exit(-1);
    }
}

SemanticAnalyzerExpressionTypes
SemanticAnalyzer::analyzeBinaryExpression(BinaryExpression *expression) {
    auto left_type = analyzeExpression(expression->left);
    auto right_type = analyzeExpression(expression->right);
    checkIfTypesMatch(left_type, right_type);

    return left_type;
}

SemanticAnalyzerExpressionTypes
SemanticAnalyzer::analyzeGroupingExpression(GroupingExpression *expression) {
    return analyzeExpression(expression->expr);
}

SemanticAnalyzerExpressionTypes
SemanticAnalyzer::analyzeUnaryExpression(UnaryExpression *expression) {
    auto expr_type = analyzeExpression(expression->expr);
    if (expr_type.type != SemanticAnalyzerExpressionTypes::NUMBER) {
        addError("Operand must be of type integer", current_statement);
    }

    return expr_type;
}

SemanticAnalyzerExpressionTypes
SemanticAnalyzer::analyzeLiteralExpression(LiteralExpression *expression) {
    auto type = SemanticAnalyzerExpressionTypes::NUMBER;
    switch (expression->literal_type) {
    case TokenType::V_INT:
    case TokenType::V_FLOAT:
    case TokenType::V_BINARY:
    case TokenType::V_HEX:
    case TokenType::V_OCTAL:
    case TokenType::V_CHAR:
        break;
    case TokenType::V_STRING:
    case TokenType::V_MULTILINE_STRING:
        type = SemanticAnalyzerExpressionTypes::STRING;
        break;
    case TokenType::TRUE:
    case TokenType::FALSE:
        type = SemanticAnalyzerExpressionTypes::BOOL;
        break;
    case TokenType::NIL:
        type = SemanticAnalyzerExpressionTypes::NIL;
        break;
    case TokenType::IDENTIFIER: {
        auto variable = locateVariable(expression->value);
        type = analyzeExpression(variable->value.value()).type;
        return {type, variable};
    }
    default:
        addError("Invalid literal type", current_statement);
        break;
    }
    return {type};
}

VariableDeclaration *SemanticAnalyzer::locateVariable(std::string_view name) {
    for (auto i = compound_index; i >= 0; i--) {
        auto compound = compounds[i];
        auto var = compound->declarations.find(name);
        if (var != compound->declarations.end()) {
            return dynamic_cast<VariableDeclaration *>(var->second);
        }
    }

    return nullptr;
}

void SemanticAnalyzer::checkIfTypesMatch(SemanticAnalyzerExpressionTypes lhs,
                                         SemanticAnalyzerExpressionTypes rhs) {
    if (lhs.type != rhs.type) {
        addError("Operands must be of same type", current_statement);
    }
}

void SemanticAnalyzer::warning(const std::string &message) {
    error.addWarning(message, current_statement->location);
}

void SemanticAnalyzer::addError(const std::string &message) {
    error.addError(message, current_statement->location);
}

void SemanticAnalyzer::addError(const std::string &message, AST *ast) {
    error.addError(message, ast->location);
}

CompoundStatement *SemanticAnalyzer::currentCompound() {
    return this->compounds[this->compound_index];
}