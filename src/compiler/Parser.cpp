//
// Created by Ashwin Paudel on 2022-03-20.
//

#include "Parser.h"

#include <utility>

std::unique_ptr<AST> Parser::parse() {
    auto ast = this->compound();
    std::cout << *ast << std::endl;
    return ast;
}

std::unique_ptr<CompoundStatement> Parser::compound() {
    std::unique_ptr<CompoundStatement> compoundStatement =
        std::make_unique<CompoundStatement>(this->current->line);

    for (;;) {
        if (this->current->type == TokenType::BRACE_CLOSE ||
            this->current->type == TokenType::T_EOF) {
            break;
        }
        std::unique_ptr<AST> statement = this->statement();
        compoundStatement->insertStatement(statement);
    }

    return compoundStatement;
}

std::unique_ptr<AST> Parser::statement() {
    switch (this->current->type) {
    case TokenType::IMPORT:
        return this->import();
    case TokenType::INT:
    case TokenType::CHAR:
    case TokenType::VOID:
    case TokenType::BOOL:
    case TokenType::FLOAT:
    case TokenType::STRING:
    case TokenType::IDENTIFIER:
        return this->functionOrVariableDeclaration();
    case TokenType::T_EOF:
        return nullptr;
    default:
        std::cout << *this->current << std::endl;
        throw std::runtime_error("Parser: Cannot Parse Token");
    }

    return nullptr;
}

std::unique_ptr<Import> Parser::import() {
    std::unique_ptr<Import> import =
        std::make_unique<Import>(this->current->value, this->current->line);

    this->advance(TokenType::IMPORT);

    if (this->current->type == TokenType::V_STRING) {
        import->import_path = this->current->value;
        this->advance(TokenType::V_STRING);
    } else if (this->current->type == TokenType::IDENTIFIER) {
        import->import_path = this->current->value;
        import->is_library = true;
        this->advance(TokenType::IDENTIFIER);
    } else {
        throw std::runtime_error("Expected string literal\n");
    }

    return import;
}

std::unique_ptr<AST>
Parser::functionOrVariableDeclaration(std::vector<TokenType> modifiers) {
    std::unique_ptr<AST> type = this->type();

    if (this->current->type == TokenType::DOUBLE_COLON) {
        advance(TokenType::DOUBLE_COLON);
        return this->functionDeclaration(type, std::move(modifiers));
    }

    return this->variableDeclaration(type, std::move(modifiers));
}

std::unique_ptr<AST>
Parser::functionDeclaration(std::unique_ptr<AST> &return_type,
                            std::vector<TokenType> modifiers) {
    std::string function_name;
    std::vector<std::unique_ptr<FunctionArgument>> function_arguments;
    std::unique_ptr<CompoundStatement> function_body;

    // Function Name
    function_name = this->current->value;
    advance(TokenType::IDENTIFIER);

    // Function Arguments
    advance(TokenType::PARENS_OPEN);
    function_arguments = this->functionArguments();
    advance(TokenType::PARENS_CLOSE);

    // Function Body
    advance(TokenType::BRACE_OPEN);
    function_body = this->compound();
    advance(TokenType::BRACE_CLOSE);

    // Function AST
    std::unique_ptr<FunctionDeclaration> function =
        std::make_unique<FunctionDeclaration>(
            modifiers, return_type, function_name, function_arguments,
            function_body, this->current->line);

    return function;
}

std::vector<std::unique_ptr<FunctionArgument>> Parser::functionArguments() {
    std::vector<std::unique_ptr<FunctionArgument>> arguments;
    while (this->current->type != TokenType::PARENS_CLOSE) {
        if (this->current->type == TokenType::PERIOD) {
            advance(TokenType::PERIOD);
            advance(TokenType::PERIOD);
            advance(TokenType::PERIOD);

            std::unique_ptr<FunctionArgument> argument =
                std::make_unique<FunctionArgument>(this->current->line);

            arguments.push_back(std::move(argument));

            break;
        }

        std::unique_ptr<AST> argument_type = this->type();
        std::string argument_name = this->current->value;

        advance(TokenType::IDENTIFIER);

        std::unique_ptr<FunctionArgument> argument =
            std::make_unique<FunctionArgument>(argument_name, argument_type,
                                               this->current->line);

        arguments.push_back(std::move(argument));

        advance(TokenType::COMMA);
    }

    return arguments;
}

std::unique_ptr<AST>
Parser::variableDeclaration(std::unique_ptr<AST> &variable_type,
                            std::vector<TokenType> modifiers) {
    // Variable Name
    std::string variable_name = this->current->value;
    std::optional<std::unique_ptr<AST>> variable_value = std::nullopt;

    advance(TokenType::IDENTIFIER);

    // Variable Initialization
    if (this->current->type == TokenType::EQUAL) {
        advance(TokenType::EQUAL);
        variable_value = this->expression();
    }

    // Variable AST
    std::unique_ptr<VariableDeclaration> variable =
        std::make_unique<VariableDeclaration>(
            modifiers, variable_name, variable_type, this->current->line,
            variable_value);

    return variable;
}

std::unique_ptr<AST> Parser::expression() { return this->equality(); }

std::unique_ptr<AST> Parser::equality() {
    std::unique_ptr<AST> left_ast = this->comparison();

    while (this->current->type == TokenType::EQUAL_EQUAL ||
           this->current->type == TokenType::NOT_EQUAL) {
        TokenType operator_type = this->current->type;
        advance(operator_type);

        std::unique_ptr<AST> right_ast = this->comparison();

        std::unique_ptr<BinaryExpression> binary_expression =
            std::make_unique<BinaryExpression>(
                left_ast, right_ast, operator_type, this->current->line);

        return binary_expression;
    }

    return left_ast;
}

std::unique_ptr<AST> Parser::comparison() {
    std::unique_ptr<AST> left_ast = this->term();

    while (this->current->type == TokenType::LESS_THAN ||
           this->current->type == TokenType::LESS_THAN_EQUAL ||
           this->current->type == TokenType::GREATER_THAN ||
           this->current->type == TokenType::GREATER_THAN_EQUAL) {
        TokenType operator_type = this->current->type;
        advance(operator_type);

        std::unique_ptr<AST> right_ast = this->term();

        std::unique_ptr<BinaryExpression> binary_expression =
            std::make_unique<BinaryExpression>(
                left_ast, right_ast, operator_type, this->current->line);

        return binary_expression;
    }

    return left_ast;
}

std::unique_ptr<AST> Parser::term() {
    std::unique_ptr<AST> left_ast = this->factor();

    while (this->current->type == TokenType::OPERATOR_ADD ||
           this->current->type == TokenType::OPERATOR_SUB ||
           this->current->type == TokenType::PERIOD) {
        TokenType operator_type = this->current->type;
        advance(operator_type);

        std::unique_ptr<AST> right_ast = this->expression();

        std::unique_ptr<BinaryExpression> binary_expression =
            std::make_unique<BinaryExpression>(
                left_ast, right_ast, operator_type, this->current->line);

        return binary_expression;
    }

    return left_ast;
}

std::unique_ptr<AST> Parser::factor() {
    std::unique_ptr<AST> left_ast = this->unary();

    while (this->current->type == TokenType::OPERATOR_MUL ||
           this->current->type == TokenType::OPERATOR_DIV ||
           this->current->type == TokenType::OPERATOR_MOD) {
        TokenType operator_type = this->current->type;
        advance(operator_type);

        std::unique_ptr<AST> right_ast = this->expression();

        std::unique_ptr<BinaryExpression> binary_expression =
            std::make_unique<BinaryExpression>(
                left_ast, right_ast, operator_type, this->current->line);

        return binary_expression;
    }

    return left_ast;
}

std::unique_ptr<AST> Parser::unary() {
    if (this->current->type == TokenType::OPERATOR_ADD ||
        this->current->type == TokenType::OPERATOR_SUB) {
        TokenType operator_type = this->current->type;
        advance(operator_type);

        std::unique_ptr<AST> expr = this->unary();

        std::unique_ptr<UnaryExpression> unary_expression =
            std::make_unique<UnaryExpression>(expr, operator_type,
                                              this->current->line);

        return unary_expression;
    }

    return this->primary();
}

std::unique_ptr<AST> Parser::primary() {
    if (this->current->type == TokenType::V_NUMBER ||
        this->current->type == TokenType::V_CHAR ||
        this->current->type == TokenType::V_STRING ||
        this->current->type == TokenType::TRUE ||
        this->current->type == TokenType::FALSE ||
        this->current->type == TokenType::IDENTIFIER) {
        std::unique_ptr<AST> literal = std::make_unique<LiteralExpression>(
            this->current, this->current->line);
        advance();
        return literal;
    } else if (this->current->type == TokenType::PARENS_OPEN) {
        advance(TokenType::PARENS_OPEN);
        std::unique_ptr<AST> expr = this->expression();
        advance(TokenType::PARENS_CLOSE);

        std::unique_ptr<GroupingExpression> grouping_expression =
            std::make_unique<GroupingExpression>(expr, this->current->line);

        return grouping_expression;
    } else {
        throw std::runtime_error("Expected Expression");
    }
}

std::unique_ptr<AST> Parser::type() {
    std::unique_ptr<Type> type = std::make_unique<Type>(
        *this->current, false, false, false, this->current->line);

    switch (this->current->type) {
    case TokenType::INT:
    case TokenType::CHAR:
    case TokenType::VOID:
    case TokenType::BOOL:
    case TokenType::FLOAT:
    case TokenType::STRING:
    case TokenType::IDENTIFIER:
        break;
    default:
        throw std::runtime_error("Parser: Expected type\n");
    }

    advance();

    for (;;) {
        switch (this->current->type) {
        case TokenType::QUESTION:
            type->is_optional = true;
            break;
        case TokenType::OPERATOR_MUL:
            type->is_pointer = true;
            break;
        case TokenType::SQUARE_OPEN:
            this->advance();
            type->is_array = true;
            break;
        default:
            goto end;
        }

        this->advance();
    }

end:
    return type;
}

// std::unique_ptr<AST> Parser::modifiers();

void Parser::advance(TokenType type) {
    if (type != this->current->type) {
        throw std::runtime_error("Syntax error: expected " +
                                 tokenTypeAsLiteral(type) + " but got " +
                                 tokenTypeAsLiteral(this->current->type));
    }

    this->index += 1;

    this->current = std::move(this->tokens.at(this->index));
}

void Parser::advance() {
    this->index += 1;

    this->current = std::move(this->tokens.at(this->index));
}
