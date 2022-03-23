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
    auto compoundStatement = this->create_declaration<CompoundStatement>();

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
    case TokenType::ENUM:
        return this->enumDeclaration();
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

    return this->create_declaration<Import>(import_path, is_library);
}

std::unique_ptr<EnumDeclaration> Parser::enumDeclaration() {
    this->advance(TokenType::ENUM);

    auto enum_name = this->current->value;
    this->advance(TokenType::IDENTIFIER);

    std::vector<std::unique_ptr<EnumCase>> enum_cases;
    int case_enum_value = 0;

    advance(TokenType::BRACE_OPEN);

    while (this->current->type != TokenType::BRACE_CLOSE) {
        auto case_name = this->current->value;
        std::unique_ptr<AST> case_value;

        this->advance(TokenType::IDENTIFIER);

        if (this->current->type == TokenType::EQUAL) {
            this->advance(TokenType::EQUAL);
            case_value = this->expression();
        } else {
            auto case_value_string = std::to_string(case_enum_value);

            auto token = std::make_unique<Token>(
                case_value_string, TokenType::V_NUMBER, this->current->line,
                this->current->column);

            case_value = this->create_declaration<LiteralExpression>(token);
        }

        auto enum_case =
            this->create_declaration<EnumCase>(case_name, case_value);

        enum_cases.push_back(std::move(enum_case));
        case_enum_value++;

        if (this->current->type == TokenType::COMMA) {
            this->advance(TokenType::COMMA);
        }
    }

    advance(TokenType::BRACE_CLOSE);

    return this->create_declaration<EnumDeclaration>(enum_name, enum_cases);
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

    if (this->current->type == TokenType::BRACE_OPEN) {
        advance(TokenType::BRACE_OPEN);
        std::unique_ptr<CompoundStatement> function_body = this->compound();
        advance(TokenType::BRACE_CLOSE);

        return this->create_declaration<FunctionDeclaration>(
            modifiers, return_type, function_name, function_arguments,
            function_body);
    } else {
        return this->create_declaration<FunctionDeclaration>(
            modifiers, return_type, function_name, function_arguments);
    }
}

std::vector<std::unique_ptr<FunctionArgument>> Parser::functionArguments() {
    std::vector<std::unique_ptr<FunctionArgument>> arguments;
    while (this->current->type != TokenType::PARENS_CLOSE) {
        if (this->current->type == TokenType::PERIOD) {
            this->index += 3;
            this->current = std::move(this->tokens[this->index]);

            auto argument = this->create_declaration<FunctionArgument>();

            arguments.push_back(std::move(argument));
            break;
        }
        auto argument_type = this->type();
        auto argument_name = this->current->value;

        advance(TokenType::IDENTIFIER);

        auto argument = this->create_declaration<FunctionArgument>(
            argument_name, argument_type);

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

    return this->create_declaration<VariableDeclaration>(
        modifiers, variable_name, variable_type, variable_value);
}

std::unique_ptr<Return> Parser::returnStatement() {
    advance(TokenType::RETURN);

    std::optional<std::unique_ptr<AST>> return_value = std::nullopt;

    if (isRegularValue(this->current->type)) {
        return_value = this->expression();
    }

    return this->create_declaration<Return>(return_value);
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

    return this->create_declaration<ASM>(instructions);
}

std::unique_ptr<AST> Parser::expression() { return this->equality(); }

std::unique_ptr<AST> Parser::equality() {
    auto left_ast = this->comparison();

    while (isEqualitiveOperator(this->current->type)) {
        TokenType operator_type = this->current->type;
        advance(operator_type);

        auto right_ast = this->comparison();

        return this->create_declaration<BinaryExpression>(left_ast, right_ast,
                                                          operator_type);
    }

    return left_ast;
}

std::unique_ptr<AST> Parser::comparison() {
    auto left_ast = this->term();

    while (isComparitiveOperator(this->current->type)) {
        TokenType operator_type = this->current->type;
        advance(operator_type);

        auto right_ast = this->term();

        return this->create_declaration<BinaryExpression>(left_ast, right_ast,
                                                          operator_type);
    }

    return left_ast;
}

std::unique_ptr<AST> Parser::term() {
    auto left_ast = this->factor();

    while (isAdditiveOperator(this->current->type)) {
        TokenType operator_type = this->current->type;
        advance(operator_type);

        auto right_ast = this->expression();

        return this->create_declaration<BinaryExpression>(left_ast, right_ast,
                                                          operator_type);
    }

    return left_ast;
}

std::unique_ptr<AST> Parser::factor() {
    auto left_ast = this->unary();

    while (isMultiplicativeOperator(this->current->type)) {
        TokenType operator_type = this->current->type;
        advance(operator_type);

        auto right_ast = this->expression();

        return this->create_declaration<BinaryExpression>(left_ast, right_ast,
                                                          operator_type);
    }

    return left_ast;
}

std::unique_ptr<AST> Parser::unary() {
    if (isAdditiveOperator(this->current->type)) {
        TokenType operator_type = this->current->type;
        advance(operator_type);

        auto expr = this->unary();

        return this->create_declaration<UnaryExpression>(expr, operator_type);
    }

    return this->primary();
}

std::unique_ptr<AST> Parser::primary() {
    if (isRegularValue(this->current->type)) {
        auto literal =
            this->create_declaration<LiteralExpression>(this->current);
        advance();

        if (this->current->type == TokenType::PARENS_OPEN) {
            return this->functionCall(literal->token->value);
        }

        return literal;
    } else if (this->current->type == TokenType::PARENS_OPEN) {
        advance(TokenType::PARENS_OPEN);
        auto expr = this->expression();
        advance(TokenType::PARENS_CLOSE);

        return this->create_declaration<GroupingExpression>(expr);
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

    return this->create_declaration<FunctionCall>(name, arguments);
}

std::unique_ptr<AST> Parser::type() {
    auto type =
        this->create_declaration<Type>(*this->current, false, false, false);

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

template <class ast_type, class... Args>
std::unique_ptr<ast_type> Parser::create_declaration(Args &&...args) {
    return std::make_unique<ast_type>(std::forward<Args>(args)...,
                                      this->current->line);
}