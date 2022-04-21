//
// Created by Ashwin Paudel on 2022-03-26.
//

#include "Parser.h"

namespace drast::parser {

AST::Compound *Parser::parse() {
    auto compound = this->compound();

    std::cout << compound->toString() << '\n';

    return compound;
}

AST::Compound *Parser::compound() {
    auto compoundStatement = this->makeDeclaration<AST::Compound>();
    auto old_compound = current_compound;
    current_compound = compoundStatement;

    // $(int a, bool b)
    if (this->current().type == lexer::TokenType::DOLLAR) {
        if (!inside_function_body) {
            throwError("The first class function parameter must be "
                       "inside a valid body.");
        }
        compoundStatement->first_class_function = this->firstClassFunction();
    }

    advanceLines();

    while (this->current().type != lexer::TokenType::BRACE_CLOSE &&
           this->current().type != lexer::TokenType::T_EOF) {
        auto statement = this->statement();
        compoundStatement->statements.push_back(statement);

        if (this->current().type != lexer::TokenType::T_EOF) {
            if (this->current().type != lexer::TokenType::NEW_LINE) {
                if (this->current().type != lexer::TokenType::SEMICOLON) {
                    std::cout << "HEllo1: " << statement->toString() << '\n';
                }
                advance(lexer::TokenType::SEMICOLON,
                        "Expected a new line or a semicolon after statement.");
            } else {
                if (this->current().type != lexer::TokenType::NEW_LINE) {
                    std::cout << "HEllo1: " << statement->toString() << '\n';
                }
                advance(lexer::TokenType::NEW_LINE,
                        "Expected a new line or a semicolon after statement.");
            }

            advanceLines();
        }
    }

    current_compound = old_compound;

    return compoundStatement;
}

AST::Node *Parser::statement() {
    switch (this->current().type) {
    case lexer::TokenType::IMPORT:
        return import();
    case lexer::TokenType::ENUM:
        return this->enumDeclaration();
    case lexer::TokenType::STRUCT:
    case lexer::TokenType::UNION:
        return this->structDeclaration();
    case lexer::TokenType::EXTERN:
    case lexer::TokenType::VOLATILE:
    case lexer::TokenType::PRIVATE:
        return qualifiers();
    case lexer::TokenType::IDENTIFIER:
    case lexer::TokenType::SELF:
    case lexer::TokenType::ANY:
        return this->expression();
    case lexer::TokenType::FUNC:
        return this->functionDeclaration();
    case lexer::TokenType::VAR:
        return variableDeclaration();
    case lexer::TokenType::LET:
        return variableDeclaration({}, true);
    case lexer::TokenType::V_INT:
    case lexer::TokenType::V_CHAR:
    case lexer::TokenType::V_MULTILINE_STRING:
    case lexer::TokenType::V_FLOAT:
    case lexer::TokenType::V_STRING:
        return expression();
    case lexer::TokenType::FOR:
        return this->forLoop();
    case lexer::TokenType::WHILE:
        return this->whileLoop();
    case lexer::TokenType::RETURN:
        return this->returnStatement(false);
    case lexer::TokenType::THROW:
        return this->returnStatement(true);
    case lexer::TokenType::IF:
        return this->ifStatement();
    case lexer::TokenType::ASM:
        return this->inlineAssembly();
    case lexer::TokenType::GOTO:
        return this->gotoStatement();
    case lexer::TokenType::SWITCH:
        return this->switchStatement();
    case lexer::TokenType::DO:
        return this->doCatchStatement();
    case lexer::TokenType::BREAK:
    case lexer::TokenType::CONTINUE:
        return this->token();
    case lexer::TokenType::TYPEALIAS:
        return this->typealias();
    case lexer::TokenType::AT:
        return this->structInitializerDeclaration();
    case lexer::TokenType::BITWISE_NOT:
        return this->structDestructorDeclaration();
    case lexer::TokenType::PERIOD:
    case lexer::TokenType::OPERATOR_SUB:
        return this->expression();
    case lexer::TokenType::NEW_LINE:
        this->throwError(
            "Error with parsing '\\n', please create a Github issue.");
        return nullptr;
    case lexer::TokenType::T_EOF:
        return nullptr;
    default:
        std::cout << lexer::tokenTypeAsLiteral(this->current().type) << " "
                  << this->current().location.toString();
        this->throwError("Cannot parse token.");
    }
    exit(-1);
}

AST::Import *Parser::import() {
    advance(lexer::TokenType::IMPORT);

    std::string_view import_path;
    bool is_library = false;
    switch (this->current().type) {
    case lexer::TokenType::V_STRING:
        import_path = getAndAdvance(lexer::TokenType::V_STRING);
        break;
    case lexer::TokenType::IDENTIFIER:
        import_path = getAndAdvance();
        is_library = true;
        break;
    default:
        this->throwError(
            "Expected a string literal or a library name after import.");
    }

    return this->makeDeclaration<AST::Import>(import_path, is_library);
}

AST::StructDeclaration *
Parser::structDeclaration(const std::vector<lexer::TokenType> &qualifiers) {
    bool is_union = false;
    if (this->current().type == lexer::TokenType::UNION) {
        advance(lexer::TokenType::UNION);
        is_union = true;
    } else {
        advance(lexer::TokenType::STRUCT);
    }

    auto struct_name =
        getAndAdvance(lexer::TokenType::IDENTIFIER,
                      "Expected a struct or a union name after declaration.");

    std::optional<AST::TemplateDeclaration *> template_ = std::nullopt;

    if (!is_union) {
        if (advanceIf(lexer::TokenType::COLON)) {
            template_ = this->templateDeclaration();
        }
    }

    advance(lexer::TokenType::BRACE_OPEN,
            "The struct declaration must have a body.");
    is_parsing_struct = true;
    auto struct_body = this->compound();
    is_parsing_struct = false;
    advance(lexer::TokenType::BRACE_CLOSE,
            "The struct declaration must have a '}' in order to close it.");

    return this->makeDeclaration<AST::StructDeclaration>(
        struct_name, qualifiers, struct_body, template_, is_union);
}

AST::StructInitializerDeclaration *Parser::structInitializerDeclaration() {
    advance(lexer::TokenType::AT);
    advance(lexer::TokenType::PARENS_OPEN,
            "Expected the initializer to have arguments.");
    auto arguments = functionArguments();
    advance(lexer::TokenType::PARENS_CLOSE,
            "Expected a ')' after the initializer.");

    advance(lexer::TokenType::BRACE_OPEN,
            "Expected a body after the struct initializer.");
    auto body = compound();
    advance(lexer::TokenType::BRACE_CLOSE,
            "The struct initializer body must be closed.");

    return this->makeDeclaration<AST::StructInitializerDeclaration>(arguments,
                                                                    body);
}

AST::Node *Parser::structDestructorDeclaration() {
    advance(lexer::TokenType::BITWISE_NOT);
    if (!advanceIf(lexer::TokenType::PARENS_OPEN)) {
        reverse();
        return this->expression();
    }
    advance(lexer::TokenType::PARENS_CLOSE,
            "Expected a close parenthesis after the destructor.");

    advance(lexer::TokenType::BRACE_OPEN, "The destructor must have body.");
    auto body = compound();
    advance(lexer::TokenType::BRACE_CLOSE,
            "The destructor body must be closed.");

    return this->makeDeclaration<AST::StructInitializerDeclaration>(body);
}

AST::EnumDeclaration *
Parser::enumDeclaration(const std::vector<lexer::TokenType> &qualifiers) {
    advance(lexer::TokenType::ENUM);

    auto enum_name =
        getAndAdvance(lexer::TokenType::IDENTIFIER, "Expected a enum name.");

    advance(lexer::TokenType::BRACE_OPEN, "The enum must have body.");
    advanceLines();
    auto cases = enumCases();
    advance(lexer::TokenType::BRACE_CLOSE, "The enum body must be closed.");

    return this->makeDeclaration<AST::EnumDeclaration>(enum_name, cases,
                                                       qualifiers);
}

AST::Node *
Parser::functionDeclaration(const std::vector<lexer::TokenType> &qualifiers) {
    // func main(): i64 { ... }
    advance(lexer::TokenType::FUNC);

    if (this->current().type == lexer::TokenType::OPERATOR) {
        return this->operatorOverload();
    }

    auto function_name = getAndAdvance(lexer::TokenType::IDENTIFIER,
                                       "Functions must have a name.");

    advance(lexer::TokenType::PARENS_OPEN, "Functions must have arguments.");

    auto function_arguments = this->functionArguments();

    advance(lexer::TokenType::PARENS_CLOSE,
            "The functions arguments must be closed.");

    std::optional<AST::Node *> return_type = std::nullopt;
    if (advanceIf(lexer::TokenType::COLON)) {
        return_type = type();
    }

    std::optional<AST::TemplateDeclaration *> template_ = std::nullopt;
    if (advanceIf(lexer::TokenType::BITWISE_PIPE_PIPE)) {
        template_ = this->templateDeclaration();
    }

    if (advanceIf(lexer::TokenType::BRACE_OPEN)) {
        auto function_body = this->compound();
        advance(lexer::TokenType::BRACE_CLOSE,
                "Function's body must be closed.");

        return this->makeDeclaration<AST::FunctionDeclaration>(
            qualifiers, return_type, function_name, function_arguments,
            function_body, template_);
    }

    if (template_) {
        this->throwError("Functions without a body can't have a template!");
    }

    return this->makeDeclaration<AST::FunctionDeclaration>(
        qualifiers, return_type, function_name, function_arguments);
}

AST::Node *
Parser::variableDeclaration(const std::vector<lexer::TokenType> &qualifiers,
                            bool is_let) {
    is_let ? advance(lexer::TokenType::LET) : advance(lexer::TokenType::VAR);

    auto variable_name = getAndAdvance(lexer::TokenType::IDENTIFIER,
                                       "Expected a variable or constant name.");

    std::optional<AST::Node *> variable_value = std::nullopt;
    std::optional<AST::Node *> variable_type = std::nullopt;

    if (advanceIf(lexer::TokenType::EQUAL)) {
        variable_value = this->expression();
    } else if (advanceIf(lexer::TokenType::COLON)) {
        variable_type = this->type();

        if (advanceIf(lexer::TokenType::EQUAL)) {
            variable_value = this->expression();
        }
    } else {
        this->throwError(
            "Expected ':' or '=' after variable or constant declaration.");
    }

    if (!is_parsing_struct && !variable_value) {
        this->throwError("Uninitialized variable or constant declaration.");
    }

    return this->makeDeclaration<AST::VariableDeclaration>(
        variable_name, variable_type, variable_value, qualifiers, is_let);
}

AST::OperatorOverload *Parser::operatorOverload() {
    /*
     func operator[](offset: f32): i32 {
        return self.items[offset]
     }
     */
    advance(lexer::TokenType::OPERATOR);
    std::vector<lexer::TokenType> operators;

    while (this->current().type != lexer::TokenType::PARENS_OPEN) {
        if (!isOperatorOverloadType(this->current().type)) {
            this->throwError("Invalid Operator");
        }

        operators.push_back(this->current().type);
        advance();
    }

    advance(lexer::TokenType::PARENS_OPEN,
            "Expected a '(' after the operator overloading.");
    auto arguments = this->functionArguments();
    advance(lexer::TokenType::PARENS_CLOSE,
            "Expected a ')' after operator overloading.");

    advance(lexer::TokenType::COLON,
            "Expected a ':' after the operator overloading.");
    auto return_type = this->type();

    advance(lexer::TokenType::BRACE_OPEN,
            "Expected a body after the operator overloading.");
    auto body = this->compound();
    advance(lexer::TokenType::BRACE_CLOSE,
            "Body after operator overloading must end.");

    return this->makeDeclaration<AST::OperatorOverload>(return_type, operators,
                                                        arguments, body);
}

AST::Node *Parser::structMemberAccess() {
    std::string_view variable_name;
    if (this->current().type == lexer::TokenType::IDENTIFIER) {
        variable_name = getAndAdvance(lexer::TokenType::IDENTIFIER);
    } else if (advanceIf(lexer::TokenType::SELF)) {
        variable_name = "self";
    } else {
        this->throwError(
            "Expected a identifier or a self after struct member access.");
    }

    advance(lexer::TokenType::PERIOD);

    if (this->current().type == lexer::TokenType::IDENTIFIER) {
        auto identifier = getAndAdvance(lexer::TokenType::IDENTIFIER);

        if (identifier == "init") {
            advance(lexer::TokenType::PARENS_OPEN,
                    "Expected the initializer to have arguments.");
            auto arguments = functionCallArguments();
            advance(lexer::TokenType::PARENS_CLOSE,
                    "Expected a close parenthesis after initializer.");

            return this->makeDeclaration<AST::StructInitializerCall>(
                variable_name, arguments);
        } else if (identifier == "deinit") {
            advance(lexer::TokenType::PARENS_OPEN,
                    "Expected the deinitializer to have a open parenthesis.");
            advance(lexer::TokenType::PARENS_CLOSE,
                    "Expected the deinitializer to have a close parenthesis.");

            return this->makeDeclaration<AST::StructInitializerCall>(
                variable_name, true);
        } else {
            reverse();
            goto expression;
        }
    }

expression:
    auto struct_member = expression();

    return this->makeDeclaration<AST::StructMemberAccess>(variable_name,
                                                          struct_member);
}

AST::Node *Parser::structMemberAccess(AST::FunctionCall *function_call) {
    advance(lexer::TokenType::PERIOD);

    auto struct_member = expression();

    return this->makeDeclaration<AST::StructMemberAccess>(function_call,
                                                          struct_member);
}

AST::Node *Parser::initOrEnumCase() {
    advance(lexer::TokenType::PERIOD);

    auto identifier = getAndAdvance(
        lexer::TokenType::IDENTIFIER,
        "Expected a identifier after struct initializer or enum case.");

    if (identifier == "init") {
        advance(lexer::TokenType::PARENS_OPEN,
                "Expected the initializer to have arguments.");
        auto arguments = functionCallArguments();
        advance(lexer::TokenType::PARENS_CLOSE,
                "Expected a close parenthesis after the initializer.");

        return this->makeDeclaration<AST::StructInitializerCall>(arguments);
    } else {
        return this->makeDeclaration<AST::EnumCaseAccess>(identifier);
    }
}

std::vector<AST::EnumCase *> Parser::enumCases() {
    std::vector<AST::EnumCase *> enum_cases;

    for (int enum_case_value = 0;
         this->current().type != lexer::TokenType::BRACE_CLOSE;
         enum_case_value++) {
        auto case_name = getAndAdvance(lexer::TokenType::IDENTIFIER,
                                       "THe enum case must have a name.");

        AST::Node *case_value;

        if (advanceIf(lexer::TokenType::EQUAL)) {
            case_value = this->expression();
        } else {
            auto case_value_string = std::to_string(enum_case_value);

            case_value = this->makeDeclaration<AST::LiteralExpression>(
                case_value_string, lexer::TokenType::V_INT);
        }

        auto enum_case =
            this->makeDeclaration<AST::EnumCase>(case_name, case_value);

        enum_cases.push_back(enum_case);
        advanceIf(lexer::TokenType::COMMA);
        advanceLines();
    }

    return enum_cases;
}

AST::RangeBasedForLoop *Parser::rangeBasedForLoop() {
    auto name = getAndAdvance(lexer::TokenType::IDENTIFIER,
                              "Expected a name for range-based for loops.");
    advance(lexer::TokenType::IN,
            "Range-based for loops must have `in` token.");

    auto name2 = expression();
    this->advance(
        lexer::TokenType::PARENS_CLOSE,
        "Expected a closing parenthesis after range-based for loops.");

    std::optional<AST::Node *> for_index = std::nullopt;
    if (advanceIf(lexer::TokenType::BITWISE_PIPE)) {
        is_parsing_struct = true;
        for_index = statement();
        advance(
            lexer::TokenType::BITWISE_PIPE,
            "Expected a closing pipe operator after range-based for loops.");
        is_parsing_struct = false;
    }

    this->advance(lexer::TokenType::BRACE_OPEN,
                  "Range-based for loops must have a body.");
    auto for_body = this->compound();
    this->advance(lexer::TokenType::BRACE_CLOSE,
                  "Range-based for loops must have a body that closes.");

    return this->makeDeclaration<AST::RangeBasedForLoop>(name, name2, for_index,
                                                         for_body);
}

AST::Node *Parser::forLoop() {
    this->advance(lexer::TokenType::FOR);

    this->advance(lexer::TokenType::PARENS_OPEN,
                  "Expected a opening parenthesis after a for loop.");

    if (peek().type == lexer::TokenType::IN) {
        return this->rangeBasedForLoop();
    }

    should_check_duplicates = false;
    auto for_initialization = this->statement();
    this->advance(lexer::TokenType::COMMA,
                  "Expected a comma after for loop's first expression.");

    auto for_condition = this->expression();
    this->advance(lexer::TokenType::COMMA,
                  "Expected a comma after for loop's second expression.");

    auto for_increment = this->statement();

    should_check_duplicates = true;

    this->advance(lexer::TokenType::PARENS_CLOSE,
                  "The for loop's parenthesis must be closed.");

    this->advance(lexer::TokenType::BRACE_OPEN,
                  "The for loop must have a body.");
    auto for_body = this->compound();
    this->advance(lexer::TokenType::BRACE_CLOSE,
                  "The for loop's body must be closed.");

    return this->makeDeclaration<AST::ForLoop>(
        for_initialization, for_condition, for_increment, for_body);
}

AST::WhileLoop *Parser::whileLoop() {
    advance(lexer::TokenType::WHILE);

    advance(lexer::TokenType::PARENS_OPEN,
            "The while loop must have a parenthesis.");
    auto expression = this->expression();
    advance(lexer::TokenType::PARENS_CLOSE,
            "While loop's parenthesis must be closed.");

    this->advance(lexer::TokenType::BRACE_OPEN,
                  "The while loop must have a body.");
    auto body = this->compound();
    this->advance(lexer::TokenType::BRACE_CLOSE,
                  "The while loop's body must be closed.");

    return this->makeDeclaration<AST::WhileLoop>(expression, body);
}

AST::Return *Parser::returnStatement(bool is_throw_statement) {
    if (is_throw_statement) {
        advance(lexer::TokenType::THROW);
    } else {
        advance(lexer::TokenType::RETURN);
    }

    std::optional<AST::Node *> return_value = std::nullopt;

    if (this->current().type != lexer::TokenType::NEW_LINE) {
        return_value = this->expression();
    }

    return this->makeDeclaration<AST::Return>(return_value, is_throw_statement);
}

AST::If *Parser::ifStatement() {
    advance(lexer::TokenType::IF);

    auto if_body_and_statement = this->ifElseStatements();

    std::vector<AST::Node *> elseif_conditions = {};
    std::vector<AST::Compound *> elseif_bodies = {};
    std::optional<AST::Compound *> else_body = std::nullopt;

    while (advanceIf(lexer::TokenType::ELSE)) {
        if (advanceIf(lexer::TokenType::IF)) {
            auto else_body_and_statement = this->ifElseStatements();

            elseif_conditions.push_back(else_body_and_statement.first);
            elseif_bodies.push_back(else_body_and_statement.second);
        } else {
            advance(lexer::TokenType::BRACE_OPEN,
                    "The else statement must have a body.");
            else_body = this->compound();
            advance(lexer::TokenType::BRACE_CLOSE,
                    "The else statement's body must be closed.");
            break;
        }
    }

    return this->makeDeclaration<AST::If>(
        if_body_and_statement.first, if_body_and_statement.second,
        elseif_conditions, elseif_bodies, else_body);
}

std::pair<AST::Node *, AST::Compound *> Parser::ifElseStatements() {
    advance(lexer::TokenType::PARENS_OPEN,
            "Expected a expression after a if or else statement.");
    auto condition = this->expression();
    advance(lexer::TokenType::PARENS_CLOSE,
            "Parenthesis must be closed inside an if statement.");

    advance(lexer::TokenType::BRACE_OPEN,
            "The if or else statement must have a body.");
    auto body = this->compound();
    advance(lexer::TokenType::BRACE_CLOSE,
            "The if or else statement's body must be closed.");

    return std::make_pair(condition, body);
}

AST::ASM *Parser::inlineAssembly() {
    advance(lexer::TokenType::ASM);
    advance(lexer::TokenType::PARENS_OPEN,
            "Expected a parenthesis after assembly.");
    advanceLines();

    std::vector<std::string_view> instructions;

    while (this->current().type != lexer::TokenType::PARENS_CLOSE) {
        if (this->current().type == lexer::TokenType::V_STRING) {
            instructions.push_back(getAndAdvance(
                lexer::TokenType::V_STRING,
                "The assembly instruction must be inside a string literal."));
        } else {
            instructions.push_back(
                getAndAdvance(lexer::TokenType::V_MULTILINE_STRING,
                              "The assembly instruction must be inside a "
                              "string or multiline."));
        }

        advanceLines();
    }

    advance(lexer::TokenType::PARENS_CLOSE, "Assembly must be closed.");

    return this->makeDeclaration<AST::ASM>(instructions);
}

AST::GOTO *Parser::gotoStatement() {
    advance(lexer::TokenType::GOTO);

    auto label = getAndAdvance(lexer::TokenType::IDENTIFIER,
                               "Expected a label after the goto statement.");

    return this->makeDeclaration<AST::GOTO>(label, true);
}

AST::SwitchStatement *Parser::switchStatement() {
    advance(lexer::TokenType::SWITCH);

    advance(lexer::TokenType::PARENS_OPEN,
            "The switch statement must have an opening parenthesis.");
    auto switch_expression = expression();
    advance(lexer::TokenType::PARENS_CLOSE,
            "The switch statement must have a closing parenthesis.");

    advance(lexer::TokenType::BRACE_OPEN,
            "The switch statement must have a body.");
    advanceLines();
    auto cases = switchCases();
    advance(lexer::TokenType::BRACE_CLOSE,
            "The body of the switch statement must end with a '}'.");

    return this->makeDeclaration<AST::SwitchStatement>(switch_expression,
                                                       cases);
}

std::vector<AST::SwitchCase *> Parser::switchCases() {
    std::vector<AST::SwitchCase *> cases;

    while (this->current().type != lexer::TokenType::BRACE_CLOSE) {
        cases.push_back(switchCase());
        advanceLines();
    }

    return cases;
}

AST::SwitchCase *Parser::switchCase() {
    bool is_case = advanceIf(lexer::TokenType::CASE);
    if (!is_case) {
        advance(lexer::TokenType::DEFAULT);
    }

    AST::Node *case_expression;
    if (is_case) {
        parses_goto_labels = false;
        case_expression = primary();
        parses_goto_labels = true;
    }

    advance(lexer::TokenType::COLON, "Expected a ':' after the switch case.");

    auto *case_body = this->makeDeclaration<AST::Compound>();

    advanceLines();
    while (this->current().type != lexer::TokenType::CASE &&
           this->current().type != lexer::TokenType::DEFAULT &&
           this->current().type != lexer::TokenType::BRACE_CLOSE) {
        case_body->statements.push_back(statement());
        advanceLines();
    }

    if (is_case) {
        return this->makeDeclaration<AST::SwitchCase>(case_expression,
                                                      case_body, is_case);
    } else {
        return this->makeDeclaration<AST::SwitchCase>(case_body, is_case);
    }
}

AST::DoCatchStatement *Parser::doCatchStatement() {
    advance(lexer::TokenType::DO);

    advance(lexer::TokenType::BRACE_OPEN,
            "Expected a '(' after the do statement case.");
    auto do_body = this->compound();
    advance(lexer::TokenType::BRACE_CLOSE,
            "Expected a ')' after the do statement case.");

    advance(lexer::TokenType::CATCH,
            "The do statement must have a catch clause.");
    std::optional<AST::Node *> catch_expression = std::nullopt;
    if (advanceIf(lexer::TokenType::PARENS_OPEN)) {
        is_parsing_struct = true;
        catch_expression = this->statement();
        advance(lexer::TokenType::PARENS_CLOSE,
                "Expected a ')' after the catch statement expression.");
        is_parsing_struct = false;
    }

    advance(lexer::TokenType::BRACE_OPEN,
            "The catch statement must have a body.");
    auto catch_body = this->compound();
    advance(lexer::TokenType::BRACE_CLOSE,
            "The catch statement's body must end.");

    return this->makeDeclaration<AST::DoCatchStatement>(do_body, catch_body,
                                                        catch_expression);
}

AST::Node *Parser::functionCall(
    std::string_view function_name,
    const std::optional<std::vector<AST::Node *>> &template_arguments) {

    auto arguments = functionCallArguments();
    advance(lexer::TokenType::PARENS_CLOSE,
            "Expected a closing parenthesis after function call.");

    auto decl = this->makeDeclaration<AST::FunctionCall>(
        function_name, arguments, template_arguments);

    if (this->current().type == lexer::TokenType::PERIOD) {
        return this->structMemberAccess(decl);
    } else if (this->current().type == lexer::TokenType::SQUARE_OPEN) {
        return arrayAccess(decl);
    }

    return decl;
}

AST::Node *Parser::arrayAccess() {
    // myVariable[40]
    auto variable_name =
        getAndAdvance(lexer::TokenType::IDENTIFIER,
                      "Expecteda  identifier after array access.");
    advance(lexer::TokenType::SQUARE_OPEN);
    auto inside = this->expression();
    advance(lexer::TokenType::SQUARE_CLOSE,
            "Expected a closing square after array access.");

    return this->makeDeclaration<AST::ArrayAccess>(variable_name, inside);
}

AST::Node *Parser::arrayAccess(AST::FunctionCall *function_call) {
    // myFunction()[30]
    advance(lexer::TokenType::SQUARE_OPEN);
    auto inside = this->expression();
    advance(lexer::TokenType::SQUARE_CLOSE,
            "Expected a closing square after array access.");

    return this->makeDeclaration<AST::ArrayAccess>(function_call, inside);
}

AST::Node *Parser::arrayCreation() {
    // [50, 30, 20]
    advance(lexer::TokenType::SQUARE_OPEN);

    std::vector<AST::Node *> items;
    while (this->current().type != lexer::TokenType::SQUARE_CLOSE) {
        items.push_back(this->expression());

        advanceIf(lexer::TokenType::COMMA);
    }

    advance(lexer::TokenType::SQUARE_CLOSE,
            "Expected a closing square after array creation.");

    return this->makeDeclaration<AST::ArrayCreation>(items);
}

std::vector<AST::Node *> Parser::functionCallArguments() {
    std::vector<AST::Node *> arguments;

    this->parses_goto_labels = false;
    bool argument_can_have_unnamed_arguments = true;
    while (this->current().type != lexer::TokenType::PARENS_CLOSE) {
        if (advanceIf(lexer::TokenType::NOT)) {
            if (this->current().type != lexer::TokenType::BRACE_OPEN) {
                goto argument_expression;
            }
            inside_function_body = true;
            advance(lexer::TokenType::BRACE_OPEN,
                    "Expected a '{' after using first class function.");
            auto body = this->compound();
            arguments.push_back(body);
            advance(lexer::TokenType::BRACE_CLOSE,
                    "Expected a '}' after using first class function.");
            advanceIf(lexer::TokenType::COMMA);
            inside_function_body = false;
            continue;
        }

        if (peek().type == lexer::TokenType::COLON) {
            // name based argument
            auto argument_name =
                getAndAdvance(lexer::TokenType::IDENTIFIER,
                              "Expected a identifier after using a name based "
                              "argument style");
            advance(lexer::TokenType::COLON);
            auto argument_value = this->expression();

            auto argument = this->makeDeclaration<AST::FunctionArgumentName>(
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

        advanceIf(lexer::TokenType::COMMA);
    }
    this->parses_goto_labels = true;

    return arguments;
}

AST::Node *Parser::typealias() {
    advance(lexer::TokenType::TYPEALIAS);
    auto type_name =
        getAndAdvance(lexer::TokenType::IDENTIFIER,
                      "Expected a type name after the type alias.");
    advance(lexer::TokenType::EQUAL, "Expected a type after the type alias.");
    auto value = expression();

    return this->makeDeclaration<AST::Typealias>(type_name, value);
}

AST::Node *Parser::token() {
    auto type = this->current().type;
    advance(type);
    return this->makeDeclaration<AST::ASTToken>(type);
}

AST::TemplateDeclaration *Parser::templateDeclaration() {
    advance(lexer::TokenType::AT);
    advance(lexer::TokenType::PARENS_OPEN,
            "Expected a '(' when using template variables.");

    auto arguments = templateArguments();

    advance(lexer::TokenType::PARENS_CLOSE,
            "Expected a ')' when using template variables.");

    return this->makeDeclaration<AST::TemplateDeclaration>(arguments);
}

std::vector<AST::TemplateDeclarationArgument *> Parser::templateArguments() {
    std::vector<AST::TemplateDeclarationArgument *> arguments;

    while (this->current().type != lexer::TokenType::PARENS_CLOSE) {
        if (!isTemplateKeyword(this->current().type)) {
            this->throwError("Invalid template type.");
        }

        auto argument_type = getAndAdvanceType();
        auto argument_name = getAndAdvance(
            lexer::TokenType::IDENTIFIER, "Expected a template argument name.");

        auto argument = this->makeDeclaration<AST::TemplateDeclarationArgument>(
            argument_name, argument_type);

        arguments.push_back(argument);

        if (!advanceIf(lexer::TokenType::COMMA)) {
            break;
        }
    }

    return arguments;
}

std::vector<AST::Node *> Parser::templateCallArguments() {
    this->advance(lexer::TokenType::AT);
    advance(lexer::TokenType::PARENS_OPEN,
            "Expected a '(' when using template calls.");

    std::vector<AST::Node *> template_values;
    while (this->current().type != lexer::TokenType::PARENS_CLOSE) {
        template_values.push_back(this->type());

        if (!advanceIf(lexer::TokenType::COMMA)) {
            break;
        }
    }

    return template_values;
}

// func main(argc: usize, let argv: string[])
std::vector<AST::FunctionArgument *> Parser::functionArguments() {
    std::vector<AST::FunctionArgument *> arguments;

    bool can_use_arguments_without_default_value = true;

    while (this->current().type != lexer::TokenType::PARENS_CLOSE) {
        bool is_constant = false;
        if (advanceIf(lexer::TokenType::LET)) {
            is_constant = true;
        }
        if (advanceIf(lexer::TokenType::PERIOD)) {
            advance(lexer::TokenType::PERIOD,
                    "Expected a '..' when using vaargs.");
            advance(lexer::TokenType::PERIOD,
                    "Expected a '.' when using vaargs.");

            auto argument = this->makeDeclaration<AST::FunctionArgument>();
            argument->is_vaarg = true;
            argument->is_constant = is_constant;
            argument->name = getAndAdvance(lexer::TokenType::IDENTIFIER,
                                           "Expected a vaarg name.");
            arguments.push_back(argument);

            break;
        }

        std::optional<std::string_view> argument_name;
        std::optional<AST::Node *> argument_default_value = std::nullopt;
        std::optional<AST::Node *> argument_type = std::nullopt;
        if (this->current().type == lexer::TokenType::IDENTIFIER) {
            argument_name = getAndAdvance(lexer::TokenType::IDENTIFIER);

            if (advanceIf(lexer::TokenType::COLON)) {
                argument_type = this->type();
            }

            if (advanceIf(lexer::TokenType::EQUAL)) {
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

        auto argument = this->makeDeclaration<AST::FunctionArgument>(
            argument_name, argument_type, argument_default_value, is_constant);

        arguments.push_back(argument);

        if (!advanceIf(lexer::TokenType::COMMA)) {
            break;
        }
    }

    return arguments;
}

AST::Node *Parser::expression() { return this->equality(); }

AST::Node *Parser::tryExpression() {
    advance(lexer::TokenType::TRY);
    bool is_force_cast = false;    // try!
    bool is_optional_cast = false; // try?

    if (advanceIf(lexer::TokenType::QUESTION)) {
        is_optional_cast = true;
    } else if (advanceIf(lexer::TokenType::NOT)) {
        is_force_cast = true;
    }

    auto expression = this->expression();

    return this->makeDeclaration<AST::TryExpression>(expression, is_force_cast,
                                                     is_optional_cast);
}

AST::Node *Parser::castExpression() {
    // cast?(50.3, int) || cast!(50.3, int)
    advance(lexer::TokenType::CAST);

    bool is_force_cast = false;
    bool is_optional_cast = false;

    if (advanceIf(lexer::TokenType::QUESTION)) {
        is_optional_cast = true;
    } else if (advanceIf(lexer::TokenType::NOT)) {
        is_force_cast = true;
    } else {
        this->throwError("The cast expression must be forced or optional.");
    }

    advance(lexer::TokenType::PARENS_OPEN,
            "THe cast expression must have '('.");
    auto cast_value = expression();
    advance(lexer::TokenType::COMMA,
            "The cast expression must have a comma after expression.");
    auto type = this->type();
    advance(lexer::TokenType::PARENS_CLOSE,
            "Expected a ')' at the end of the cast expression.");

    return this->makeDeclaration<AST::CastExpression>(
        cast_value, type, is_force_cast, is_optional_cast);
}

AST::TernaryExpression *Parser::ternaryExpression(AST::Node *bool_expression) {
    // (expression) ? 50 : 20
    // 50 == 30 ? return true : return false
    advance(lexer::TokenType::QUESTION);

    parses_goto_labels = false;
    auto first_expression = statement();
    advance(lexer::TokenType::COLON,
            "Expected a ':' after the ternary expression.");
    auto second_expression = statement();
    parses_goto_labels = true;

    return this->makeDeclaration<AST::TernaryExpression>(
        bool_expression, first_expression, second_expression);
}

AST::OptionalUnwrapExpression *Parser::optionalUnwrap(AST::Node *expression) {
    // myOptionalVariable ?? 40
    advance(lexer::TokenType::QUESTION);
    advance(lexer::TokenType::QUESTION);

    auto if_nilled_value = this->expression();

    return this->makeDeclaration<AST::OptionalUnwrapExpression>(
        expression, if_nilled_value);
}

AST::ForceUnwrapExpression *Parser::forceUnwrap(AST::Node *expression) {
    advance(lexer::TokenType::NOT);
    return this->makeDeclaration<AST::ForceUnwrapExpression>(expression);
}

AST::Node *Parser::equality() {
    auto expr = this->comparison();

    while (isEqualityOperator(this->current().type)) {
        lexer::TokenType operator_type = getAndAdvanceType();

        auto right_ast = this->comparison();

        expr = this->makeDeclaration<AST::BinaryExpression>(expr, right_ast,
                                                            operator_type);
    }

    return expr;
}

AST::Node *Parser::comparison() {
    auto expr = this->term();

    while (isComparativeOperator(this->current().type)) {
        lexer::TokenType operator_type = getAndAdvanceType();

        auto right_ast = this->term();

        expr = this->makeDeclaration<AST::BinaryExpression>(expr, right_ast,
                                                            operator_type);
    }

    return expr;
}

AST::Node *Parser::term() {
    auto expr = this->factor();

    while (isAdditiveOperator(this->current().type)) {
        lexer::TokenType operator_type = getAndAdvanceType();

        auto right_ast = this->factor();

        expr = this->makeDeclaration<AST::BinaryExpression>(expr, right_ast,
                                                            operator_type);
    }

    return expr;
}

AST::Node *Parser::factor() {
    auto expr = this->unary();

    while (isMultiplicativeOperator(this->current().type)) {
        lexer::TokenType operator_type = getAndAdvanceType();

        auto right_ast = this->unary();

        expr = this->makeDeclaration<AST::BinaryExpression>(expr, right_ast,
                                                            operator_type);
    }

    return expr;
}

AST::Node *Parser::unary() {
    if (isUnaryOperator(this->current().type)) {
        lexer::TokenType operator_type = getAndAdvanceType();

        auto expr = this->unary();

        return this->makeDeclaration<AST::UnaryExpression>(expr, operator_type);
    }

    return this->primary();
}

AST::Node *Parser::primary() {
    if (this->current().type == lexer::TokenType::TRY) {
        return tryExpression();
    }

    if (peek().type == lexer::TokenType::PERIOD) {
        return structMemberAccess();
    } else if (peek().type == lexer::TokenType::SQUARE_OPEN) {
        return arrayAccess();
    }

    if (this->current().type == lexer::TokenType::SQUARE_OPEN) {
        return arrayCreation();
    }

    if (this->current().type == lexer::TokenType::PERIOD) {
        return initOrEnumCase();
    }
    if (isRegularValue(this->current().type)) {
        auto literal = this->makeDeclaration<AST::LiteralExpression>(
            this->current().value(this->error.buffer), this->current().type);
        advance();

        if (advanceIf(lexer::TokenType::PARENS_OPEN)) {
            return this->functionCall(literal->value);
        } else if (parses_goto_labels) {
            if (advanceIf(lexer::TokenType::COLON)) {
                return this->makeDeclaration<AST::GOTO>(literal->value, false);
            }
        }

        if (this->current().type == lexer::TokenType::AT) {
            auto template_arguments = this->templateCallArguments();

            advance(
                lexer::TokenType::PARENS_CLOSE,
                "Expected a ')' after the function call template arguments.");
            // TODO: Optional Templates
            advance(
                lexer::TokenType::PARENS_OPEN,
                "Expected a '(' after the function call template arguments.");

            // TODO: Just to function call for now, in the future, when there
            // will be nested structs, we might have to make this more complex
            return this->functionCall(literal->value, template_arguments);
        }

        if (this->current().type == lexer::TokenType::QUESTION) {
            if (peek().type == lexer::TokenType::QUESTION) {
                return optionalUnwrap(literal);
            }
            return ternaryExpression(literal);
        }

        if (this->current().type == lexer::TokenType::NOT) {
            return forceUnwrap(literal);
        }

        return literal;
    } else if (advanceIf(lexer::TokenType::PARENS_OPEN)) {
        auto expr = this->expression();
        advance(lexer::TokenType::PARENS_CLOSE,
                "Expected a closing parenthesis after group expression.");

        return this->makeDeclaration<AST::GroupingExpression>(expr);
    } else if (this->current().type == lexer::TokenType::SELF) {
        return structMemberAccess();
    } else if (this->current().type == lexer::TokenType::CAST) {
        return castExpression();
    } else {
        std::cout << lexer::tokenTypeAsLiteral(this->current().type);
        this->throwError("Invalid expression.");
    }
    exit(-1);
}

AST::FirstClassFunction *Parser::firstClassFunction() {
    // $bool(int, string)
    advance(lexer::TokenType::DOLLAR);

    bool is_type = true;
    std::optional<AST::Node *> return_type = std::nullopt;
    if (this->current().type == lexer::TokenType::PARENS_OPEN) {
        is_type = false;
    } else {
        return_type = this->type();
    }
    advance(lexer::TokenType::PARENS_OPEN,
            "Expected a '(' at the start of a first class function.");
    auto arguments = this->functionArguments();
    advance(lexer::TokenType::PARENS_CLOSE,
            "Expected a ')' at the end of a first class function.");

    return this->makeDeclaration<AST::FirstClassFunction>(return_type,
                                                          arguments, is_type);
}

AST::Node *Parser::type() {
    auto type = this->makeDeclaration<AST::Type>(
        current().type, current().value(this->error.buffer), false, false,
        false, false);

    if (!isRegularType(this->current().type)) {
        std::cout << lexer::tokenTypeAsLiteral(this->current().type) << '\n';
        this->throwError("Parser: Expected type\n");
    }

    if (this->current().type == lexer::TokenType::DOLLAR) {
        return this->firstClassFunction();
    }

    advance();

    for (;;) {
        switch (this->current().type) {
        case lexer::TokenType::QUESTION:
            type->is_optional = true;
            break;
        case lexer::TokenType::OPERATOR_MUL:
            type->is_pointer = true;
            break;
        case lexer::TokenType::SQUARE_OPEN:
            this->advance();
            type->is_array = true;
            break;
        case lexer::TokenType::NOT:
            type->is_throw_statement = true;
            break;
        case lexer::TokenType::AT:
            type->template_values = templateCallArguments();
            advance(lexer::TokenType::PARENS_CLOSE,
                    "Expected a closing template argument.");
        default:
            goto end;
        }

        this->advance();
    }

end:
    return type;
}

AST::Node *Parser::qualifiers() {
    auto qualifiers = getQualifiers();

    if (this->current().type == lexer::TokenType::ENUM) {
        return this->enumDeclaration(qualifiers);
    } else if (this->current().type == lexer::TokenType::STRUCT ||
               this->current().type == lexer::TokenType::UNION) {
        return this->structDeclaration(qualifiers);
    } else if (this->current().type == lexer::TokenType::VAR) {
        return this->variableDeclaration(qualifiers);
    } else if (this->current().type == lexer::TokenType::LET) {
        return variableDeclaration(qualifiers, true);
    } else if (this->current().type == lexer::TokenType::FUNC) {
        return this->functionDeclaration(qualifiers);
    }
    this->throwError("Expected enum, struct, function or variable.");
    exit(-1);
}

std::vector<lexer::TokenType> Parser::getQualifiers() {
    std::vector<lexer::TokenType> modifiers;
    while (this->current().type == lexer::TokenType::EXTERN ||
           this->current().type == lexer::TokenType::VOLATILE ||
           this->current().type == lexer::TokenType::PRIVATE) {
        modifiers.push_back(this->current().type);
        advance();
    }
    return modifiers;
}

void Parser::advance() { this->index += 1; }

void Parser::reverse() { this->index -= 1; }

void Parser::advance(lexer::TokenType type, const char *error_message) {
    if (this->current().type == type) {
        this->advance();
    } else {
        //        std::cout << lexer::tokenTypeAsLiteral(this->current().type)
        //                  << " :: " << lexer::tokenTypeAsLiteral(type)
        //                  << " :: " << this->current().location.toString() <<
        //                  '\n';
        this->throwError(error_message);
    }
}

bool Parser::advanceIf(lexer::TokenType type) {
    if (this->current().type == type) {
        this->advance();
        return true;
    }

    return false;
}

void Parser::advanceLines() {
    while (this->current().type == lexer::TokenType::NEW_LINE) {
        advance();
    }
}

lexer::TokenType Parser::getAndAdvanceType() {
    auto value = this->current().type;
    advance();
    return value;
}

std::string_view Parser::getAndAdvance() {
    auto value = this->current().value(this->error.buffer);
    advance();
    return value;
}

std::string_view Parser::getAndAdvance(lexer::TokenType type,
                                       const char *message) {
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

} // namespace drast::parser
