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
    auto old_compound = current_compound;
    current_compound = compoundStatement;

    // $(int a, bool b)
    if (this->current().type == TokenType::DOLLAR) {
        if (!inside_function_body) {
            throw throw_error("The First Class Function Parameter must be "
                              "inside a valid body.");
        }
        compoundStatement->first_class_function = this->first_class_function();
    }

    advanceLines();

    while (this->current().type != TokenType::BRACE_CLOSE &&
           this->current().type != TokenType::T_EOF) {
        auto statement = this->statement();
        compoundStatement->statements.push_back(statement);

        if (this->current().type != TokenType::T_EOF) {
            if (this->current().type != TokenType::NEW_LINE) {
                advance(TokenType::SEMICOLON,
                        "Expected new line or semicolon after statement.");
            } else {
                advance(TokenType::NEW_LINE,
                        "Expected new line or semicolon after statement.");
            }

            advanceLines();
        }
    }

    for (int i = 0; i < compoundStatement->declaration_names.size(); ++i) {
        for (int j = i + 1; j < compoundStatement->declaration_names.size();
             ++j) {
            if (compoundStatement->declaration_names[i].first ==
                compoundStatement->declaration_names[j].first) {
                std::string error = "Found duplicate declaration '";
                error += compoundStatement->declaration_names[i].first;
                error += "'";
                this->throw_error(
                    error.c_str(),
                    compoundStatement->declaration_names[j].second);
            }
        }
    }

    current_compound = old_compound;

    return compoundStatement;
}

AST *Parser::statement() {
    switch (this->current().type) {
    case TokenType::IMPORT:
        return import();
    case TokenType::ENUM:
        return this->enum_declaration();
    case TokenType::STRUCT:
    case TokenType::UNION:
        return this->struct_declaration();
    case TokenType::EXTERN:
    case TokenType::VOLATILE:
    case TokenType::PRIVATE:
        return qualifiers();
    case TokenType::IDENTIFIER:
    case TokenType::SELF:
    case TokenType::ANY:
        return this->expression();
    case TokenType::FUNC:
        return this->function_declaration();
    case TokenType::VAR:
        return variable_declaration();
    case TokenType::LET:
        return variable_declaration({}, true);
    case TokenType::V_INT:
    case TokenType::V_CHAR:
    case TokenType::V_FLOAT:
    case TokenType::V_STRING:
        return expression();
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
        return this->struct_destructor_declaration();
    case TokenType::PERIOD:
    case TokenType::OPERATOR_SUB:
        return this->expression();
    case TokenType::NEW_LINE:
        throw this->throw_error(
            "Parser: Error with '\\n' parsing, please create a Github issue.");
        return nullptr;
    case TokenType::T_EOF:
        return nullptr;
    default:
        std::cout << tokenTypeAsLiteral(this->current().type) << " "
                  << this->current().location.toString();
        throw this->throw_error("Parser: Cannot parse token.");
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
        this->throw_error(
            "Expected string literal or library name after import.");
    }

    return this->create_declaration<ImportStatement>(import_path, is_library);
}

StructDeclaration *
Parser::struct_declaration(const std::vector<TokenType> &qualifiers) {
    bool is_union = false;
    if (this->current().type == TokenType::UNION) {
        advance(TokenType::UNION);
        is_union = true;
    } else {
        advance(TokenType::STRUCT);
    }

    auto struct_name =
        getAndAdvance(TokenType::IDENTIFIER,
                      "Expected a struct or a union name after declaration.")
            ->value(this->printer.source);

    std::optional<TemplateDeclaration *> template_ = std::nullopt;

    if (!is_union) {
        if (advanceIf(TokenType::COLON)) {
            template_ = this->template_declaration();
        }
    }

    advance(TokenType::BRACE_OPEN, "The struct declaration must have a body.");
    is_parsing_struct = true;
    auto struct_body = this->compound();
    is_parsing_struct = false;
    advance(TokenType::BRACE_CLOSE,
            "The struct declaration must have a '}' in order to close it.");

    return this->create_declaration<StructDeclaration>(
        struct_name, qualifiers, struct_body, template_, is_union);
}

AST *Parser::struct_member_access() {
    std::string_view variable_name;
    if (this->current().type == TokenType::IDENTIFIER) {
        variable_name =
            getAndAdvance(TokenType::IDENTIFIER)->value(this->printer.source);
    } else if (advanceIf(TokenType::SELF)) {
        variable_name = "self";
    } else {
        throw this->throw_error(
            "Expected a identifier or a self after struct member access.");
    }

    advance(TokenType::PERIOD);

    if (this->current().type == TokenType::IDENTIFIER) {
        auto identifier =
            getAndAdvance(TokenType::IDENTIFIER)->value(this->printer.source);

        if (identifier == "init") {
            advance(TokenType::PARENS_OPEN,
                    "Expected initializer to have arguments.");
            auto arguments = function_call_arguments();
            advance(TokenType::PARENS_CLOSE,
                    "Expected close parenthesis after initializer.");

            return this->create_declaration<StructInitializerCall>(
                variable_name, arguments);
        } else if (identifier == "deinit") {
            advance(TokenType::PARENS_OPEN,
                    "Expected the deinitializer to have a open parenthesis.");
            advance(TokenType::PARENS_CLOSE,
                    "Expected the deinitializer to have a close parenthesis.");

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

AST *Parser::struct_member_access(FunctionCall *function_call) {
    advance(TokenType::PERIOD);

    auto struct_member = expression();

    return this->create_declaration<StructMemberAccess>(function_call,
                                                        struct_member);
}

AST *Parser::struct_init_or_enum_case_access() {
    advance(TokenType::PERIOD);

    auto identifier =
        getAndAdvance(
            TokenType::IDENTIFIER,
            "Expected a identifier after struct initializer or enum case.")
            ->value(this->printer.source);

    if (identifier == "init") {
        advance(TokenType::PARENS_OPEN,
                "Expected the initializer to have arguments.");
        auto arguments = function_call_arguments();
        advance(TokenType::PARENS_CLOSE,
                "Expected close parenthesis after the initializer.");

        return this->create_declaration<StructInitializerCall>(arguments);
    } else {
        return this->create_declaration<EnumCaseAccess>(identifier);
    }
}

StructInitializerDeclaration *Parser::struct_initializer_declaration() {
    advance(TokenType::AT);
    advance(TokenType::PARENS_OPEN, "Expected the initializer to have arguments.");
    auto arguments = function_arguments();
    advance(TokenType::PARENS_CLOSE,
            "Expected close parenthesis after the initialize.r");

    advance(TokenType::BRACE_OPEN, "Expected a body after the struct initializer.");
    auto body = compound();
    advance(TokenType::BRACE_CLOSE, "The struct initializer body must be closed.");

    return this->create_declaration<StructInitializerDeclaration>(arguments,
                                                                  body);
}

AST *Parser::struct_destructor_declaration() {
    advance(TokenType::BITWISE_NOT);
    if (!advanceIf(TokenType::PARENS_OPEN)) {
        this->index -= 1;
        return this->expression();
    }
    advance(TokenType::PARENS_CLOSE,
            "Expected close parenthesis after the destructor.");

    advance(TokenType::BRACE_OPEN, "The destructor must have body.");
    auto body = compound();
    advance(TokenType::BRACE_CLOSE, "The destructor body must be closed.");

    return this->create_declaration<StructInitializerDeclaration>(body);
}

EnumDeclaration *
Parser::enum_declaration(const std::vector<TokenType> &qualifiers) {
    advance(TokenType::ENUM);

    auto enum_name =
        getAndAdvance(TokenType::IDENTIFIER, "Expected a enum name.")
            ->value(this->printer.source);

    advance(TokenType::BRACE_OPEN, "The enum must have body.");
    advanceLines();
    auto cases = enum_cases();
    advance(TokenType::BRACE_CLOSE, "The enum body must be closed.");

    if (should_check_duplicates) {
        current_compound->declaration_names.emplace_back(
            enum_name, this->current().location);
    }
    return this->create_declaration<EnumDeclaration>(enum_name, cases,
                                                     qualifiers);
}

std::vector<EnumCase *> Parser::enum_cases() {
    std::vector<EnumCase *> enum_cases;

    for (int enum_case_value = 0;
         this->current().type != TokenType::BRACE_CLOSE; enum_case_value++) {
        auto case_name =
            getAndAdvance(TokenType::IDENTIFIER, "THe enum case must have a name.")
                ->value(this->printer.source);

        AST *case_value;

        if (advanceIf(TokenType::EQUAL)) {
            case_value = this->expression();
        } else {
            auto case_value_string = std::to_string(enum_case_value);

            case_value = this->create_declaration<LiteralExpression>(
                case_value_string, TokenType::V_INT);
        }

        auto enum_case =
            this->create_declaration<EnumCase>(case_name, case_value);

        enum_cases.push_back(enum_case);
        advanceIf(TokenType::COMMA);
        advanceLines();
    }

    return enum_cases;
}

FunctionDeclaration *
Parser::function_declaration(const std::vector<TokenType> &qualifiers) {
    // func main(): i64 { ... }
    advance(TokenType::FUNC);

    auto function_name =
        getAndAdvance(TokenType::IDENTIFIER, "Functions must have a name.")
            ->value(this->printer.source);

    advance(TokenType::PARENS_OPEN, "Functions must have arguments.");

    auto function_arguments = this->function_arguments();

    advance(TokenType::PARENS_CLOSE, "Function's arguments must be closed.");

    std::optional<AST *> return_type = std::nullopt;
    if (advanceIf(TokenType::COLON)) {
        return_type = type();
    }

    std::optional<TemplateDeclaration *> template_ = std::nullopt;
    if (advanceIf(TokenType::BITWISE_PIPE_PIPE)) {
        template_ = this->template_declaration();
    }

    if (should_check_duplicates) {
        current_compound->declaration_names.emplace_back(
            function_name, this->current().location);
    }

    if (advanceIf(TokenType::BRACE_OPEN)) {
        auto function_body = this->compound();
        advance(TokenType::BRACE_CLOSE, "Function's body must be closed.");
        return this->create_declaration<FunctionDeclaration>(
            qualifiers, return_type, function_name, function_arguments,
            function_body, template_);
    }

    if (template_) {
        throw this->throw_error("Functions without a body can't have a template!");
    }

    return this->create_declaration<FunctionDeclaration>(
        qualifiers, return_type, function_name, function_arguments);
}

AST *Parser::variable_declaration(const std::vector<TokenType> &qualifiers,
                                  bool is_let) {
    is_let ? advance(TokenType::LET) : advance(TokenType::VAR);

    auto variable_name = getAndAdvance(TokenType::IDENTIFIER,
                                       "Expected a variable or constant name.")
                             ->value(this->printer.source);

    if (should_check_duplicates) {
        current_compound->declaration_names.emplace_back(
            variable_name, this->current().location);
    }

    std::optional<AST *> variable_value = std::nullopt;
    std::optional<AST *> variable_type = std::nullopt;
    if (advanceIf(TokenType::EQUAL)) {
        variable_value = this->expression();
    } else if (advanceIf(TokenType::COLON)) {
        variable_type = this->type();

        if (advanceIf(TokenType::EQUAL)) {
            variable_value = this->expression();
        }
    } else {
        throw this->throw_error(
            "Expected ':' or '=' after variable or constant declaration.");
    }

    if (!is_parsing_struct && !variable_value) {
        throw this->throw_error(
            "Uninitialized variable or constant declaration.");
    }

    return this->create_declaration<VariableDeclaration>(
        variable_name, variable_type, variable_value, qualifiers, is_let);
}

RangeBasedForLoop *Parser::range_based_for_loop() {
    auto name = getAndAdvance(TokenType::IDENTIFIER,
                              "Expected a name for range-based for loops.")
                    ->value(this->printer.source);
    advance(TokenType::IN, "Range-based for loops must have `in` token.");

    auto name2 = expression();
    this->advance(TokenType::PARENS_CLOSE,
                  "Expected a closing parenthesis after range-based for loops.");

    std::optional<AST *> for_index = std::nullopt;
    if (advanceIf(TokenType::BITWISE_PIPE)) {
        for_index = statement();
        advance(TokenType::BITWISE_PIPE,
                "Expected a closing pipe operator after range-based for loops.");
    }

    this->advance(TokenType::BRACE_OPEN,
                  "Range-based for loops must have a body.");
    auto for_body = this->compound();
    this->advance(TokenType::BRACE_CLOSE,
                  "Range-based for loops must have a body that closes.");

    return this->create_declaration<RangeBasedForLoop>(name, name2, for_index,
                                                       for_body);
}

AST *Parser::for_loop() {
    this->advance(TokenType::FOR);

    this->advance(TokenType::PARENS_OPEN,
                  "Expected a opening parenthesis after a for loop.");

    if (peek().type == TokenType::IN) {
        return this->range_based_for_loop();
    }

    should_check_duplicates = false;
    auto for_initialization = this->statement();
    this->advance(TokenType::COMMA,
                  "Expected a comma after for loop's first expression.");

    auto for_condition = this->expression();
    this->advance(TokenType::COMMA,
                  "Expected a comma after for loop's second expression.");

    auto for_increment = this->statement();

    should_check_duplicates = true;

    this->advance(TokenType::PARENS_CLOSE,
                  "The for loop's parenthesis must be closed.");

    this->advance(TokenType::BRACE_OPEN, "The for loop must have a body.");
    auto for_body = this->compound();
    this->advance(TokenType::BRACE_CLOSE,
                  "The for loop's body must be closed.");

    return this->create_declaration<ForLoop>(for_initialization, for_condition,
                                             for_increment, for_body);
}

WhileLoop *Parser::while_loop() {
    advance(TokenType::WHILE);

    advance(TokenType::PARENS_OPEN, "The while loop must have a parenthesis.");
    auto expression = this->expression();
    advance(TokenType::PARENS_CLOSE,
            "While loop's parenthesis must be closed.");

    this->advance(TokenType::BRACE_OPEN, "The while loop must have a body.");
    auto body = this->compound();
    this->advance(TokenType::BRACE_CLOSE, "The while loop's body must be closed.");

    return this->create_declaration<WhileLoop>(expression, body);
}

Return *Parser::return_statement(bool is_throw_statement) {
    if (is_throw_statement) {
        advance(TokenType::THROW);
    } else {
        advance(TokenType::RETURN);
    }

    std::optional<AST *> return_value = std::nullopt;

    if (this->current().type != TokenType::NEW_LINE) {
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
            advance(TokenType::BRACE_OPEN, "The else statement must have a body.");
            else_body = this->compound();
            advance(TokenType::BRACE_CLOSE,
                    "The else statement's body must be closed.");
            break;
        }
    }

    return this->create_declaration<If>(
        if_body_and_statement.first, if_body_and_statement.second,
        elseif_conditions, elseif_bodies, else_body);
}

std::pair<AST *, CompoundStatement *> Parser::if_else_statements() {
    advance(TokenType::PARENS_OPEN,
            "Expected a expression after a if or else statement.");
    auto condition = this->expression();
    advance(TokenType::PARENS_CLOSE,
            "Parenthesis must be closed inside an if statement.");

    advance(TokenType::BRACE_OPEN, "The if or else statement must have a body.");
    auto body = this->compound();
    advance(TokenType::BRACE_CLOSE,
            "The if or else statement's body must be closed.");

    return std::make_pair(condition, body);
}

ASM *Parser::inline_assembly() {
    advance(TokenType::ASM);
    advance(TokenType::PARENS_OPEN, "Expected a parenthesis after assembly.");
    advanceLines();

    std::vector<std::string_view> instructions;

    while (this->current().type != TokenType::PARENS_CLOSE) {
        instructions.push_back(
            getAndAdvance(TokenType::V_STRING,
                          "The assembly instruction must be inside a string literal.")
                ->value(this->printer.source));
        advanceLines();
    }

    advance(TokenType::PARENS_CLOSE, "Assembly must be closed.");

    return this->create_declaration<ASM>(instructions);
}

GOTO *Parser::goto_statement() {
    advance(TokenType::GOTO);

    auto label = getAndAdvance(TokenType::IDENTIFIER,
                               "Expected a label after the goto statement.")
                     ->value(this->printer.source);

    return this->create_declaration<GOTO>(label, true);
}

SwitchStatement *Parser::switch_statement() {
    advance(TokenType::SWITCH);

    advance(TokenType::PARENS_OPEN,
            "The switch statement must have an opening parenthesis.");
    auto switch_expression = expression();
    advance(TokenType::PARENS_CLOSE,
            "The switch statement must have a closing parenthesis.");

    advance(TokenType::BRACE_OPEN, "The switch statement must have a body.");
    advanceLines();
    auto cases = switch_cases();
    advance(TokenType::BRACE_CLOSE,
            "The body of the switch statement must end with a '}'.");

    return this->create_declaration<SwitchStatement>(switch_expression, cases);
}

std::vector<SwitchCase *> Parser::switch_cases() {
    std::vector<SwitchCase *> cases;

    while (this->current().type != TokenType::BRACE_CLOSE) {
        cases.push_back(switch_case());
        advanceLines();
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
        parses_goto_labels = false;
        case_expression = primary();
        parses_goto_labels = true;
    }

    advance(TokenType::COLON, "Expected a ':' after the switch case.");

    auto *case_body = this->create_declaration<CompoundStatement>();

    advanceLines();
    while (this->current().type != TokenType::CASE &&
           this->current().type != TokenType::DEFAULT &&
           this->current().type != TokenType::BRACE_CLOSE) {
        case_body->statements.push_back(statement());
        advanceLines();
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

    advance(TokenType::BRACE_OPEN, "Expected a '(' after the do statement case.");
    auto do_body = this->compound();
    advance(TokenType::BRACE_CLOSE, "Expected a ')' after the do statement case.");

    advance(TokenType::CATCH, "The do statement must have a catch clause.");
    std::optional<AST *> catch_expression = std::nullopt;
    if (advanceIf(TokenType::PARENS_OPEN)) {
        catch_expression = this->statement();
        advance(TokenType::PARENS_CLOSE,
                "Expected a ')' after the catch statement expression.");
    }

    advance(TokenType::BRACE_OPEN, "The catch statement must have a body.");
    auto catch_body = this->compound();
    advance(TokenType::BRACE_CLOSE, "The catch statement's body must end.");

    return this->create_declaration<DoCatchStatement>(do_body, catch_body,
                                                      catch_expression);
}

OperatorOverload *Parser::operator_overload(AST *&return_type) {
    /*
     int :: operator[](float offset) {
        return self.items[offset]
     }
     */
    advance(TokenType::OPERATOR);
    std::vector<TokenType> operators;

    while (this->current().type != TokenType::PARENS_OPEN) {
        if (!isOperatorOverloadType(this->current().type)) {
            throw this->throw_error("Invalid Operator");
        }

        operators.push_back(this->current().type);
        advance();
    }

    advance(TokenType::PARENS_OPEN, "Expected a '(' after the operator overloading.");
    auto arguments = this->function_arguments();
    advance(TokenType::PARENS_CLOSE,
            "Expected a ')' after operator overloading.");

    advance(TokenType::BRACE_OPEN, "Expected a body after the operator overloading.");
    auto body = this->compound();
    advance(TokenType::BRACE_CLOSE,
            "Body after operator overloading must end.");

    return this->create_declaration<OperatorOverload>(return_type, operators,
                                                      arguments, body);
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

AST *Parser::cast_expression() {
    // cast?(50.3, int) || cast!(50.3, int)
    advance(TokenType::CAST);

    bool is_force_cast = false;
    bool is_optional_cast = false;

    if (advanceIf(TokenType::QUESTION)) {
        is_optional_cast = true;
    } else if (advanceIf(TokenType::NOT)) {
        is_force_cast = true;
    } else {
        throw this->throw_error("The cast expression must be forced or optional.");
    }

    advance(TokenType::PARENS_OPEN, "THe cast expression must have '('.");
    auto cast_value = expression();
    advance(TokenType::COMMA,
            "The cast expression must have a comma after expression.");
    auto type = this->type();
    advance(TokenType::PARENS_CLOSE,
            "Expected a ')' at the end of the cast expression.");

    return this->create_declaration<CastExpression>(
        cast_value, type, is_force_cast, is_optional_cast);
}

TernaryExpression *Parser::ternary_expression(AST *bool_expression) {
    // (expression) ? 50 : 20
    // 50 == 30 ? return true : return false
    advance(TokenType::QUESTION);

    parses_goto_labels = false;
    auto first_expression = statement();
    advance(TokenType::COLON, "Expected a ':' after the ternary expression.");
    auto second_expression = statement();
    parses_goto_labels = true;

    return this->create_declaration<TernaryExpression>(
        bool_expression, first_expression, second_expression);
}

OptionalUnwrapExpression *Parser::optional_unwrap(AST *expression) {
    // myOptionalVariable ?? 40
    advance(TokenType::QUESTION);
    advance(TokenType::QUESTION);

    auto if_nilled_value = this->expression();

    return this->create_declaration<OptionalUnwrapExpression>(expression,
                                                              if_nilled_value);
}

ForceUnwrapExpression *Parser::force_unwrap(AST *expression) {
    advance(TokenType::NOT);
    return this->create_declaration<ForceUnwrapExpression>(expression);
}

AST *Parser::equality() {
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

AST *Parser::primary() {
    if (this->current().type == TokenType::TRY) {
        return try_expression();
    }

    if (peek().type == TokenType::PERIOD) {
        return struct_member_access();
    } else if (peek().type == TokenType::SQUARE_OPEN) {
        return array_access();
    }

    if (this->current().type == TokenType::SQUARE_OPEN) {
        return array_creation();
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
        } else if (parses_goto_labels) {
            if (advanceIf(TokenType::COLON)) {
                return this->create_declaration<GOTO>(literal->value, false);
            }
        }

        if (this->current().type == TokenType::AT) {
            auto template_arguments = this->template_call_arguments();

            advance(TokenType::PARENS_CLOSE,
                    "Expected a ')' after the function call template arguments.");
            // TODO: Optional Templates
            advance(TokenType::PARENS_OPEN,
                    "Expected a '(' after the function call template arguments.");

            // TODO: Just to function call for now, in the future, when there
            // will be nested structs, we might have to make this more complex
            return this->function_call(literal->value, template_arguments);
        }

        if (this->current().type == TokenType::QUESTION) {
            if (peek().type == TokenType::QUESTION) {
                return optional_unwrap(literal);
            }
            return ternary_expression(literal);
        }

        if (this->current().type == TokenType::NOT) {
            return force_unwrap(literal);
        }

        return literal;
    } else if (advanceIf(TokenType::PARENS_OPEN)) {
        auto expr = this->expression();
        advance(TokenType::PARENS_CLOSE,
                "Expected a closing parenthesis after group expression.");

        return this->create_declaration<GroupingExpression>(expr);
    } else if (this->current().type == TokenType::SELF) {
        return struct_member_access();
    } else if (this->current().type == TokenType::CAST) {
        return cast_expression();
    } else {
        std::cout << tokenTypeAsLiteral(this->current().type);
        throw this->throw_error("Invalid expression.");
    }
}

AST *Parser::function_call(
    std::string_view function_name,
    const std::optional<std::vector<AST *>> &template_arguments) {

    auto arguments = function_call_arguments();
    advance(TokenType::PARENS_CLOSE,
            "Expected a closing parenthesis after function call.");

    auto decl = this->create_declaration<FunctionCall>(function_name, arguments,
                                                       template_arguments);

    if (this->current().type == TokenType::PERIOD) {
        return this->struct_member_access(decl);
    } else if (this->current().type == TokenType::SQUARE_OPEN) {
        return array_access(decl);
    }

    return decl;
}

AST *Parser::array_access() {
    // myVariable[40]
    auto variable_name = getAndAdvance(TokenType::IDENTIFIER,
                                       "Expecteda  identifier after array access.")
                             ->value(this->printer.source);
    advance(TokenType::SQUARE_OPEN);
    auto inside = this->expression();
    advance(TokenType::SQUARE_CLOSE,
            "Expected a closing square after array access.");

    return this->create_declaration<ArrayAccess>(variable_name, inside);
}

AST *Parser::array_access(FunctionCall *function_call) {
    // myFunction()[30]
    advance(TokenType::SQUARE_OPEN);
    auto inside = this->expression();
    advance(TokenType::SQUARE_CLOSE,
            "Expected a closing square after array access.");

    return this->create_declaration<ArrayAccess>(function_call, inside);
}

AST *Parser::array_creation() {
    // [50, 30, 20]
    advance(TokenType::SQUARE_OPEN);

    std::vector<AST *> items;
    while (this->current().type != TokenType::SQUARE_CLOSE) {
        items.push_back(this->expression());

        advanceIf(TokenType::COMMA);
    }

    advance(TokenType::SQUARE_CLOSE,
            "Expected a closing square after array creation.");

    return this->create_declaration<ArrayCreation>(items);
}

std::vector<AST *> Parser::function_call_arguments() {
    std::vector<AST *> arguments;

    this->parses_goto_labels = false;
    bool argument_can_have_unnamed_arguments = true;
    while (this->current().type != TokenType::PARENS_CLOSE) {
        if (advanceIf(TokenType::NOT)) {
            if (this->current().type != TokenType::BRACE_OPEN) {
                goto argument_expression;
            }
            inside_function_body = true;
            advance(TokenType::BRACE_OPEN,
                    "Expected a '{' after using first class function.");
            auto body = this->compound();
            arguments.push_back(body);
            advance(TokenType::BRACE_CLOSE,
                    "Expected a '}' after using first class function.");
            advanceIf(TokenType::COMMA);
            inside_function_body = false;
            continue;
        }

        if (peek().type == TokenType::COLON) {
            // name based argument
            auto argument_name =
                getAndAdvance(TokenType::IDENTIFIER,
                              "Expected a identifier after using a name based "
                              "argument style")
                    ->value(this->printer.source);
            advance(TokenType::COLON);
            auto argument_value = this->expression();

            auto argument =
                this->create_declaration<FunctionCallNameBasedArgument>(
                    argument_name, argument_value);

            argument_can_have_unnamed_arguments = false;
            arguments.push_back(argument);
        } else {
        argument_expression:
            if (!argument_can_have_unnamed_arguments) {
                throw throw_error("Cannot use unnamed arguments after "
                                  "using a named argument");
            }
            arguments.push_back(this->expression());
        }

        advanceIf(TokenType::COMMA);
    }
    this->parses_goto_labels = true;

    return arguments;
}

AST *Parser::typealias() {
    advance(TokenType::TYPEALIAS);
    auto type_name = getAndAdvance(TokenType::IDENTIFIER,
                                   "Expected a type name after the type alias.")
                         ->value(this->printer.source);
    advance(TokenType::EQUAL, "Expected a type after the type alias.");
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
    advance(TokenType::PARENS_OPEN,
            "Expected a '(' when using template declarations.");

    auto arguments = template_arguments();

    advance(TokenType::PARENS_CLOSE,
            "Expected a ')' when using template declarations.");

    return this->create_declaration<TemplateDeclaration>(arguments);
}

std::vector<TemplateDeclarationArgument *> Parser::template_arguments() {
    std::vector<TemplateDeclarationArgument *> arguments;

    while (this->current().type != TokenType::PARENS_CLOSE) {
        if (!isTemplateKeyword(this->current().type)) {
            throw this->throw_error("Invalid template type.");
        }

        auto argument_type = getAndAdvance()->type;
        auto argument_name = getAndAdvance(TokenType::IDENTIFIER,
                                           "Expected a template argument name.")
                                 ->value(this->printer.source);

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
    advance(TokenType::PARENS_OPEN, "Expected a '(' when using template calls.");

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

    bool can_use_arguments_without_default_value = true;
    while (this->current().type != TokenType::PARENS_CLOSE) {
        if (advanceIf(TokenType::PERIOD)) {
            advance(TokenType::PERIOD, "Expected a '..' when using vaargs.");
            advance(TokenType::PERIOD, "Expected a '.' when using vaargs.");

            auto argument = this->create_declaration<FunctionArgument>();
            argument->is_vaarg = true;
            argument->name =
                getAndAdvance(TokenType::IDENTIFIER, "Expected a vaarg name.")
                    ->value(this->printer.source);
            arguments.push_back(argument);

            break;
        }

        AST *argument_type = this->type();
        std::optional<std::string_view> argument_name;
        std::optional<AST *> argument_default_value = std::nullopt;

        if (this->current().type == TokenType::IDENTIFIER) {
            argument_name = getAndAdvance(TokenType::IDENTIFIER)
                                ->value(this->printer.source);

            if (advanceIf(TokenType::EQUAL)) {
                can_use_arguments_without_default_value = false;
                argument_default_value = this->expression();
            }
        }

        if (!can_use_arguments_without_default_value &&
            !argument_default_value) {
            throw throw_error("Move the argument with a default value to the "
                              "end of the argument list, since you have "
                              "already specified a default value.");
        }

        auto argument = this->create_declaration<FunctionArgument>(
            argument_name, argument_type, argument_default_value);

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
    advance(TokenType::PARENS_OPEN,
            "Expected a '(' at the start of a first class function.");
    auto arguments = this->function_arguments();
    advance(TokenType::PARENS_CLOSE,
            "Expected a ')' at the end of a first class function.");

    return this->create_declaration<FirstClassFunction>(return_type, arguments,
                                                        is_type);
}

AST *Parser::type() {
    auto type = this->create_declaration<Type>(
        current().type, current().value(this->printer.source), false, false,
        false, false);

    if (!isRegularType(this->current().type)) {
        std::cout << tokenTypeAsLiteral(this->current().type) << std::endl;
        this->throw_error("Parser: Expected type\n");
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
        case TokenType::NOT:
            type->is_throw_statement = true;
            break;
        case TokenType::AT:
            type->template_values = template_call_arguments();
            advance(TokenType::PARENS_CLOSE,
                    "Expected a closing template argument.");
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
    } else if (this->current().type == TokenType::STRUCT ||
               this->current().type == TokenType::UNION) {
        return this->struct_declaration(qualifiers);
    } else if (this->current().type == TokenType::VAR) {
        return this->variable_declaration(qualifiers);
    } else if (this->current().type == TokenType::LET) {
        return variable_declaration(qualifiers, true);
    } else if (this->current().type == TokenType::FUNC) {
        return this->function_declaration(qualifiers);
    }
    throw this->throw_error("Expected enum, struct, function or variable.");
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

void Parser::advance(TokenType type, const char *error_message) {
    if (this->current().type == type) {
        this->advance();
    } else {
        //        std::cout << tokenTypeAsLiteral(this->current().type)
        //                  << " :: " << tokenTypeAsLiteral(type)
        //                  << " :: " << this->current().location.toString() <<
        //                  std::endl;
        this->throw_error(error_message);
    }
}

bool Parser::advanceIf(TokenType type) {
    if (this->current().type == type) {
        this->advance();
        return true;
    }

    return false;
}

void Parser::advanceLines() {
    while (this->current().type == TokenType::NEW_LINE) {
        advance();
    }
}

Token *Parser::getAndAdvance() {
    Token *token = &this->current();
    this->advance();
    return token;
}

Token *Parser::getAndAdvance(TokenType type, const char *message) {
    Token *token = &this->current();
    this->advance(type, message);
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
    printer.error(message, this->current().location);
    return -1;
}

int Parser::throw_error(const char *message, Location location) {
    printer.error(message, location);
    return -1;
}