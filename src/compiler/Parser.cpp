//
// Created by Ashwin Paudel on 2022-03-26.
//

#include "Parser.h"

void Parser::parse() {
    auto compound = this->compound();

    std::cout << compound->toString() << std::endl;
}

CompoundStatement *Parser::compound() {
    auto compoundStatement = this->create_declaration<CompoundStatement>();

    while (this->current().type != TokenType::BRACE_CLOSE &&
           this->current().type != TokenType::T_EOF) {
        auto statement = this->statement();
        compoundStatement->statements.push_back(statement);
    }

    return compoundStatement;
}

AST *Parser::statement() {
    switch (this->current().type) {
    case TokenType::IMPORT:
        return import();
    case TokenType::ENUM:
        return this->enum_declaration();
    case TokenType::STRUCT:
        return this->struct_declaration();
    case TokenType::EXTERN:
    case TokenType::VOLATILE:
    case TokenType::PRIVATE:
        return qualifiers();
    case TokenType::INT:
    case TokenType::CHAR:
    case TokenType::VOID:
    case TokenType::BOOL:
    case TokenType::FLOAT:
    case TokenType::STRING:
    case TokenType::IDENTIFIER:
        return this->function_or_variable_declaration({});
    case TokenType::FOR:
        return this->for_loop();
    case TokenType::WHILE:
        return this->while_loop();
    case TokenType::RETURN:
        return this->return_statement();
    case TokenType::IF:
        return this->if_statement();
    case TokenType::ASM:
        return this->inline_assembly();
    case TokenType::GOTO:
        return this->goto_statement();
    case TokenType::SWITCH:
        return this->switch_statement();
    case TokenType::BREAK:
    case TokenType::CONTINUE:
        return this->token();
    case TokenType::T_EOF:
        return nullptr;
    default:
        throw this->throw_error("Parser: Cannot Parse Token");
    }

    return nullptr;
}

ImportStatement *Parser::import() {
    advance(TokenType::IMPORT);

    std::string_view import_path;
    bool is_library = false;
    switch (this->current().type) {
    case TokenType::V_STRING:
        import_path =
            getAndAdvance(TokenType::V_STRING)->value(this->printer.source);
        break;
    case TokenType::IDENTIFIER:
        import_path =
            getAndAdvance(TokenType::IDENTIFIER)->value(this->printer.source);
        is_library = true;
        break;
    default:
        this->throw_error("Expected string literal\n");
    }

    return this->create_declaration<ImportStatement>(import_path, is_library);
}

StructDeclaration *
Parser::struct_declaration(const std::vector<TokenType> &qualifiers) {
    advance(TokenType::STRUCT);

    auto struct_name =
        getAndAdvance(TokenType::IDENTIFIER)->value(this->printer.source);

    advance(TokenType::BRACE_OPEN);
    auto struct_body = this->compound();
    advance(TokenType::BRACE_CLOSE);

    return this->create_declaration<StructDeclaration>(struct_name, qualifiers,
                                                       struct_body);
}

EnumDeclaration *
Parser::enum_declaration(const std::vector<TokenType> &qualifiers) {
    advance(TokenType::ENUM);

    auto enum_name =
        getAndAdvance(TokenType::IDENTIFIER)->value(this->printer.source);

    advance(TokenType::BRACE_OPEN);
    auto cases = enum_cases();
    advance(TokenType::BRACE_CLOSE);

    return this->create_declaration<EnumDeclaration>(enum_name, cases,
                                                     qualifiers);
}

std::vector<EnumCase *> Parser::enum_cases() {
    std::vector<EnumCase *> enum_cases;

    for (int enum_case_value = 0;
         this->current().type != TokenType::BRACE_CLOSE; enum_case_value++) {
        auto case_name =
            getAndAdvance(TokenType::IDENTIFIER)->value(this->printer.source);

        AST *case_value;

        if (advanceIf(TokenType::EQUAL)) {
            case_value = this->expression();
        } else {
            auto case_value_string = std::to_string(enum_case_value);

            case_value = this->create_declaration<LiteralExpression>(
                case_value_string, TokenType::V_NUMBER);
        }

        auto enum_case =
            this->create_declaration<EnumCase>(case_name, case_value);

        enum_cases.push_back(enum_case);
        advanceIf(TokenType::COMMA);
    }

    return enum_cases;
}

AST *Parser::function_or_variable_declaration(
    std::vector<TokenType> qualifiers) {
    if (isOperatorType(peek().type)) {
        if (isKeywordType(this->current().type)) {
            goto start;
        }
        return this->expression();
    }

start:
    auto type = this->type();

    if (advanceIf(TokenType::DOUBLE_COLON)) {
        return this->function_declaration(type, qualifiers);
    }

    return this->variable_declaration(type, qualifiers);
}

FunctionDeclaration *
Parser::function_declaration(AST *&return_type,
                             std::vector<TokenType> qualifiers) {
    auto function_name =
        getAndAdvance(TokenType::IDENTIFIER)->value(this->printer.source);

    advance(TokenType::PARENS_OPEN);

    auto function_arguments = this->function_arguments();

    advance(TokenType::PARENS_CLOSE);

    if (advanceIf(TokenType::BRACE_OPEN)) {
        auto function_body = this->compound();
        advance(TokenType::BRACE_CLOSE);
        return this->create_declaration<FunctionDeclaration>(
            qualifiers, return_type, function_name, function_arguments,
            function_body);
    }

    return this->create_declaration<FunctionDeclaration>(
        qualifiers, return_type, function_name, function_arguments);
}

std::vector<FunctionArgument *> Parser::function_arguments() {
    std::vector<FunctionArgument *> arguments;

    while (this->current().type != TokenType::PARENS_CLOSE) {
        if (advanceIf(TokenType::PERIOD)) {
            advance(TokenType::PERIOD);
            advance(TokenType::PERIOD);

            auto argument = this->create_declaration<FunctionArgument>();
            argument->is_vaarg = true;

            arguments.push_back(argument);

            break;
        }
        auto argument_type = this->type();
        auto argument_name =
            getAndAdvance(TokenType::IDENTIFIER)->value(this->printer.source);

        auto argument = this->create_declaration<FunctionArgument>(
            argument_name, argument_type);
        arguments.push_back(argument);

        if (!advanceIf(TokenType::COMMA)) {
            break;
        }
    }

    return arguments;
}

AST *Parser::variable_declaration(AST *&variable_type,
                                  std::vector<TokenType> qualifiers) {
    auto variable_name =
        getAndAdvance(TokenType::IDENTIFIER)->value(this->printer.source);

    std::optional<AST *> variable_value = std::nullopt;
    if (advanceIf(TokenType::EQUAL)) {
        variable_value = this->expression();
    }

    return this->create_declaration<VariableDeclaration>(
        variable_name, variable_type, variable_value, qualifiers);
}

ForLoop *Parser::for_loop() {
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

WhileLoop *Parser::while_loop() {
    advance(TokenType::WHILE);

    advance(TokenType::PARENS_OPEN);
    auto expression = this->expression();
    advance(TokenType::PARENS_CLOSE);

    this->advance(TokenType::BRACE_OPEN);
    auto body = this->compound();
    this->advance(TokenType::BRACE_CLOSE);

    return this->create_declaration<WhileLoop>(expression, body);
}

Return *Parser::return_statement() {
    advance(TokenType::RETURN);

    std::optional<AST *> return_value = std::nullopt;

    if (isRegularValue(this->current().type)) {
        return_value = this->expression();
    }

    return this->create_declaration<Return>(return_value);
}

If *Parser::if_statement() {
    advance(TokenType::IF);

    auto if_body_and_statement = this->if_else_statements();

    std::vector<AST *> elseif_conditions = {};
    std::vector<CompoundStatement *> elseif_bodies = {};
    std::optional<CompoundStatement *> else_body = std::nullopt;

    while (advanceIf(TokenType::ELSE)) {
        if (advanceIf(TokenType::IF)) {
            auto else_body_and_statement = this->if_else_statements();

            elseif_conditions.push_back(else_body_and_statement.first);
            elseif_bodies.push_back(else_body_and_statement.second);
        } else {
            advance(TokenType::BRACE_OPEN);
            else_body = this->compound();
            advance(TokenType::BRACE_CLOSE);
            break;
        }
    }

    return this->create_declaration<If>(
        if_body_and_statement.first, if_body_and_statement.second,
        elseif_conditions, elseif_bodies, else_body);
}

std::pair<AST *, CompoundStatement *> Parser::if_else_statements() {
    advance(TokenType::PARENS_OPEN);
    auto condition = this->expression();
    advance(TokenType::PARENS_CLOSE);

    advance(TokenType::BRACE_OPEN);
    auto body = this->compound();
    advance(TokenType::BRACE_CLOSE);

    return std::make_pair(condition, body);
}

ASM *Parser::inline_assembly() {
    advance(TokenType::ASM);
    advance(TokenType::PARENS_OPEN);

    std::vector<std::string_view> instructions;

    while (this->current().type != TokenType::PARENS_CLOSE) {
        instructions.push_back(
            getAndAdvance(TokenType::V_STRING)->value(this->printer.source));
    }

    advance(TokenType::PARENS_CLOSE);

    return this->create_declaration<ASM>(instructions);
}

GOTO *Parser::goto_statement() {
    advance(TokenType::GOTO);

    auto label =
        getAndAdvance(TokenType::IDENTIFIER)->value(this->printer.source);

    return this->create_declaration<GOTO>(label, true);
}

SwitchStatement *Parser::switch_statement() {
    advance(TokenType::SWITCH);

    advance(TokenType::PARENS_OPEN);
    auto switch_expression = expression();
    advance(TokenType::PARENS_CLOSE);

    advance(TokenType::BRACE_OPEN);
    auto cases = switch_cases();
    advance(TokenType::BRACE_CLOSE);

    return this->create_declaration<SwitchStatement>(switch_expression, cases);
}

std::vector<SwitchCase *> Parser::switch_cases() {
    std::vector<SwitchCase *> cases;

    while (this->current().type != TokenType::BRACE_CLOSE) {
        cases.push_back(switch_case());
    }

    return cases;
}

SwitchCase *Parser::switch_case() {
    bool is_case = advanceIf(TokenType::CASE);
    if (!is_case) {
        advance(TokenType::DEFAULT);
    }

    AST *case_expression;
    if (is_case) {
        case_expression = primary(false);
    }

    advance(TokenType::COLON);

    CompoundStatement *case_body =
        this->create_declaration<CompoundStatement>();

    while (this->current().type != TokenType::CASE &&
           this->current().type != TokenType::DEFAULT &&
           this->current().type != TokenType::BRACE_CLOSE) {
        case_body->statements.push_back(statement());
    }

    if (is_case) {
        return this->create_declaration<SwitchCase>(case_expression, case_body,
                                                    is_case);
    } else {
        return this->create_declaration<SwitchCase>(case_body, is_case);
    }
}

AST *Parser::expression() { return this->equality(); }

AST *Parser::equality() {
    auto expr = this->comparison();

    while (isEqualitiveOperator(this->current().type)) {
        TokenType operator_type = getAndAdvance()->type;

        auto right_ast = this->comparison();

        expr = this->create_declaration<BinaryExpression>(expr, right_ast,
                                                          operator_type);
    }

    return expr;
}

AST *Parser::comparison() {
    auto expr = this->term();

    while (isComparitiveOperator(this->current().type)) {
        TokenType operator_type = getAndAdvance()->type;

        auto right_ast = this->term();

        expr = this->create_declaration<BinaryExpression>(expr, right_ast,
                                                          operator_type);
    }

    return expr;
}

AST *Parser::term() {
    auto expr = this->factor();

    while (isAdditiveOperator(this->current().type)) {
        TokenType operator_type = getAndAdvance()->type;

        auto right_ast = this->factor();

        expr = this->create_declaration<BinaryExpression>(expr, right_ast,
                                                          operator_type);
    }

    return expr;
}

AST *Parser::factor() {
    auto expr = this->unary();

    while (isMultiplicativeOperator(this->current().type)) {
        TokenType operator_type = getAndAdvance()->type;

        auto right_ast = this->unary();

        expr = this->create_declaration<BinaryExpression>(expr, right_ast,
                                                          operator_type);
    }

    return expr;
}

AST *Parser::unary() {
    if (isAdditiveOperator(this->current().type)) {
        TokenType operator_type = getAndAdvance()->type;

        auto expr = this->unary();

        return this->create_declaration<UnaryExpression>(expr, operator_type);
    }

    return this->primary();
}

AST *Parser::primary(bool parses_goto) {
    if (isRegularValue(this->current().type)) {
        auto literal = this->create_declaration<LiteralExpression>(
            this->current().value(this->printer.source), this->current().type);
        advance();

        if (advanceIf(TokenType::PARENS_OPEN)) {
            return this->function_call(literal->value);
        } else if (parses_goto) {
            if (advanceIf(TokenType::COLON)) {
                return this->create_declaration<GOTO>(literal->value, false);
            }
        }

        return literal;
    } else if (advanceIf(TokenType::PARENS_OPEN)) {
        auto expr = this->expression();
        advance(TokenType::PARENS_CLOSE);

        return this->create_declaration<GroupingExpression>(expr);
    } else {
        throw this->throw_error("Invalid Expression");
    }
}

AST *Parser::function_call(std::string_view function_name) {
    std::vector<AST *> arguments;

    while (this->current().type != TokenType::PARENS_CLOSE) {
        arguments.push_back(this->expression());

        advanceIf(TokenType::COMMA);
    }

    advance(TokenType::PARENS_CLOSE);

    return this->create_declaration<FunctionCall>(function_name, arguments);
}

AST *Parser::token() {
    auto type = this->current().type;
    advance(type);
    return this->create_declaration<ASTToken>(type);
}

AST *Parser::type() {
    auto type = this->create_declaration<Type>(
        current().type, current().value(this->printer.source), false, false,
        false);

    if (!isRegularType(this->current().type)) {
        std::cout << tokenTypeAsLiteral(this->current().type) << std::endl;
        this->throw_error("Parser: Expected type\n");
    }

    advance();

    for (;;) {
        switch (this->current().type) {
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

AST *Parser::qualifiers() {
    auto qualifiers = getQualifiers();

    if (this->current().type == TokenType::ENUM) {
        return this->enum_declaration(qualifiers);
    } else if (this->current().type == TokenType::STRUCT) {
        return this->struct_declaration(qualifiers);
    }
    return function_or_variable_declaration(qualifiers);
}

std::vector<TokenType> Parser::getQualifiers() {
    std::vector<TokenType> modifiers;
    while (this->current().type == TokenType::EXTERN ||
           this->current().type == TokenType::VOLATILE ||
           this->current().type == TokenType::PRIVATE) {
        modifiers.push_back(this->current().type);
        advance();
    }
    return modifiers;
}

void Parser::advance() {
    this->index += 1;

    this->current() = this->tokens.at(this->index);
}

void Parser::advance(TokenType type) {
    if (this->current().type == type) {
        this->advance();
    } else {
        std::cout << tokenTypeAsLiteral(this->current().type)
                  << " :: " << tokenTypeAsLiteral(type)
                  << " :: " << this->current().location.toString() << std::endl;
        this->throw_error("Didn't Get Expected Token");
    }
}

bool Parser::advanceIf(TokenType type) {
    if (this->current().type == type) {
        this->advance();
        return true;
    }

    return false;
}

Token *Parser::getAndAdvance() {
    Token *token = &this->current();
    this->advance();
    return token;
}

Token *Parser::getAndAdvance(TokenType type) {
    Token *token = &this->current();
    this->advance(type);
    return token;
}

Token &Parser::peek(int offset) {
    return this->tokens.at(this->index + offset);
}

template <class ast_type, class... Args>
ast_type *Parser::create_declaration(Args &&...args) {
    return new ast_type(std::forward<Args>(args)..., this->current().location);
}

int Parser::throw_error(const char *message) {
    std::cout << "Error: " << message << std::endl;
    exit(-1);
}
