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

    // $(int a, bool b)
    if (this->current().type == TokenType::DOLLAR) {
        compoundStatement->first_class_function = this->first_class_function();
    }

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
    case TokenType::SELF:
        return this->function_or_variable_declaration({});
    case TokenType::FOR:
        return this->for_loop();
    case TokenType::WHILE:
        return this->while_loop();
    case TokenType::RETURN:
        return this->return_statement(false);
    case TokenType::THROW:
        return this->return_statement(true);
    case TokenType::IF:
        return this->if_statement();
    case TokenType::ASM:
        return this->inline_assembly();
    case TokenType::GOTO:
        return this->goto_statement();
    case TokenType::SWITCH:
        return this->switch_statement();
    case TokenType::DO:
        return this->do_catch_statement();
    case TokenType::BREAK:
    case TokenType::CONTINUE:
        return this->token();
    case TokenType::TYPEALIAS:
        return this->typealias();
    case TokenType::AT:
        return this->struct_initializer_declaration();
    case TokenType::BITWISE_NOT:
        return this->struct_deinitializer_declaration();
    case TokenType::PERIOD:
    case TokenType::OPERATOR_SUB:
        return this->expression();
    case TokenType::T_EOF:
        return nullptr;
    default:
        std::cout << tokenTypeAsLiteral(this->current().type);
        throw this->throw_error("Parser: Cannot Parse Token");
    }
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
        Parser::throw_error("Expected string literal\n");
    }

    return this->create_declaration<ImportStatement>(import_path, is_library);
}

StructDeclaration *
Parser::struct_declaration(const std::vector<TokenType> &qualifiers) {
    advance(TokenType::STRUCT);

    auto struct_name =
        getAndAdvance(TokenType::IDENTIFIER)->value(this->printer.source);

    std::optional<TemplateDeclaration *> template_ = std::nullopt;
    if (advanceIf(TokenType::COLON)) {
        template_ = this->template_declaration();
    }

    advance(TokenType::BRACE_OPEN);
    auto struct_body = this->compound();
    advance(TokenType::BRACE_CLOSE);

    return this->create_declaration<StructDeclaration>(struct_name, qualifiers,
                                                       struct_body, template_);
}

AST *Parser::struct_member_access() {
    std::string_view variable_name;
    if (this->current().type == TokenType::IDENTIFIER) {
        variable_name =
            getAndAdvance(TokenType::IDENTIFIER)->value(this->printer.source);
    } else if (this->current().type == TokenType::SELF) {
        variable_name = "self";
        advance(TokenType::SELF);
    } else {
        throw Parser::throw_error("Expected Identifier Or Self");
    }

    advance(TokenType::PERIOD);

    if (this->current().type == TokenType::IDENTIFIER) {
        auto identifier =
            getAndAdvance(TokenType::IDENTIFIER)->value(this->printer.source);

        if (identifier == "init") {
            advance(TokenType::PARENS_OPEN);
            auto arguments = function_call_arguments();
            advance(TokenType::PARENS_CLOSE);

            return this->create_declaration<StructInitializerCall>(
                variable_name, arguments);
        } else if (identifier == "deinit") {
            advance(TokenType::PARENS_OPEN);
            advance(TokenType::PARENS_CLOSE);

            return this->create_declaration<StructInitializerCall>(
                variable_name, true);
        } else {
            this->index -= 1;
            goto expression;
        }
    }

expression:
    auto struct_member = expression();

    return this->create_declaration<StructMemberAccess>(variable_name,
                                                        struct_member);
}

AST *Parser::struct_init_or_enum_case_access() {
    advance(TokenType::PERIOD);

    auto identifier =
        getAndAdvance(TokenType::IDENTIFIER)->value(this->printer.source);

    if (identifier == "init") {
        advance(TokenType::PARENS_OPEN);
        auto arguments = function_call_arguments();
        advance(TokenType::PARENS_CLOSE);

        return this->create_declaration<StructInitializerCall>(arguments);
    } else {
        return this->create_declaration<EnumCaseAccess>(identifier);
    }
}

StructInitializerDeclaration *Parser::struct_initializer_declaration() {
    advance(TokenType::AT);
    advance(TokenType::PARENS_OPEN);
    auto arguments = function_arguments();
    advance(TokenType::PARENS_CLOSE);

    advance(TokenType::BRACE_OPEN);
    auto body = compound();
    advance(TokenType::BRACE_CLOSE);

    return this->create_declaration<StructInitializerDeclaration>(arguments,
                                                                  body);
}

AST *Parser::struct_deinitializer_declaration() {
    advance(TokenType::BITWISE_NOT);
    if (!advanceIf(TokenType::PARENS_OPEN)) {
        this->index -= 1;
        return this->expression();
    }
    advance(TokenType::PARENS_CLOSE);

    advance(TokenType::BRACE_OPEN);
    auto body = compound();
    advance(TokenType::BRACE_CLOSE);

    return this->create_declaration<StructInitializerDeclaration>(body);
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
    const std::vector<TokenType> &qualifiers) {
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
                             const std::vector<TokenType> &qualifiers) {
    auto function_name =
        getAndAdvance(TokenType::IDENTIFIER)->value(this->printer.source);

    advance(TokenType::PARENS_OPEN);

    auto function_arguments = this->function_arguments();

    advance(TokenType::PARENS_CLOSE);

    std::optional<TemplateDeclaration *> template_ = std::nullopt;
    if (advanceIf(TokenType::COLON)) {
        template_ = this->template_declaration();
    }

    if (advanceIf(TokenType::BRACE_OPEN)) {
        auto function_body = this->compound();
        advance(TokenType::BRACE_CLOSE);
        return this->create_declaration<FunctionDeclaration>(
            qualifiers, return_type, function_name, function_arguments,
            function_body, template_);
    }

    if (template_) {
        throw Parser::throw_error(
            "Functions without body can't have template!");
    }

    return this->create_declaration<FunctionDeclaration>(
        qualifiers, return_type, function_name, function_arguments);
}

AST *Parser::variable_declaration(AST *&variable_type,
                                  const std::vector<TokenType> &qualifiers) {
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

Return *Parser::return_statement(bool is_throw_statement) {
    if (is_throw_statement) {
        advance(TokenType::THROW);
    } else {
        advance(TokenType::RETURN);
    }

    std::optional<AST *> return_value = std::nullopt;

    if (isRegularValue(this->current().type)) {
        return_value = this->expression();
    }

    return this->create_declaration<Return>(return_value, is_throw_statement);
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

    auto *case_body = this->create_declaration<CompoundStatement>();

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

DoCatchStatement *Parser::do_catch_statement() {
    advance(TokenType::DO);

    advance(TokenType::BRACE_OPEN);
    auto do_body = this->compound();
    advance(TokenType::BRACE_CLOSE);

    advance(TokenType::CATCH);
    std::optional<AST *> catch_expression = std::nullopt;
    if (advanceIf(TokenType::PARENS_OPEN)) {
        catch_expression = this->statement();
        advance(TokenType::PARENS_CLOSE);
    }

    advance(TokenType::BRACE_OPEN);
    auto catch_body = this->compound();
    advance(TokenType::BRACE_CLOSE);

    return this->create_declaration<DoCatchStatement>(do_body, catch_body,
                                                      catch_expression);
}

AST *Parser::expression() { return this->equality(); }

AST *Parser::try_expression() {
    advance(TokenType::TRY);
    bool is_force_cast = false;    // try!
    bool is_optional_cast = false; // try?

    if (advanceIf(TokenType::QUESTION)) {
        is_optional_cast = true;
    } else if (advanceIf(TokenType::NOT)) {
        is_force_cast = true;
    }

    auto expression = this->expression();

    return this->create_declaration<TryExpression>(expression, is_force_cast,
                                                   is_optional_cast);
}

AST *Parser::equality() {
    if (this->current().type == TokenType::TRY) {
        return try_expression();
    }
    auto expr = this->comparison();

    while (isEqualityOperator(this->current().type)) {
        TokenType operator_type = getAndAdvance()->type;

        auto right_ast = this->comparison();

        expr = this->create_declaration<BinaryExpression>(expr, right_ast,
                                                          operator_type);
    }

    return expr;
}

AST *Parser::comparison() {
    auto expr = this->term();

    while (isComparativeOperator(this->current().type)) {
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
    if (isUnaryOperator(this->current().type)) {
        TokenType operator_type = getAndAdvance()->type;

        auto expr = this->unary();

        return this->create_declaration<UnaryExpression>(expr, operator_type);
    }

    return this->primary();
}

AST *Parser::primary(bool parses_goto) {
    if (peek().type == TokenType::PERIOD) {
        return struct_member_access();
    }
    if (this->current().type == TokenType::PERIOD) {
        return struct_init_or_enum_case_access();
    }
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

        if (this->current().type == TokenType::AT) {
            auto template_arguments = this->template_call_arguments();

            advance(TokenType::PARENS_CLOSE);
            // TODO: Optional Templates
            advance(TokenType::PARENS_OPEN);

            // TODO: Just to function call for now, in the future, when there
            // will be nested structs, we might have to make this more complex
            return this->function_call(literal->value, template_arguments);
        }

        return literal;
    } else if (advanceIf(TokenType::PARENS_OPEN)) {
        auto expr = this->expression();
        advance(TokenType::PARENS_CLOSE);

        return this->create_declaration<GroupingExpression>(expr);
    } else if (this->current().type == TokenType::SELF) {
        return struct_member_access();
    } else {
        std::cout << tokenTypeAsLiteral(this->current().type);
        throw Parser::throw_error("Invalid Expression");
    }
}

AST *Parser::function_call(
    std::string_view function_name,
    std::optional<std::vector<AST *>> template_arguments) {

    auto arguments = function_call_arguments();
    advance(TokenType::PARENS_CLOSE);

    return this->create_declaration<FunctionCall>(function_name, arguments,
                                                  template_arguments);
}

std::vector<AST *> Parser::function_call_arguments() {
    std::vector<AST *> arguments;

    while (this->current().type != TokenType::PARENS_CLOSE) {
        if (advanceIf(TokenType::NOT)) {
            advance(TokenType::BRACE_OPEN);
            auto body = this->compound();
            arguments.push_back(body);
            advance(TokenType::BRACE_CLOSE);
            advanceIf(TokenType::COMMA);
            continue;
        }

        arguments.push_back(this->expression());
        advanceIf(TokenType::COMMA);
    }

    return arguments;
}

AST *Parser::typealias() {
    advance(TokenType::TYPEALIAS);
    auto type_name =
        getAndAdvance(TokenType::IDENTIFIER)->value(this->printer.source);
    advance(TokenType::EQUAL);
    auto value = expression();

    return this->create_declaration<Typealias>(type_name, value);
}

AST *Parser::token() {
    auto type = this->current().type;
    advance(type);
    return this->create_declaration<ASTToken>(type);
}

TemplateDeclaration *Parser::template_declaration() {
    advance(TokenType::AT);
    advance(TokenType::PARENS_OPEN);

    auto arguments = template_arguments();

    advance(TokenType::PARENS_CLOSE);

    return this->create_declaration<TemplateDeclaration>(arguments);
}

std::vector<TemplateDeclarationArgument *> Parser::template_arguments() {
    std::vector<TemplateDeclarationArgument *> arguments;

    while (this->current().type != TokenType::PARENS_CLOSE) {
        if (!isTemplateKeyword(this->current().type)) {
            throw Parser::throw_error("Invalid Template Type");
        }

        auto argument_type = getAndAdvance()->type;
        auto argument_name =
            getAndAdvance(TokenType::IDENTIFIER)->value(this->printer.source);

        auto argument = this->create_declaration<TemplateDeclarationArgument>(
            argument_name, argument_type);

        arguments.push_back(argument);

        if (!advanceIf(TokenType::COMMA)) {
            break;
        }
    }

    return arguments;
}

std::vector<AST *> Parser::template_call_arguments() {
    this->advance(TokenType::AT);
    advance(TokenType::PARENS_OPEN);

    std::vector<AST *> template_values;
    while (this->current().type != TokenType::PARENS_CLOSE) {
        template_values.push_back(this->type());

        if (!advanceIf(TokenType::COMMA)) {
            break;
        }
    }

    return template_values;
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

        AST *argument_type = this->type();
        std::optional<std::string_view> argument_name;

        if (this->current().type == TokenType::IDENTIFIER) {
            argument_name = getAndAdvance(TokenType::IDENTIFIER)
                                ->value(this->printer.source);
        }

        auto argument = this->create_declaration<FunctionArgument>(
            argument_name, argument_type);

        arguments.push_back(argument);

        if (!advanceIf(TokenType::COMMA)) {
            break;
        }
    }

    return arguments;
}

FirstClassFunction *Parser::first_class_function() {
    // $bool(int, string)
    advance(TokenType::DOLLAR);

    bool is_type = true;
    std::optional<AST *> return_type = std::nullopt;
    if (this->current().type == TokenType::PARENS_OPEN) {
        is_type = false;
    } else {
        return_type = this->type();
    }
    advance(TokenType::PARENS_OPEN);
    auto arguments = this->function_arguments();
    advance(TokenType::PARENS_CLOSE);

    return this->create_declaration<FirstClassFunction>(return_type, arguments,
                                                        is_type);
}

AST *Parser::type() {
    auto type = this->create_declaration<Type>(
        current().type, current().value(this->printer.source), false, false,
        false);

    if (!isRegularType(this->current().type)) {
        std::cout << tokenTypeAsLiteral(this->current().type) << std::endl;
        Parser::throw_error("Parser: Expected type\n");
    }

    if (this->current().type == TokenType::DOLLAR) {
        return this->first_class_function();
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
        case TokenType::AT:
            type->template_values = template_call_arguments();
            advance(TokenType::PARENS_CLOSE);
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
        Parser::throw_error("Didn't Get Expected Token");
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
    advanceIf(TokenType::SEMICOLON);
    return new ast_type(std::forward<Args>(args)..., this->current().location);
}

int Parser::throw_error(const char *message) {
    std::cout << "Error: " << message << std::endl;
    exit(-1);
}
