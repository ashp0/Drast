//
// Created by Ashwin Paudel on 2022-03-26.
//

#include "Parser.h"

CompoundStatement *Parser::parse() {
    auto compound = this->compound();

    //    std::cout << compound->toString() << std::endl;

    return compound;
}

CompoundStatement *Parser::compound() {
    auto compoundStatement = this->makeDeclaration<CompoundStatement>();
    auto old_compound = current_compound;
    current_compound = compoundStatement;

    // $(int a, bool b)
    if (this->current().type == TokenType::DOLLAR) {
        if (!inside_function_body) {
            throwError("The first class function parameter must be "
                       "inside a valid body.");
        }
        compoundStatement->first_class_function = this->firstClassFunction();
    }

    advanceLines();

    while (this->current().type != TokenType::BRACE_CLOSE &&
           this->current().type != TokenType::T_EOF) {
        auto statement = this->statement();
        compoundStatement->statements.push_back(statement);

        if (this->current().type != TokenType::T_EOF) {
            if (this->current().type != TokenType::NEW_LINE) {
                if (this->current().type != TokenType::SEMICOLON) {
                    std::cout << "HEllo1: " << statement->toString()
                              << std::endl;
                }
                advance(TokenType::SEMICOLON,
                        "Expected a new line or a semicolon after statement.");
            } else {
                if (this->current().type != TokenType::NEW_LINE) {
                    std::cout << "HEllo1: " << statement->toString()
                              << std::endl;
                }
                advance(TokenType::NEW_LINE,
                        "Expected a new line or a semicolon after statement.");
            }

            advanceLines();
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
        return this->enumDeclaration();
    case TokenType::STRUCT:
    case TokenType::UNION:
        return this->structDeclaration();
    case TokenType::EXTERN:
    case TokenType::VOLATILE:
    case TokenType::PRIVATE:
        return qualifiers();
    case TokenType::IDENTIFIER:
    case TokenType::SELF:
    case TokenType::ANY:
        return this->expression();
    case TokenType::FUNC:
        return this->functionDeclaration();
    case TokenType::VAR:
        return variableDeclaration();
    case TokenType::LET:
        return variableDeclaration({}, true);
    case TokenType::V_INT:
    case TokenType::V_CHAR:
    case TokenType::V_MULTILINE_STRING:
    case TokenType::V_FLOAT:
    case TokenType::V_STRING:
        return expression();
    case TokenType::FOR:
        return this->forLoop();
    case TokenType::WHILE:
        return this->whileLoop();
    case TokenType::RETURN:
        return this->returnStatement(false);
    case TokenType::THROW:
        return this->returnStatement(true);
    case TokenType::IF:
        return this->ifStatement();
    case TokenType::ASM:
        return this->inlineAssembly();
    case TokenType::GOTO:
        return this->gotoStatement();
    case TokenType::SWITCH:
        return this->switchStatement();
    case TokenType::DO:
        return this->doCatchStatement();
    case TokenType::BREAK:
    case TokenType::CONTINUE:
        return this->token();
    case TokenType::TYPEALIAS:
        return this->typealias();
    case TokenType::AT:
        return this->structInitializerDeclaration();
    case TokenType::BITWISE_NOT:
        return this->structDestructorDeclaration();
    case TokenType::PERIOD:
    case TokenType::OPERATOR_SUB:
        return this->expression();
    case TokenType::NEW_LINE:
        this->throwError(
            "Error with parsing '\\n', please create a Github issue.");
        return nullptr;
    case TokenType::T_EOF:
        return nullptr;
    default:
        std::cout << tokenTypeAsLiteral(this->current().type) << " "
                  << this->current().location.toString();
        this->throwError("Cannot parse token.");
    }
    exit(-1);
}

ImportStatement *Parser::import() {
    advance(TokenType::IMPORT);

    std::string_view import_path;
    bool is_library = false;
    switch (this->current().type) {
    case TokenType::V_STRING:
        import_path = getAndAdvance(TokenType::V_STRING);
        break;
    case TokenType::IDENTIFIER:
        import_path = getAndAdvance();
        is_library = true;
        break;
    default:
        this->throwError(
            "Expected a string literal or a library name after import.");
    }

    return this->makeDeclaration<ImportStatement>(import_path, is_library);
}

StructDeclaration *
Parser::structDeclaration(const std::vector<TokenType> &qualifiers) {
    bool is_union = false;
    if (this->current().type == TokenType::UNION) {
        advance(TokenType::UNION);
        is_union = true;
    } else {
        advance(TokenType::STRUCT);
    }

    auto struct_name =
        getAndAdvance(TokenType::IDENTIFIER,
                      "Expected a struct or a union name after declaration.");

    std::optional<TemplateDeclaration *> template_ = std::nullopt;

    if (!is_union) {
        if (advanceIf(TokenType::COLON)) {
            template_ = this->templateDeclaration();
        }
    }

    advance(TokenType::BRACE_OPEN, "The struct declaration must have a body.");
    is_parsing_struct = true;
    auto struct_body = this->compound();
    is_parsing_struct = false;
    advance(TokenType::BRACE_CLOSE,
            "The struct declaration must have a '}' in order to close it.");

    return this->makeDeclaration<StructDeclaration>(
        struct_name, qualifiers, struct_body, template_, is_union);
}

AST *Parser::structMemberAccess() {
    std::string_view variable_name;
    if (this->current().type == TokenType::IDENTIFIER) {
        variable_name = getAndAdvance(TokenType::IDENTIFIER);
    } else if (advanceIf(TokenType::SELF)) {
        variable_name = "self";
    } else {
        this->throwError(
            "Expected a identifier or a self after struct member access.");
    }

    advance(TokenType::PERIOD);

    if (this->current().type == TokenType::IDENTIFIER) {
        auto identifier = getAndAdvance(TokenType::IDENTIFIER);

        if (identifier == "init") {
            advance(TokenType::PARENS_OPEN,
                    "Expected the initializer to have arguments.");
            auto arguments = functionCallArguments();
            advance(TokenType::PARENS_CLOSE,
                    "Expected a close parenthesis after initializer.");

            return this->makeDeclaration<StructInitializerCall>(variable_name,
                                                                arguments);
        } else if (identifier == "deinit") {
            advance(TokenType::PARENS_OPEN,
                    "Expected the deinitializer to have a open parenthesis.");
            advance(TokenType::PARENS_CLOSE,
                    "Expected the deinitializer to have a close parenthesis.");

            return this->makeDeclaration<StructInitializerCall>(variable_name,
                                                                true);
        } else {
            reverse();
            goto expression;
        }
    }

expression:
    auto struct_member = expression();

    return this->makeDeclaration<StructMemberAccess>(variable_name,
                                                     struct_member);
}

AST *Parser::structMemberAccess(FunctionCall *function_call) {
    advance(TokenType::PERIOD);

    auto struct_member = expression();

    return this->makeDeclaration<StructMemberAccess>(function_call,
                                                     struct_member);
}

AST *Parser::initOrEnumCase() {
    advance(TokenType::PERIOD);

    auto identifier = getAndAdvance(
        TokenType::IDENTIFIER,
        "Expected a identifier after struct initializer or enum case.");

    if (identifier == "init") {
        advance(TokenType::PARENS_OPEN,
                "Expected the initializer to have arguments.");
        auto arguments = functionCallArguments();
        advance(TokenType::PARENS_CLOSE,
                "Expected a close parenthesis after the initializer.");

        return this->makeDeclaration<StructInitializerCall>(arguments);
    } else {
        return this->makeDeclaration<EnumCaseAccess>(identifier);
    }
}

StructInitializerDeclaration *Parser::structInitializerDeclaration() {
    advance(TokenType::AT);
    advance(TokenType::PARENS_OPEN,
            "Expected the initializer to have arguments.");
    auto arguments = functionArguments();
    advance(TokenType::PARENS_CLOSE, "Expected a ')' after the initializer.");

    advance(TokenType::BRACE_OPEN,
            "Expected a body after the struct initializer.");
    auto body = compound();
    advance(TokenType::BRACE_CLOSE,
            "The struct initializer body must be closed.");

    return this->makeDeclaration<StructInitializerDeclaration>(arguments, body);
}

AST *Parser::structDestructorDeclaration() {
    advance(TokenType::BITWISE_NOT);
    if (!advanceIf(TokenType::PARENS_OPEN)) {
        reverse();
        return this->expression();
    }
    advance(TokenType::PARENS_CLOSE,
            "Expected a close parenthesis after the destructor.");

    advance(TokenType::BRACE_OPEN, "The destructor must have body.");
    auto body = compound();
    advance(TokenType::BRACE_CLOSE, "The destructor body must be closed.");

    return this->makeDeclaration<StructInitializerDeclaration>(body);
}

EnumDeclaration *
Parser::enumDeclaration(const std::vector<TokenType> &qualifiers) {
    advance(TokenType::ENUM);

    auto enum_name =
        getAndAdvance(TokenType::IDENTIFIER, "Expected a enum name.");

    advance(TokenType::BRACE_OPEN, "The enum must have body.");
    advanceLines();
    auto cases = enumCases();
    advance(TokenType::BRACE_CLOSE, "The enum body must be closed.");

    return this->makeDeclaration<EnumDeclaration>(enum_name, cases, qualifiers);
}

std::vector<EnumCase *> Parser::enumCases() {
    std::vector<EnumCase *> enum_cases;

    for (int enum_case_value = 0;
         this->current().type != TokenType::BRACE_CLOSE; enum_case_value++) {
        auto case_name = getAndAdvance(TokenType::IDENTIFIER,
                                       "THe enum case must have a name.");

        AST *case_value;

        if (advanceIf(TokenType::EQUAL)) {
            case_value = this->expression();
        } else {
            auto case_value_string = std::to_string(enum_case_value);

            case_value = this->makeDeclaration<LiteralExpression>(
                case_value_string, TokenType::V_INT);
        }

        auto enum_case = this->makeDeclaration<EnumCase>(case_name, case_value);

        enum_cases.push_back(enum_case);
        advanceIf(TokenType::COMMA);
        advanceLines();
    }

    return enum_cases;
}

AST *Parser::functionDeclaration(const std::vector<TokenType> &qualifiers) {
    // func main(): i64 { ... }
    advance(TokenType::FUNC);

    if (this->current().type == TokenType::OPERATOR) {
        return this->operatorOverload();
    }

    auto function_name =
        getAndAdvance(TokenType::IDENTIFIER, "Functions must have a name.");

    advance(TokenType::PARENS_OPEN, "Functions must have arguments.");

    auto function_arguments = this->functionArguments();

    advance(TokenType::PARENS_CLOSE, "The functions arguments must be closed.");

    std::optional<AST *> return_type = std::nullopt;
    if (advanceIf(TokenType::COLON)) {
        return_type = type();
    }

    std::optional<TemplateDeclaration *> template_ = std::nullopt;
    if (advanceIf(TokenType::BITWISE_PIPE_PIPE)) {
        template_ = this->templateDeclaration();
    }

    if (advanceIf(TokenType::BRACE_OPEN)) {
        auto function_body = this->compound();
        advance(TokenType::BRACE_CLOSE, "Function's body must be closed.");

        return this->makeDeclaration<FunctionDeclaration>(
            qualifiers, return_type, function_name, function_arguments,
            function_body, template_);
    }

    if (template_) {
        this->throwError("Functions without a body can't have a template!");
    }

    return this->makeDeclaration<FunctionDeclaration>(
        qualifiers, return_type, function_name, function_arguments);
}

AST *Parser::variableDeclaration(const std::vector<TokenType> &qualifiers,
                                 bool is_let) {
    is_let ? advance(TokenType::LET) : advance(TokenType::VAR);

    auto variable_name = getAndAdvance(TokenType::IDENTIFIER,
                                       "Expected a variable or constant name.");

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
        this->throwError(
            "Expected ':' or '=' after variable or constant declaration.");
    }

    if (!is_parsing_struct && !variable_value) {
        this->throwError("Uninitialized variable or constant declaration.");
    }

    return this->makeDeclaration<VariableDeclaration>(
        variable_name, variable_type, variable_value, qualifiers, is_let);
}

RangeBasedForLoop *Parser::rangeBasedForLoop() {
    auto name = getAndAdvance(TokenType::IDENTIFIER,
                              "Expected a name for range-based for loops.");
    advance(TokenType::IN, "Range-based for loops must have `in` token.");

    auto name2 = expression();
    this->advance(
        TokenType::PARENS_CLOSE,
        "Expected a closing parenthesis after range-based for loops.");

    std::optional<AST *> for_index = std::nullopt;
    if (advanceIf(TokenType::BITWISE_PIPE)) {
        is_parsing_struct = true;
        for_index = statement();
        advance(
            TokenType::BITWISE_PIPE,
            "Expected a closing pipe operator after range-based for loops.");
        is_parsing_struct = false;
    }

    this->advance(TokenType::BRACE_OPEN,
                  "Range-based for loops must have a body.");
    auto for_body = this->compound();
    this->advance(TokenType::BRACE_CLOSE,
                  "Range-based for loops must have a body that closes.");

    return this->makeDeclaration<RangeBasedForLoop>(name, name2, for_index,
                                                    for_body);
}

AST *Parser::forLoop() {
    this->advance(TokenType::FOR);

    this->advance(TokenType::PARENS_OPEN,
                  "Expected a opening parenthesis after a for loop.");

    if (peek().type == TokenType::IN) {
        return this->rangeBasedForLoop();
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

    return this->makeDeclaration<ForLoop>(for_initialization, for_condition,
                                          for_increment, for_body);
}

WhileLoop *Parser::whileLoop() {
    advance(TokenType::WHILE);

    advance(TokenType::PARENS_OPEN, "The while loop must have a parenthesis.");
    auto expression = this->expression();
    advance(TokenType::PARENS_CLOSE,
            "While loop's parenthesis must be closed.");

    this->advance(TokenType::BRACE_OPEN, "The while loop must have a body.");
    auto body = this->compound();
    this->advance(TokenType::BRACE_CLOSE,
                  "The while loop's body must be closed.");

    return this->makeDeclaration<WhileLoop>(expression, body);
}

Return *Parser::returnStatement(bool is_throw_statement) {
    if (is_throw_statement) {
        advance(TokenType::THROW);
    } else {
        advance(TokenType::RETURN);
    }

    std::optional<AST *> return_value = std::nullopt;

    if (this->current().type != TokenType::NEW_LINE) {
        return_value = this->expression();
    }

    return this->makeDeclaration<Return>(return_value, is_throw_statement);
}

If *Parser::ifStatement() {
    advance(TokenType::IF);

    auto if_body_and_statement = this->ifElseStatements();

    std::vector<AST *> elseif_conditions = {};
    std::vector<CompoundStatement *> elseif_bodies = {};
    std::optional<CompoundStatement *> else_body = std::nullopt;

    while (advanceIf(TokenType::ELSE)) {
        if (advanceIf(TokenType::IF)) {
            auto else_body_and_statement = this->ifElseStatements();

            elseif_conditions.push_back(else_body_and_statement.first);
            elseif_bodies.push_back(else_body_and_statement.second);
        } else {
            advance(TokenType::BRACE_OPEN,
                    "The else statement must have a body.");
            else_body = this->compound();
            advance(TokenType::BRACE_CLOSE,
                    "The else statement's body must be closed.");
            break;
        }
    }

    return this->makeDeclaration<If>(
        if_body_and_statement.first, if_body_and_statement.second,
        elseif_conditions, elseif_bodies, else_body);
}

std::pair<AST *, CompoundStatement *> Parser::ifElseStatements() {
    advance(TokenType::PARENS_OPEN,
            "Expected a expression after a if or else statement.");
    auto condition = this->expression();
    advance(TokenType::PARENS_CLOSE,
            "Parenthesis must be closed inside an if statement.");

    advance(TokenType::BRACE_OPEN,
            "The if or else statement must have a body.");
    auto body = this->compound();
    advance(TokenType::BRACE_CLOSE,
            "The if or else statement's body must be closed.");

    return std::make_pair(condition, body);
}

ASM *Parser::inlineAssembly() {
    advance(TokenType::ASM);
    advance(TokenType::PARENS_OPEN, "Expected a parenthesis after assembly.");
    advanceLines();

    std::vector<std::string_view> instructions;

    while (this->current().type != TokenType::PARENS_CLOSE) {
        if (this->current().type == TokenType::V_STRING) {
            instructions.push_back(getAndAdvance(
                TokenType::V_STRING,
                "The assembly instruction must be inside a string literal."));
        } else {
            instructions.push_back(
                getAndAdvance(TokenType::V_MULTILINE_STRING,
                              "The assembly instruction must be inside a "
                              "string or multiline."));
        }

        advanceLines();
    }

    advance(TokenType::PARENS_CLOSE, "Assembly must be closed.");

    return this->makeDeclaration<ASM>(instructions);
}

GOTO *Parser::gotoStatement() {
    advance(TokenType::GOTO);

    auto label = getAndAdvance(TokenType::IDENTIFIER,
                               "Expected a label after the goto statement.");

    return this->makeDeclaration<GOTO>(label, true);
}

SwitchStatement *Parser::switchStatement() {
    advance(TokenType::SWITCH);

    advance(TokenType::PARENS_OPEN,
            "The switch statement must have an opening parenthesis.");
    auto switch_expression = expression();
    advance(TokenType::PARENS_CLOSE,
            "The switch statement must have a closing parenthesis.");

    advance(TokenType::BRACE_OPEN, "The switch statement must have a body.");
    advanceLines();
    auto cases = switchCases();
    advance(TokenType::BRACE_CLOSE,
            "The body of the switch statement must end with a '}'.");

    return this->makeDeclaration<SwitchStatement>(switch_expression, cases);
}

std::vector<SwitchCase *> Parser::switchCases() {
    std::vector<SwitchCase *> cases;

    while (this->current().type != TokenType::BRACE_CLOSE) {
        cases.push_back(switchCase());
        advanceLines();
    }

    return cases;
}

SwitchCase *Parser::switchCase() {
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

    auto *case_body = this->makeDeclaration<CompoundStatement>();

    advanceLines();
    while (this->current().type != TokenType::CASE &&
           this->current().type != TokenType::DEFAULT &&
           this->current().type != TokenType::BRACE_CLOSE) {
        case_body->statements.push_back(statement());
        advanceLines();
    }

    if (is_case) {
        return this->makeDeclaration<SwitchCase>(case_expression, case_body,
                                                 is_case);
    } else {
        return this->makeDeclaration<SwitchCase>(case_body, is_case);
    }
}

DoCatchStatement *Parser::doCatchStatement() {
    advance(TokenType::DO);

    advance(TokenType::BRACE_OPEN,
            "Expected a '(' after the do statement case.");
    auto do_body = this->compound();
    advance(TokenType::BRACE_CLOSE,
            "Expected a ')' after the do statement case.");

    advance(TokenType::CATCH, "The do statement must have a catch clause.");
    std::optional<AST *> catch_expression = std::nullopt;
    if (advanceIf(TokenType::PARENS_OPEN)) {
        is_parsing_struct = true;
        catch_expression = this->statement();
        advance(TokenType::PARENS_CLOSE,
                "Expected a ')' after the catch statement expression.");
        is_parsing_struct = false;
    }

    advance(TokenType::BRACE_OPEN, "The catch statement must have a body.");
    auto catch_body = this->compound();
    advance(TokenType::BRACE_CLOSE, "The catch statement's body must end.");

    return this->makeDeclaration<DoCatchStatement>(do_body, catch_body,
                                                   catch_expression);
}

OperatorOverload *Parser::operatorOverload() {
    /*
     func operator[](offset: f32): i32 {
        return self.items[offset]
     }
     */
    advance(TokenType::OPERATOR);
    std::vector<TokenType> operators;

    while (this->current().type != TokenType::PARENS_OPEN) {
        if (!isOperatorOverloadType(this->current().type)) {
            this->throwError("Invalid Operator");
        }

        operators.push_back(this->current().type);
        advance();
    }

    advance(TokenType::PARENS_OPEN,
            "Expected a '(' after the operator overloading.");
    auto arguments = this->functionArguments();
    advance(TokenType::PARENS_CLOSE,
            "Expected a ')' after operator overloading.");

    advance(TokenType::COLON, "Expected a ':' after the operator overloading.");
    auto return_type = this->type();

    advance(TokenType::BRACE_OPEN,
            "Expected a body after the operator overloading.");
    auto body = this->compound();
    advance(TokenType::BRACE_CLOSE,
            "Body after operator overloading must end.");

    return this->makeDeclaration<OperatorOverload>(return_type, operators,
                                                   arguments, body);
}

AST *Parser::expression() { return this->equality(); }

AST *Parser::tryExpression() {
    advance(TokenType::TRY);
    bool is_force_cast = false;    // try!
    bool is_optional_cast = false; // try?

    if (advanceIf(TokenType::QUESTION)) {
        is_optional_cast = true;
    } else if (advanceIf(TokenType::NOT)) {
        is_force_cast = true;
    }

    auto expression = this->expression();

    return this->makeDeclaration<TryExpression>(expression, is_force_cast,
                                                is_optional_cast);
}

AST *Parser::castExpression() {
    // cast?(50.3, int) || cast!(50.3, int)
    advance(TokenType::CAST);

    bool is_force_cast = false;
    bool is_optional_cast = false;

    if (advanceIf(TokenType::QUESTION)) {
        is_optional_cast = true;
    } else if (advanceIf(TokenType::NOT)) {
        is_force_cast = true;
    } else {
        this->throwError("The cast expression must be forced or optional.");
    }

    advance(TokenType::PARENS_OPEN, "THe cast expression must have '('.");
    auto cast_value = expression();
    advance(TokenType::COMMA,
            "The cast expression must have a comma after expression.");
    auto type = this->type();
    advance(TokenType::PARENS_CLOSE,
            "Expected a ')' at the end of the cast expression.");

    return this->makeDeclaration<CastExpression>(
        cast_value, type, is_force_cast, is_optional_cast);
}

TernaryExpression *Parser::ternaryExpression(AST *bool_expression) {
    // (expression) ? 50 : 20
    // 50 == 30 ? return true : return false
    advance(TokenType::QUESTION);

    parses_goto_labels = false;
    auto first_expression = statement();
    advance(TokenType::COLON, "Expected a ':' after the ternary expression.");
    auto second_expression = statement();
    parses_goto_labels = true;

    return this->makeDeclaration<TernaryExpression>(
        bool_expression, first_expression, second_expression);
}

OptionalUnwrapExpression *Parser::optionalUnwrap(AST *expression) {
    // myOptionalVariable ?? 40
    advance(TokenType::QUESTION);
    advance(TokenType::QUESTION);

    auto if_nilled_value = this->expression();

    return this->makeDeclaration<OptionalUnwrapExpression>(expression,
                                                           if_nilled_value);
}

ForceUnwrapExpression *Parser::forceUnwrap(AST *expression) {
    advance(TokenType::NOT);
    return this->makeDeclaration<ForceUnwrapExpression>(expression);
}

AST *Parser::equality() {
    auto expr = this->comparison();

    while (isEqualityOperator(this->current().type)) {
        TokenType operator_type = getAndAdvanceType();

        auto right_ast = this->comparison();

        expr = this->makeDeclaration<BinaryExpression>(expr, right_ast,
                                                       operator_type);
    }

    return expr;
}

AST *Parser::comparison() {
    auto expr = this->term();

    while (isComparativeOperator(this->current().type)) {
        TokenType operator_type = getAndAdvanceType();

        auto right_ast = this->term();

        expr = this->makeDeclaration<BinaryExpression>(expr, right_ast,
                                                       operator_type);
    }

    return expr;
}

AST *Parser::term() {
    auto expr = this->factor();

    while (isAdditiveOperator(this->current().type)) {
        TokenType operator_type = getAndAdvanceType();

        auto right_ast = this->factor();

        expr = this->makeDeclaration<BinaryExpression>(expr, right_ast,
                                                       operator_type);
    }

    return expr;
}

AST *Parser::factor() {
    auto expr = this->unary();

    while (isMultiplicativeOperator(this->current().type)) {
        TokenType operator_type = getAndAdvanceType();

        auto right_ast = this->unary();

        expr = this->makeDeclaration<BinaryExpression>(expr, right_ast,
                                                       operator_type);
    }

    return expr;
}

AST *Parser::unary() {
    if (isUnaryOperator(this->current().type)) {
        TokenType operator_type = getAndAdvanceType();

        auto expr = this->unary();

        return this->makeDeclaration<UnaryExpression>(expr, operator_type);
    }

    return this->primary();
}

AST *Parser::primary() {
    if (this->current().type == TokenType::TRY) {
        return tryExpression();
    }

    if (peek().type == TokenType::PERIOD) {
        return structMemberAccess();
    } else if (peek().type == TokenType::SQUARE_OPEN) {
        return arrayAccess();
    }

    if (this->current().type == TokenType::SQUARE_OPEN) {
        return arrayCreation();
    }

    if (this->current().type == TokenType::PERIOD) {
        return initOrEnumCase();
    }
    if (isRegularValue(this->current().type)) {
        auto literal = this->makeDeclaration<LiteralExpression>(
            this->current().value(this->error.buffer), this->current().type);
        advance();

        if (advanceIf(TokenType::PARENS_OPEN)) {
            return this->functionCall(literal->value);
        } else if (parses_goto_labels) {
            if (advanceIf(TokenType::COLON)) {
                return this->makeDeclaration<GOTO>(literal->value, false);
            }
        }

        if (this->current().type == TokenType::AT) {
            auto template_arguments = this->templateCallArguments();

            advance(
                TokenType::PARENS_CLOSE,
                "Expected a ')' after the function call template arguments.");
            // TODO: Optional Templates
            advance(
                TokenType::PARENS_OPEN,
                "Expected a '(' after the function call template arguments.");

            // TODO: Just to function call for now, in the future, when there
            // will be nested structs, we might have to make this more complex
            return this->functionCall(literal->value, template_arguments);
        }

        if (this->current().type == TokenType::QUESTION) {
            if (peek().type == TokenType::QUESTION) {
                return optionalUnwrap(literal);
            }
            return ternaryExpression(literal);
        }

        if (this->current().type == TokenType::NOT) {
            return forceUnwrap(literal);
        }

        return literal;
    } else if (advanceIf(TokenType::PARENS_OPEN)) {
        auto expr = this->expression();
        advance(TokenType::PARENS_CLOSE,
                "Expected a closing parenthesis after group expression.");

        return this->makeDeclaration<GroupingExpression>(expr);
    } else if (this->current().type == TokenType::SELF) {
        return structMemberAccess();
    } else if (this->current().type == TokenType::CAST) {
        return castExpression();
    } else {
        std::cout << tokenTypeAsLiteral(this->current().type);
        this->throwError("Invalid expression.");
    }
    exit(-1);
}

AST *Parser::functionCall(
    std::string_view function_name,
    const std::optional<std::vector<AST *>> &template_arguments) {

    auto arguments = functionCallArguments();
    advance(TokenType::PARENS_CLOSE,
            "Expected a closing parenthesis after function call.");

    auto decl = this->makeDeclaration<FunctionCall>(function_name, arguments,
                                                    template_arguments);

    if (this->current().type == TokenType::PERIOD) {
        return this->structMemberAccess(decl);
    } else if (this->current().type == TokenType::SQUARE_OPEN) {
        return arrayAccess(decl);
    }

    return decl;
}

AST *Parser::arrayAccess() {
    // myVariable[40]
    auto variable_name = getAndAdvance(
        TokenType::IDENTIFIER, "Expecteda  identifier after array access.");
    advance(TokenType::SQUARE_OPEN);
    auto inside = this->expression();
    advance(TokenType::SQUARE_CLOSE,
            "Expected a closing square after array access.");

    return this->makeDeclaration<ArrayAccess>(variable_name, inside);
}

AST *Parser::arrayAccess(FunctionCall *function_call) {
    // myFunction()[30]
    advance(TokenType::SQUARE_OPEN);
    auto inside = this->expression();
    advance(TokenType::SQUARE_CLOSE,
            "Expected a closing square after array access.");

    return this->makeDeclaration<ArrayAccess>(function_call, inside);
}

AST *Parser::arrayCreation() {
    // [50, 30, 20]
    advance(TokenType::SQUARE_OPEN);

    std::vector<AST *> items;
    while (this->current().type != TokenType::SQUARE_CLOSE) {
        items.push_back(this->expression());

        advanceIf(TokenType::COMMA);
    }

    advance(TokenType::SQUARE_CLOSE,
            "Expected a closing square after array creation.");

    return this->makeDeclaration<ArrayCreation>(items);
}

std::vector<AST *> Parser::functionCallArguments() {
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
                              "argument style");
            advance(TokenType::COLON);
            auto argument_value = this->expression();

            auto argument =
                this->makeDeclaration<FunctionCallNameBasedArgument>(
                    argument_name, argument_value);

            argument_can_have_unnamed_arguments = false;
            arguments.push_back(argument);
        } else {
        argument_expression:
            if (!argument_can_have_unnamed_arguments) {
                throwError("Cannot use unnamed arguments after "
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
    auto type_name = getAndAdvance(
        TokenType::IDENTIFIER, "Expected a type name after the type alias.");
    advance(TokenType::EQUAL, "Expected a type after the type alias.");
    auto value = expression();

    return this->makeDeclaration<Typealias>(type_name, value);
}

AST *Parser::token() {
    auto type = this->current().type;
    advance(type);
    return this->makeDeclaration<ASTToken>(type);
}

TemplateDeclaration *Parser::templateDeclaration() {
    advance(TokenType::AT);
    advance(TokenType::PARENS_OPEN,
            "Expected a '(' when using template declarations.");

    auto arguments = templateArguments();

    advance(TokenType::PARENS_CLOSE,
            "Expected a ')' when using template declarations.");

    return this->makeDeclaration<TemplateDeclaration>(arguments);
}

std::vector<TemplateDeclarationArgument *> Parser::templateArguments() {
    std::vector<TemplateDeclarationArgument *> arguments;

    while (this->current().type != TokenType::PARENS_CLOSE) {
        if (!isTemplateKeyword(this->current().type)) {
            this->throwError("Invalid template type.");
        }

        auto argument_type = getAndAdvanceType();
        auto argument_name = getAndAdvance(
            TokenType::IDENTIFIER, "Expected a template argument name.");

        auto argument = this->makeDeclaration<TemplateDeclarationArgument>(
            argument_name, argument_type);

        arguments.push_back(argument);

        if (!advanceIf(TokenType::COMMA)) {
            break;
        }
    }

    return arguments;
}

std::vector<AST *> Parser::templateCallArguments() {
    this->advance(TokenType::AT);
    advance(TokenType::PARENS_OPEN,
            "Expected a '(' when using template calls.");

    std::vector<AST *> template_values;
    while (this->current().type != TokenType::PARENS_CLOSE) {
        template_values.push_back(this->type());

        if (!advanceIf(TokenType::COMMA)) {
            break;
        }
    }

    return template_values;
}

// func main(argc: usize, let argv: string[])
std::vector<FunctionArgument *> Parser::functionArguments() {
    std::vector<FunctionArgument *> arguments;

    bool can_use_arguments_without_default_value = true;

    while (this->current().type != TokenType::PARENS_CLOSE) {
        bool is_constant = false;
        if (advanceIf(TokenType::LET)) {
            is_constant = true;
        }
        if (advanceIf(TokenType::PERIOD)) {
            advance(TokenType::PERIOD, "Expected a '..' when using vaargs.");
            advance(TokenType::PERIOD, "Expected a '.' when using vaargs.");

            auto argument = this->makeDeclaration<FunctionArgument>();
            argument->is_vaarg = true;
            argument->is_constant = is_constant;
            argument->name =
                getAndAdvance(TokenType::IDENTIFIER, "Expected a vaarg name.");
            arguments.push_back(argument);

            break;
        }

        std::optional<std::string_view> argument_name;
        std::optional<AST *> argument_default_value = std::nullopt;
        std::optional<AST *> argument_type = std::nullopt;
        if (this->current().type == TokenType::IDENTIFIER) {
            argument_name = getAndAdvance(TokenType::IDENTIFIER);

            if (advanceIf(TokenType::COLON)) {
                argument_type = this->type();
            }

            if (advanceIf(TokenType::EQUAL)) {
                can_use_arguments_without_default_value = false;
                argument_default_value = this->expression();
            }
        } else {
            argument_type = this->type();
        }

        if (!can_use_arguments_without_default_value &&
            !argument_default_value) {
            throwError("Move the argument with a default value to the "
                       "end of the argument list, since you have "
                       "already specified a default value.");
        }

        auto argument = this->makeDeclaration<FunctionArgument>(
            argument_name, argument_type, argument_default_value, is_constant);

        arguments.push_back(argument);

        if (!advanceIf(TokenType::COMMA)) {
            break;
        }
    }

    return arguments;
}

FirstClassFunction *Parser::firstClassFunction() {
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
    auto arguments = this->functionArguments();
    advance(TokenType::PARENS_CLOSE,
            "Expected a ')' at the end of a first class function.");

    return this->makeDeclaration<FirstClassFunction>(return_type, arguments,
                                                     is_type);
}

AST *Parser::type() {
    auto type = this->makeDeclaration<Type>(current().type,
                                            current().value(this->error.buffer),
                                            false, false, false, false);

    if (!isRegularType(this->current().type)) {
        std::cout << tokenTypeAsLiteral(this->current().type) << std::endl;
        this->throwError("Parser: Expected type\n");
    }

    if (this->current().type == TokenType::DOLLAR) {
        return this->firstClassFunction();
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
            type->template_values = templateCallArguments();
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
        return this->enumDeclaration(qualifiers);
    } else if (this->current().type == TokenType::STRUCT ||
               this->current().type == TokenType::UNION) {
        return this->structDeclaration(qualifiers);
    } else if (this->current().type == TokenType::VAR) {
        return this->variableDeclaration(qualifiers);
    } else if (this->current().type == TokenType::LET) {
        return variableDeclaration(qualifiers, true);
    } else if (this->current().type == TokenType::FUNC) {
        return this->functionDeclaration(qualifiers);
    }
    this->throwError("Expected enum, struct, function or variable.");
    exit(-1);
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

void Parser::advance() { this->index += 1; }

void Parser::reverse() { this->index -= 1; }

void Parser::advance(TokenType type, const char *error_message) {
    if (this->current().type == type) {
        this->advance();
    } else {
        //        std::cout << tokenTypeAsLiteral(this->current().type)
        //                  << " :: " << tokenTypeAsLiteral(type)
        //                  << " :: " << this->current().location.toString() <<
        //                  std::endl;
        this->throwError(error_message);
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

TokenType Parser::getAndAdvanceType() {
    auto value = this->current().type;
    advance();
    return value;
}

std::string_view Parser::getAndAdvance() {
    auto value = this->current().value(this->error.buffer);
    advance();
    return value;
}

std::string_view Parser::getAndAdvance(TokenType type, const char *message) {
    auto value = this->current().value(this->error.buffer);
    advance(type, message);
    return value;
}

template <class ast_type, class... Args>
ast_type *Parser::makeDeclaration(Args &&...args) {
    return new ast_type(std::forward<Args>(args)..., this->current().location);
}

int Parser::throwError(const char *message) {
    error.addError(message, this->current().location);
    error.displayMessages();
    exit(-1);
    return -1;
}

int Parser::throwError(const char *message, Location location) {
    error.addError(message, location);
    error.displayMessages();
    exit(-1);
    return -1;
}
