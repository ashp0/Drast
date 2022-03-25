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
    case TokenType::STRUCT:
        return this->structDeclaration();
    case TokenType::EXTERN:
    case TokenType::VOLATILE:
    case TokenType::PRIVATE:
        return this->modifiers();
    case TokenType::INT:
    case TokenType::CHAR:
    case TokenType::VOID:
    case TokenType::BOOL:
    case TokenType::FLOAT:
    case TokenType::STRING:
    case TokenType::IDENTIFIER:
        return this->functionOrVariableDeclaration();
    case TokenType::FOR:
        return this->forLoop();
    case TokenType::RETURN:
        return this->returnStatement();
    case TokenType::IF:
        return this->ifStatements();
    case TokenType::ASM:
        return this->inlineAssembly();
    case TokenType::GOTO:
        return this->gotoStatement();
    case TokenType::T_EOF:
        return nullptr;
    default:
        std::cout << *this->current << std::endl;
        throw this->throw_error("Parser: Cannot Parse Token");
    }
}

std::unique_ptr<Import> Parser::import() {
    std::string import_path;
    bool is_library;

    this->advance(TokenType::IMPORT);

    switch (this->current->type) {
    case TokenType::V_STRING:
        import_path = getAndAdvance(TokenType::V_STRING)->value;
        break;
    case TokenType::IDENTIFIER:
        import_path = getAndAdvance(TokenType::IDENTIFIER)->value;
        is_library = true;
        break;
    default:
        throw this->throw_error("Expected string literal\n");
    }

    return this->create_declaration<Import>(import_path, is_library);
}

std::unique_ptr<StructDeclaration>
Parser::structDeclaration(std::vector<TokenType> modifiers) {
    this->advance(TokenType::STRUCT);

    auto struct_name = getAndAdvance(TokenType::IDENTIFIER)->value;

    this->advance(TokenType::BRACE_OPEN);
    auto struct_body = this->compound();
    this->advance(TokenType::BRACE_CLOSE);

    return this->create_declaration<StructDeclaration>(struct_name, struct_body,
                                                       modifiers);
}

std::unique_ptr<EnumDeclaration>
Parser::enumDeclaration(std::vector<TokenType> modifiers) {
    this->advance(TokenType::ENUM);

    auto enum_name = getAndAdvance(TokenType::IDENTIFIER)->value;

    advance(TokenType::BRACE_OPEN);

    auto enum_cases = enumCases();

    advance(TokenType::BRACE_CLOSE);

    return this->create_declaration<EnumDeclaration>(enum_name, enum_cases,
                                                     modifiers);
}

std::vector<std::unique_ptr<EnumCase>> Parser::enumCases() {
    std::vector<std::unique_ptr<EnumCase>> enum_cases;

    for (int enum_case_value = 0; this->current->type != TokenType::BRACE_CLOSE;
         enum_case_value++) {
        auto case_name = getAndAdvance(TokenType::IDENTIFIER)->value;
        std::unique_ptr<AST> case_value;

        if (matches(TokenType::EQUAL)) {
            case_value = this->expression();
        } else {
            auto case_value_string = std::to_string(enum_case_value);

            case_value = this->create_declaration<LiteralExpression>(
                case_value_string, TokenType::V_NUMBER);
        }

        auto enum_case =
            this->create_declaration<EnumCase>(case_name, case_value);

        enum_cases.push_back(std::move(enum_case));
        matches(TokenType::COMMA);
    }

    return enum_cases;
}

std::unique_ptr<AST>
Parser::functionOrVariableDeclaration(std::vector<TokenType> modifiers) {
    // if (isEqualitiveOperator(peek()->type)) {
    //     return this->expression();
    // }

    auto type = this->type();

    if (matches(TokenType::DOUBLE_COLON)) {
        return this->functionDeclaration(type, std::move(modifiers));
    }

    return this->variableDeclaration(type, std::move(modifiers));
}

std::unique_ptr<AST>
Parser::functionDeclaration(std::unique_ptr<AST> &return_type,
                            std::vector<TokenType> modifiers) {
    auto function_name = getAndAdvance(TokenType::IDENTIFIER)->value;

    advance(TokenType::PARENS_OPEN);
    auto function_arguments = this->functionArguments();
    advance(TokenType::PARENS_CLOSE);

    if (matches(TokenType::BRACE_OPEN)) {
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
        if (matches(TokenType::PERIOD)) {
            advance(TokenType::PERIOD);
            advance(TokenType::PERIOD);

            auto argument = this->create_declaration<FunctionArgument>();

            arguments.push_back(std::move(argument));
            break;
        }
        auto argument_type = this->type();
        auto argument_name = getAndAdvance(TokenType::IDENTIFIER)->value;

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
    std::string variable_name = getAndAdvance(TokenType::IDENTIFIER)->value;
    std::optional<std::unique_ptr<AST>> variable_value = std::nullopt;

    // Variable Initialization
    if (matches(TokenType::EQUAL)) {
        variable_value = this->expression();
    }

    return this->create_declaration<VariableDeclaration>(
        modifiers, variable_name, variable_type, variable_value);
}

std::unique_ptr<ForLoop> Parser::forLoop() {
    this->advance(TokenType::FOR);

    this->advance(TokenType::PARENS_OPEN);

    auto for_initialization = this->statement();
    this->advance(TokenType::COMMA);

    auto for_condition = this->expression();
    this->advance(TokenType::COMMA);

    auto for_increment = this->statement();
    this->advance(TokenType::PARENS_CLOSE);

    this->advance(TokenType::BRACE_OPEN);
    auto for_body = this->compound();
    this->advance(TokenType::BRACE_CLOSE);

    return this->create_declaration<ForLoop>(for_initialization, for_condition,
                                             for_increment, for_body);
}

std::unique_ptr<Return> Parser::returnStatement() {
    advance(TokenType::RETURN);

    std::optional<std::unique_ptr<AST>> return_value = std::nullopt;

    if (isRegularValue(this->current->type)) {
        return_value = this->expression();
    }

    return this->create_declaration<Return>(return_value);
}

std::unique_ptr<If> Parser::ifStatements() {
    advance(TokenType::IF);

    auto if_body_and_statement = this->ifOrElseStatement();

    std::vector<std::unique_ptr<AST>> elseif_conditions = {};
    std::vector<std::unique_ptr<CompoundStatement>> elseif_bodies = {};
    std::optional<std::unique_ptr<CompoundStatement>> else_body = std::nullopt;

    while (matches(TokenType::ELSE)) {
        if (matches(TokenType::IF)) {
            auto else_body_and_statement = this->ifOrElseStatement();

            elseif_conditions.push_back(
                std::move(else_body_and_statement.first));
            elseif_bodies.push_back(std::move(else_body_and_statement.second));
        } else {
            advance(TokenType::BRACE_OPEN);
            else_body = this->compound();
            advance(TokenType::BRACE_CLOSE);
        }
    }

    return this->create_declaration<If>(
        if_body_and_statement.first, if_body_and_statement.second,
        elseif_conditions, elseif_bodies, else_body);
}

std::pair<std::unique_ptr<AST>, std::unique_ptr<CompoundStatement>>
Parser::ifOrElseStatement() {
    advance(TokenType::PARENS_OPEN);
    auto condition = this->expression();
    advance(TokenType::PARENS_CLOSE);

    advance(TokenType::BRACE_OPEN);
    auto body = this->compound();
    advance(TokenType::BRACE_CLOSE);

    return std::make_pair(std::move(condition), std::move(body));
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

std::unique_ptr<GOTO> Parser::gotoStatement() {
    advance(TokenType::GOTO);

    auto label = getAndAdvance(TokenType::IDENTIFIER)->value;

    return this->create_declaration<GOTO>(label);
}

std::unique_ptr<AST> Parser::expression() { return this->equality(); }

std::unique_ptr<AST> Parser::equality() {
    auto expr = this->comparison();
    std::cout << "Equality: " << tokenTypeAsLiteral(this->current->type)
              << std::endl;

    while (isEqualitiveOperator(this->current->type)) {
        TokenType operator_type = getAndAdvance()->type;

        auto right_ast = this->comparison();

        expr = this->create_declaration<BinaryExpression>(expr, right_ast,
                                                          operator_type);
    }

    return expr;
}

std::unique_ptr<AST> Parser::comparison() {
    auto expr = this->term();

    while (isComparitiveOperator(this->current->type)) {
        TokenType operator_type = getAndAdvance()->type;

        auto right_ast = this->term();

        expr = this->create_declaration<BinaryExpression>(expr, right_ast,
                                                          operator_type);
    }

    return expr;
}

std::unique_ptr<AST> Parser::term() {
    auto expr = this->factor();

    while (isAdditiveOperator(this->current->type)) {
        TokenType operator_type = getAndAdvance()->type;

        auto right_ast = this->factor();

        expr = this->create_declaration<BinaryExpression>(expr, right_ast,
                                                          operator_type);
    }

    return expr;
}

std::unique_ptr<AST> Parser::factor() {
    auto expr = this->unary();

    while (isMultiplicativeOperator(this->current->type)) {
        TokenType operator_type = getAndAdvance()->type;

        auto right_ast = this->unary();

        expr = this->create_declaration<BinaryExpression>(expr, right_ast,
                                                          operator_type);
    }

    return expr;
}

std::unique_ptr<AST> Parser::unary() {
    if (isAdditiveOperator(this->current->type)) {
        TokenType operator_type = getAndAdvance()->type;

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

        if (matches(TokenType::PARENS_OPEN)) {
            return this->functionCall(literal->value);
        }

        return literal;
    } else if (matches(TokenType::PARENS_OPEN)) {
        auto expr = this->expression();
        advance(TokenType::PARENS_CLOSE);

        return this->create_declaration<GroupingExpression>(expr);
    } else {
        throw this->throw_error("Invalid Expression");
    }
}

std::unique_ptr<AST> Parser::functionCall(std::string &name) {
    std::vector<std::unique_ptr<AST>> arguments;

    while (this->current->type != TokenType::PARENS_CLOSE) {
        arguments.push_back(this->expression());

        matches(TokenType::COMMA);
    }

    advance(TokenType::PARENS_CLOSE);

    return this->create_declaration<FunctionCall>(name, arguments);
}

std::unique_ptr<AST> Parser::type() {
    auto type =
        this->create_declaration<Type>(*this->current, false, false, false);

    if (!isRegularType(this->current->type)) {
        throw this->throw_error("Parser: Expected type\n");
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
           this->current->type == TokenType::VOLATILE ||
           this->current->type == TokenType::PRIVATE) {
        modifiers.push_back(this->current->type);
        advance();
    }

    if (this->current->type == TokenType::ENUM) {
        return this->enumDeclaration(modifiers);
    } else if (this->current->type == TokenType::STRUCT) {
        return this->structDeclaration(modifiers);
    }
    return functionOrVariableDeclaration(modifiers);
}

void Parser::advance(TokenType type) {
    if (type != this->current->type) {
        throw this->throw_error("Syntax error: expected " +
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

std::unique_ptr<Token> Parser::getAndAdvance() {
    auto prevToken = std::move(this->current);
    advance();
    return prevToken;
}

std::unique_ptr<Token> Parser::getAndAdvance(TokenType type) {
    if (type != this->current->type) {
        throw this->throw_error("Syntax error: expected " +
                                tokenTypeAsLiteral(type) + " but got " +
                                tokenTypeAsLiteral(this->current->type));
    }

    auto prevToken = std::move(this->current);
    advance();
    return prevToken;
}

// std::unique_ptr<Token>& Parser::peek(int offset) {
//     return this->tokens.at(this->index + offset);
// }

bool Parser::matches(TokenType type) {
    if (this->current->type == type) {
        advance(type);
        return true;
    }

    return false;
}

template <class ast_type, class... Args>
std::unique_ptr<ast_type> Parser::create_declaration(Args &&...args) {
    return std::make_unique<ast_type>(std::forward<Args>(args)...,
                                      this->current->location);
}

int Parser::throw_error(std::string message) {
    throw print::error(std::move(message), this->current->location);
}
