//
// Created by Ashwin Paudel on 2022-03-20.
//

#include "Parser.h"

std::unique_ptr<AST> Parser::parse() {
    auto ast = this->compound();

    std::cout << *ast << std::endl;

    return ast;
}

std::unique_ptr<CompoundStatement> Parser::compound() {
    auto compoundStatement =
        std::make_unique<CompoundStatement>(this->current->line);

    while (this->current->type != TokenType::BRACE_CLOSE &&
           this->current->type != TokenType::T_EOF) {
        auto statement = this->statement();
        compoundStatement->insertStatement(statement);
    }

    return compoundStatement;
}

std::unique_ptr<AST> Parser::statement() {
    switch (this->current->type) {
    case TokenType::IMPORT:
        return this->import();
    case TokenType::EXTERN:
    case TokenType::VOLATILE:
        return this->modifiers();
    case TokenType::INT:
    case TokenType::CHAR:
    case TokenType::VOID:
    case TokenType::BOOL:
    case TokenType::FLOAT:
    case TokenType::STRING:
    case TokenType::IDENTIFIER:
        return this->functionOrVariableDeclaration();
    case TokenType::RETURN:
        return this->returnStatement();
    case TokenType::ASM:
        return this->inlineAssembly();
    case TokenType::T_EOF:
        return nullptr;
    default:
        std::cout << *this->current << std::endl;
        throw std::runtime_error("Parser: Cannot Parse Token");
    }

    return nullptr;
}

std::unique_ptr<Import> Parser::import() {
    std::string import_path;
    bool is_library;

    this->advance(TokenType::IMPORT);

    switch (this->current->type) {
    case TokenType::V_STRING:
        import_path = this->current->value;
        this->advance(TokenType::V_STRING);
        break;
    case TokenType::IDENTIFIER:
        import_path = this->current->value;
        is_library = true;
        this->advance(TokenType::IDENTIFIER);
        break;
    default:
        throw std::runtime_error("Expected string literal\n");
    }

    return std::make_unique<Import>(import_path, this->current->line,
                                    is_library);
}

std::unique_ptr<AST>
Parser::functionOrVariableDeclaration(std::vector<TokenType> modifiers) {
    auto type = this->type();

    if (this->current->type == TokenType::DOUBLE_COLON) {
        advance(TokenType::DOUBLE_COLON);
        return this->functionDeclaration(type, std::move(modifiers));
    }

    return this->variableDeclaration(type, std::move(modifiers));
}

std::unique_ptr<AST>
Parser::functionDeclaration(std::unique_ptr<AST> &return_type,
                            std::vector<TokenType> modifiers) {
    auto function_name = this->current->value;
    advance(TokenType::IDENTIFIER);

    advance(TokenType::PARENS_OPEN);
    auto function_arguments = this->functionArguments();
    advance(TokenType::PARENS_CLOSE);

    advance(TokenType::BRACE_OPEN);
    auto function_body = this->compound();
    advance(TokenType::BRACE_CLOSE);

    return std::make_unique<FunctionDeclaration>(
        modifiers, return_type, function_name, function_arguments,
        function_body, this->current->line);
}

std::vector<std::unique_ptr<FunctionArgument>> Parser::functionArguments() {
    std::vector<std::unique_ptr<FunctionArgument>> arguments;
    while (this->current->type != TokenType::PARENS_CLOSE) {
        if (this->current->type == TokenType::PERIOD) {
            this->index += 3;
            this->current = std::move(this->tokens[this->index]);

            std::unique_ptr<FunctionArgument> argument =
                std::make_unique<FunctionArgument>(this->current->line);

            arguments.push_back(std::move(argument));
            break;
        } else {
            auto argument_type = this->type();
            auto argument_name = this->current->value;

            advance(TokenType::IDENTIFIER);

            auto argument = std::make_unique<FunctionArgument>(
                argument_name, argument_type, this->current->line);

            arguments.push_back(std::move(argument));

            advance(TokenType::COMMA);
        }
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

    return std::make_unique<VariableDeclaration>(
        modifiers, variable_name, variable_type, this->current->line,
        variable_value);
}

std::unique_ptr<Return> Parser::returnStatement() {
    advance(TokenType::RETURN);

    std::optional<std::unique_ptr<AST>> return_value = std::nullopt;

    if (isRegularValue(this->current->type)) {
        return_value = this->expression();
    }

    return std::make_unique<Return>(return_value, this->current->line);
}

std::unique_ptr<ASM> Parser::inlineAssembly() {
    advance(TokenType::ASM);
    advance(TokenType::PARENS_OPEN);

    std::vector<std::string> instructions;

    while (this->current->type != TokenType::PARENS_CLOSE) {
        instructions.push_back(this->current->value);
        advance(TokenType::V_STRING);
    }

    advance(TokenType::PARENS_CLOSE);

    return std::make_unique<ASM>(instructions, this->current->line);
}

std::unique_ptr<AST> Parser::expression() { return this->equality(); }

std::unique_ptr<AST> Parser::equality() {
    auto left_ast = this->comparison();

    while (isEqualitiveOperator(this->current->type)) {
        TokenType operator_type = this->current->type;
        advance(operator_type);

        auto right_ast = this->comparison();

        return std::make_unique<BinaryExpression>(
            left_ast, right_ast, operator_type, this->current->line);
    }

    return left_ast;
}

std::unique_ptr<AST> Parser::comparison() {
    auto left_ast = this->term();

    while (isComparitiveOperator(this->current->type)) {
        TokenType operator_type = this->current->type;
        advance(operator_type);

        auto right_ast = this->term();

        return std::make_unique<BinaryExpression>(
            left_ast, right_ast, operator_type, this->current->line);
    }

    return left_ast;
}

std::unique_ptr<AST> Parser::term() {
    auto left_ast = this->factor();

    while (isAdditiveOperator(this->current->type)) {
        TokenType operator_type = this->current->type;
        advance(operator_type);

        auto right_ast = this->expression();

        return std::make_unique<BinaryExpression>(
            left_ast, right_ast, operator_type, this->current->line);
    }

    return left_ast;
}

std::unique_ptr<AST> Parser::factor() {
    auto left_ast = this->unary();

    while (isMultiplicativeOperator(this->current->type)) {
        TokenType operator_type = this->current->type;
        advance(operator_type);

        auto right_ast = this->expression();

        return std::make_unique<BinaryExpression>(
            left_ast, right_ast, operator_type, this->current->line);
    }

    return left_ast;
}

std::unique_ptr<AST> Parser::unary() {
    if (isAdditiveOperator(this->current->type)) {
        TokenType operator_type = this->current->type;
        advance(operator_type);

        auto expr = this->unary();

        return std::make_unique<UnaryExpression>(expr, operator_type,
                                                 this->current->line);
    }

    return this->primary();
}

std::unique_ptr<AST> Parser::primary() {
    if (isRegularValue(this->current->type)) {
        auto literal = std::make_unique<LiteralExpression>(this->current,
                                                           this->current->line);
        advance();

        if (this->current->type == TokenType::PARENS_OPEN) {
            return this->functionCall(literal->token->value);
        }

        return literal;
    } else if (this->current->type == TokenType::PARENS_OPEN) {
        advance(TokenType::PARENS_OPEN);
        auto expr = this->expression();
        advance(TokenType::PARENS_CLOSE);

        return std::make_unique<GroupingExpression>(expr, this->current->line);
    } else {
        throw std::runtime_error("Expected Expression");
    }
}

std::unique_ptr<AST> Parser::functionCall(std::string &name) {
    advance(TokenType::PARENS_OPEN);

    std::vector<std::unique_ptr<AST>> arguments;

    while (this->current->type != TokenType::PARENS_CLOSE) {
        arguments.push_back(this->expression());

        if (this->current->type == TokenType::COMMA) {
            advance(TokenType::COMMA);
        }
    }

    advance(TokenType::PARENS_CLOSE);

    return std::make_unique<FunctionCall>(name, arguments, this->current->line);
}

std::unique_ptr<AST> Parser::type() {
    auto type = std::make_unique<Type>(*this->current, false, false, false,
                                       this->current->line);

    if (!isRegularType(this->current->type)) {
        throw std::runtime_error("Parser: Expected type\n");
    }

    advance();

    while (!type->is_optional || !type->is_array || !type->is_optional) {
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

std::unique_ptr<AST> Parser::modifiers() {
    std::vector<TokenType> modifiers;
    while (this->current->type == TokenType::EXTERN ||
           this->current->type == TokenType::VOLATILE) {
        modifiers.push_back(this->current->type);
        advance();
    }

    return functionOrVariableDeclaration(modifiers);
}

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
