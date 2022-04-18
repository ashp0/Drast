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
    default:
        return false;
    }
}

bool SemanticAnalyzer::importStatement(ImportStatement *ast) {
    //    warning("Import statement cannot be checked");
    return true;
}

ExpressionType SemanticAnalyzer::analyzeExpression(AST *expression) {
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

ExpressionType
SemanticAnalyzer::analyzeBinaryExpression(BinaryExpression *expression) {
    ExpressionType left_type = analyzeExpression(expression->left);
    ExpressionType right_type = analyzeExpression(expression->right);
    checkIfTypesMatch(left_type, right_type);

    return left_type;
}

ExpressionType
SemanticAnalyzer::analyzeGroupingExpression(GroupingExpression *expression) {
    return analyzeExpression(expression->expr);
}

ExpressionType
SemanticAnalyzer::analyzeUnaryExpression(UnaryExpression *expression) {
    auto expr_type = analyzeExpression(expression->expr);
    if (expr_type.type != ExpressionType::NUMBER) {
        addError("Operand must be of type integer", current_statement);
    }

    return expr_type;
}

ExpressionType
SemanticAnalyzer::analyzeLiteralExpression(LiteralExpression *expression) {
    ExpressionType::Type type = ExpressionType::NUMBER;
    if (expression->literal_type == TokenType::V_INT ||
        expression->literal_type == TokenType::V_FLOAT ||
        expression->literal_type == TokenType::V_BINARY ||
        expression->literal_type == TokenType::V_HEX ||
        expression->literal_type == TokenType::V_OCTAL ||
        expression->literal_type == TokenType::V_CHAR) {
        goto end;
    } else if (expression->literal_type == TokenType::V_STRING ||
               expression->literal_type == TokenType::V_MULTILINE_STRING) {
        type = ExpressionType::STRING;
    } else if (expression->literal_type == TokenType::TRUE ||
               expression->literal_type == TokenType::FALSE) {
        type = ExpressionType::BOOL;
    } else if (expression->literal_type == TokenType::NIL) {
        type = ExpressionType::NIL;
    } else if (expression->literal_type == TokenType::IDENTIFIER) {
        auto var = findVariable(expression->value);
        if (var != nullptr) {
            if (var->value) {
                type = analyzeExpression(var->value.value()).type;
            } else {
                addError("Variable is not initialized", var);
                type = ExpressionType::TYPE;
            }
        }
    } else {
        addError("Invalid literal type", current_statement);
    }
end:
    return {type};
}

void SemanticAnalyzer::checkIfTypesMatch(ExpressionType lhs,
                                         ExpressionType rhs) {
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

VariableDeclaration *SemanticAnalyzer::findVariable(std::string_view name) {
    for (auto &compound : this->compounds) {
        for (auto &declaration_name : compound->declaration_names) {
            if (declaration_name.second->type ==
                ASTType::VARIABLE_DECLARATION) {
                if (declaration_name.first == name) {
                    return dynamic_cast<VariableDeclaration *>(
                        declaration_name.second);
                }
            }
        }
    }

    addError("Cannot find variable named: " + std::string(name));
    return nullptr;
}

CompoundStatement *SemanticAnalyzer::currentCompound() {
    return this->compounds[this->compound_index];
}