//
// Created by Ashwin Paudel on 2022-06-03.
//

#include "Parser.h"

#include <utility>


Parser::Parser(Lexer &lexer) : lexer(lexer) {
    root = new Compound(Location(0, 0));
}

void Parser::parse() {
    parseCompound();
}

void Parser::parseCompound() {
    tokens.push_back(lexer.getToken());
    tokens.push_back(lexer.getToken());

    advanceLines();
    while (current().type != TokenType::T_EOF) {
        auto stmt = parseStatement();

        root->statements.emplace_back(stmt);

        if (should_advance) {
            if (current().type != TokenType::T_EOF) {
                if (current().type == TokenType::NEW_LINE ||
                    current().type == TokenType::SEMICOLON) {
                    advance();
                } else {
                    throw Report("Expected new line or semicolon", current().location);
                }
            }

            advanceLines();
        } else {
            should_advance = true;
            advanceLines();
        }
    }
}


Block *Parser::parseBlock() {
    auto block = new Block(current().location);
    indent += 1;
    advanceLines();

    for (;;) {
        start:
        if (evaluateTab()) {
            advance();
            if (current().type == TokenType::NEW_LINE) {
                advanceLines();
                goto start;
            }
            auto stmt = parseStatement();
            block->statements.push_back(stmt);

            if (should_advance) {
                if (current().type != TokenType::T_EOF) {
                    if (current().type == TokenType::NEW_LINE ||
                        current().type == TokenType::SEMICOLON) {
                        advance();
                    } else {
                        throw Report("Expected new line or semicolon", current().location);
                    }
                }
            } else {
                should_advance = true;
            }
        } else {
            should_advance = false;
            indent -= 1;
            return block;
        }
        advanceLines();
    }
}

// using Statement = Node*
Node *Parser::parseStatement() {
    switch (current().type) {
        case TokenType::IMPORT:
            return parseImport();
        case TokenType::STRUCT:
            return parseStruct();
        case TokenType::ENUM:
            return parseEnum();
        case TokenType::FN:
            return parseFunctionDeclaration();
        case TokenType::LET:
            return parseConstant();
        case TokenType::LV_IDENTIFIER:
            return parseIdentifier();
        case TokenType::IF:
            return parseIf();
        case TokenType::WHILE:
            return parseWhile();
        case TokenType::FOR:
            return parseForLoop();
        case TokenType::BREAK:
        case TokenType::CONTINUE:
            return parseLiteralTokenType();
        case TokenType::RETURN:
            return parseReturn();
        case TokenType::TAB:
            throw Report("There is a problem in your tabs, fix it!", current().location);
        default:
            throw Report("Token cannot be parsed", current().location);
    }
}

void Parser::parseStructStatement(StructDeclaration *struct_declaration) {
    switch (current().type) {
        case TokenType::FN:
            variable_initialization = true;
            struct_declaration->functions.push_back(parseFunctionDeclaration());
            variable_initialization = false;
            break;
        case TokenType::LV_IDENTIFIER:
            variable_initialization = false;
            struct_declaration->variables.push_back(parseVariable());
            break;
        case TokenType::LET:
            variable_initialization = false;
            struct_declaration->constants.push_back(parseConstant());
            break;
        default:
            throw Report("Structs are allowed only variables, constants and functions");
    }
}

Import *Parser::parseImport() {
    advance();
    Import *val;
    if (current().type == TokenType::LV_STRING) {
        val = new Import(current().literal.value(), current().location);
    } else if (current().type == TokenType::LV_IDENTIFIER) {
        val = new Import(current().literal.value(), true, current().location);
    } else {
        throw Report("Expected string or identifier", current().location);
    }

    advance();
    return val;
}

StructDeclaration *Parser::parseStruct() {
    auto struct_declaration = new StructDeclaration(current().location);

    advance(); // struct
    check(TokenType::LV_IDENTIFIER, "Expected identifier after struct declaration");
    struct_declaration->name = current().literal.value();
    advance();
    consume(TokenType::COLON, "Expected ':'");
    advanceLines();

    indent += 1;
    variable_initialization = false;

    for (;;) {
        start:
        if (evaluateTab()) {
            advance();
            if (current().type == TokenType::NEW_LINE) {
                advanceLines();
                goto start;
            }
            parseStructStatement(struct_declaration);

            if (should_advance) {
                if (current().type != TokenType::T_EOF) {
                    if (current().type == TokenType::NEW_LINE ||
                        current().type == TokenType::SEMICOLON) {
                        advance();
                    } else {
                        throw Report("Expected new line or semicolon", current().location);
                    }
                }
            } else {
                should_advance = true;
            }
        } else {
            indent -= 1;
            should_advance = false;
            variable_initialization = true;
            return struct_declaration;
        }
        advanceLines();
    }
}

EnumDeclaration *Parser::parseEnum() {
    advance(); // enum
    check(TokenType::LV_IDENTIFIER, "Expected identifier after enum declaration");
    auto name = current().literal.value();
    advance();
    consume(TokenType::COLON, "Expected ':'");
    advanceLines();

    auto enum_declaration = new EnumDeclaration(name, current().location);

    indent += 1;

    while (evaluateTab()) {
        advance();
        check(TokenType::LV_IDENTIFIER, "Expected identifier as an enum case");

        enum_declaration->cases.push_back(current().literal.value());
        advance();
        advanceLines();
    }

    indent -= 1;
    should_advance = false;

    return enum_declaration;
}

FunctionDeclaration *Parser::parseFunctionDeclaration() {
    auto function_declaration = new FunctionDeclaration(current().location);
    advance(); // fn

    if (advanceIf(TokenType::PERIOD)) {
        function_declaration->is_struct_function = true;
    }
    check(TokenType::LV_IDENTIFIER, "Expected identifier after function declaration");
    function_declaration->name = current().literal.value();
    advance();

    consume(TokenType::PARENS_OPEN, "Expected '('");
    while (current().type != TokenType::PARENS_CLOSE) {
        auto arg = parseArgument();
        function_declaration->arguments.push_back(arg);

        if (current().type == TokenType::COMMA) {
            advance();
        } else {
            break;
        }
    }
    consume(TokenType::PARENS_CLOSE, "Expected ')'");

    if (current().type == TokenType::ARROW) {
        advance();
        function_declaration->return_type = parseType();
    }

    consume(TokenType::COLON, "Expected ':'");

    auto block = parseBlock();

    function_declaration->block = block;

    return function_declaration;
}

IfStatement *Parser::parseIf() {
    auto if_statement = new IfStatement(current().location);
    advance();

    // Starting of the if statement
    if_statement->if_condition = new IfCondition(current().location);
    if_statement->if_condition->expr = parseExpression();
    consume(TokenType::COLON, "Expected ': after if statement");
    if_statement->if_condition->block = parseBlock();
    if_statement->if_condition->is_if = true;

    // The elif statements
    if (evaluateTab()) {
        advance();
        while (advanceIf(TokenType::ELIF)) {
            auto else_if_condition = new IfCondition(current().location);
            else_if_condition->expr = parseExpression();
            consume(TokenType::COLON, "Expected ': after elif statement");
            else_if_condition->block = parseBlock();

            if_statement->elif_conditions.push_back(else_if_condition);

            if (evaluateTab()) {
                advance();
            } else {
                break;
            }
        }
    }

    // Else statement
    if (advanceIf(TokenType::ELSE)) {
        consume(TokenType::COLON, "Expected ': after else statement");
        if_statement->else_block = parseBlock();
    }

    return if_statement;
}

WhileStatement *Parser::parseWhile() {
    auto while_statement = new WhileStatement(current().location);
    advance();
    while_statement->condition = parseExpression();
    consume(TokenType::COLON);
    while_statement->body = parseBlock();

    return while_statement;
}

ForLoop *Parser::parseForLoop() {
    auto for_loop = new ForLoop(current().location);
    advance();

    if (peek().type != TokenType::IN) {
        throw Report("Invalid for Loop");
    }

    check(TokenType::LV_IDENTIFIER, "Expected identifier");
    for_loop->variable_name = current().literal.value();
    advance(); // {name}
    advance(); // in

    for_loop->array = parseExpression();

    // Syntax analyzer will check after and change value accordingly.
    for_loop->by = "1";

    consume(TokenType::COLON);
    for_loop->body = parseBlock();

    return for_loop;
}

Node *Parser::parseIdentifier() {
    if (peek().type == TokenType::COLON) {
        check(TokenType::LV_IDENTIFIER, "Expected identifier");
        auto name = current().literal.value();
        advance();
        advance();
        auto type = parseType();
        if (variable_initialization) {
            consume(TokenType::EQUAL, "Expected expression");
        }

        return parseVariable(name, type);
    } else if (peek().type == (TokenType::DECLARE_EQUAL)) {
        check(TokenType::LV_IDENTIFIER, "Expected identifier");
        auto name = current().literal.value();
        advance();
        advance();
        return parseVariable(name);
    } else {
        return parseExpression();
    }
}

VariableDeclaration *Parser::parseVariable() {
    if (peek().type == TokenType::COLON) {
        check(TokenType::LV_IDENTIFIER, "Expected identifier");
        auto name = current().literal.value();
        advance();
        advance();
        auto type = parseType();
        if (variable_initialization) {
            consume(TokenType::EQUAL, "Expected expression");
        }

        return parseVariable(name, type);
    } else if (peek().type == (TokenType::DECLARE_EQUAL)) {
        check(TokenType::LV_IDENTIFIER, "Expected identifier");
        auto name = current().literal.value();
        advance();
        advance();
        return parseVariable(name);
    }

    throw Report("Was expecting variable", current().location);
}

ConstantDeclaration *Parser::parseConstant() {
    auto constant = new ConstantDeclaration(current().location);
    advance(); // let

    check(TokenType::LV_IDENTIFIER, "Expected identifier after enum declaration");
    constant->const_name = current().literal.value();
    advance();

    if (advanceIf(TokenType::DECLARE_EQUAL)) {
        constant->value = parseExpression();
    } else if (advanceIf(TokenType::COLON)) {
        constant->const_type = parseType();
        consume(TokenType::EQUAL);
        constant->value = parseExpression();
    } else {
        throw Report("Expected ':' or ':=' after constant declaration");
    }

    return constant;
}

VariableDeclaration *Parser::parseVariable(const std::string &variable_name) {
    auto expr = parseExpression();
    return new VariableDeclaration(variable_name, expr, current().location);
}

VariableDeclaration *Parser::parseVariable(const std::string &variable_name, TypeNode *type) {
    auto variable_declaration = new VariableDeclaration(variable_name, type, current().location);
    if (variable_initialization) {
        variable_declaration->expr = parseExpression();
        return variable_declaration;
    } else {
        if (advanceIf(TokenType::EQUAL)) {
            variable_declaration->expr = parseExpression();
            return variable_declaration;
        }

        return variable_declaration;
    }
}

ReturnStatement *Parser::parseReturn() {
    auto return_statement = new ReturnStatement(current().location);
    advance();
    return_statement->expr = parseExpression();

    return return_statement;
}

Expression *Parser::parseExpression() {
    return parseAssignment();
}

Expression *Parser::parseAssignment() {
    auto expr = parseEquality();

    while (isAssignmentOp(current().type)) {
        auto op = current().type;
        advance();

        auto right = parseAssignment();

        if (op == TokenType::EQUAL) {
            if (expr->type == NodeType::LITERAL) {
                return new Assign(expr, right, current().location);
            }

            throw Report("Invalid assignment", current().location);
        }

        expr = new Binary(expr, op, right, current().location);
    }

    return expr;
}

Expression *Parser::parseEquality() {
    auto left = parseComparison();

    while (isEqualityOp(current().type)) {
        auto op = current().type;
        advance();
        auto right = parseComparison();

        left = new Binary(left, op, right, current().location);
    }

    return left;
}

Expression *Parser::parseComparison() {
    auto left = parseTerm();

    while (isComparisonOp(current().type)) {
        auto op = current().type;
        advance();
        auto right = parseTerm();

        left = new Binary(left, op, right, current().location);
    }

    return left;
}

Expression *Parser::parseTerm() {
    auto left = parseFactor();

    while (isTermOp(current().type)) {
        auto op = current().type;
        advance();
        auto right = parseFactor();

        left = new Binary(left, op, right, current().location);
    }

    return left;
}

Expression *Parser::parseFactor() {
    auto left = parseUnary();

    while (isFactorOp(current().type)) {
        auto op = current().type;
        advance();
        auto right = parseUnary();

        left = new Binary(left, op, right, current().location);
    }

    return left;
}

Expression *Parser::parseUnary() {
    if (isUnaryOp(current().type)) {
        auto op = current().type;
        advance();
        auto right = parseUnary();

        return new Unary(right, op, current().location);
    }

    return parseDot();
}

Expression *Parser::finishCall(Expression *expr) {
    auto call = new Call(expr, current().location);

    if (current().type == TokenType::PARENS_CLOSE) {
        goto end;
    }
    do {
        call->arguments.push_back(parseExpression());
    } while (advanceIf(TokenType::COMMA));

    end:
    consume(TokenType::PARENS_CLOSE);

    return call;
}

Expression *Parser::parseDot() {
    if (advanceIf(TokenType::PERIOD)) {
        // Enum access
        check(TokenType::LV_IDENTIFIER, "Expected identifier after using enum dot notation");
        auto name = current().literal.value();
        advance();

        return new EnumDot(name, current().location);
    }

    auto expr = parsePrimary();

    // Range
    if (current().type == TokenType::PERIOD && peek().type == TokenType::PERIOD) {
        advance();
        advance();
        auto to = parseEquality();
        expr = new Range(expr, to, current().location);

    } else if (advanceIf(TokenType::PERIOD)) {
        check(TokenType::LV_IDENTIFIER, "Expected identifier after using dot notation");
        auto second = parseExpression();

        expr = new Get(expr, second, current().location);
    }

    return expr;
}

Expression *Parser::parsePrimary() {
    Expression *expr;
    if (isPrimaryType(current().type)) {
        expr = new Literal(current().type, current().literal.value(), current().location);
        advance();
    } else if (advanceIf(TokenType::PARENS_OPEN)) {
        auto expr1 = parseExpression();
        consume(TokenType::PARENS_CLOSE, "Expected ')'");
        expr = new Grouping(expr1, current().location);
    } else if (advanceIf(TokenType::SQUARE_OPEN)) {
        auto array = new Array(current().location);
        do {
            auto item = parseExpression();
            array->items.push_back(item);
        } while (advanceIf(TokenType::COMMA));
        consume(TokenType::SQUARE_CLOSE, "Expected closing brace");
        expr = array;
    } else {
        std::cout << tokenTypeToString(current().type);
        throw Report("Expected Literal Value");
    }

    if (advanceIf(TokenType::PARENS_OPEN)) {
        expr = finishCall(expr);
    } else if (advanceIf(TokenType::SQUARE_OPEN)) {
        // TODO: Array calls
    }

    return expr;
}

Argument *Parser::parseArgument() {
    auto argument = new Argument(current().location);
    check(TokenType::LV_IDENTIFIER, "Expected Identifier");
    argument->name = current().literal.value();
    advance();
    consume(TokenType::COLON, "Expected colon");
    argument->arg_type = parseType();

    return argument;
}

TypeNode *Parser::parseType() {
    if (current().type == TokenType::SQUARE_OPEN) {
        advance();
        auto type = parseLiteralType(true);
        consume(TokenType::SQUARE_CLOSE, "Expected closing ']'");
        return type;
    }

    return parseLiteralType();
}

TypeNode *Parser::parseLiteralType(bool is_array) {
    TypeNode *type;
    switch (current().type) {
        case TokenType::INT:
        case TokenType::FLOAT:
        case TokenType::STRING:
        case TokenType::CHAR:
        case TokenType::BOOL:
            type = new TypeNode(current().type, is_array, current().location);
            break;
        case TokenType::LV_IDENTIFIER:
            type = new TypeNode(current().literal.value(), is_array, current().location);
            break;
        default:
            throw Report("Invalid Type");
    }

    advance();
    return type;
}

LiteralTokenType *Parser::parseLiteralTokenType() {
    auto type = new LiteralTokenType(current().type, current().location);
    advance();
    return type;
}

Token Parser::current() const {
    return tokens[index];
}

Token Parser::peek() const {
    return tokens[index + 1];
}

Token Parser::peek(size_t offset) const {
    return tokens[index + offset];
}

void Parser::advance() {
    index++;
    tokens.push_back(lexer.getToken());
}

bool Parser::advanceIf(TokenType type) {
    if (current().type == type) {
        advance();
        return true;
    }
    return false;
}

void Parser::advanceLines() {
    while (current().type == TokenType::NEW_LINE) {
        advance();
    }
}

bool Parser::evaluateTab() {
    return current().type == TokenType::TAB && current().tab_width == indent;
}

void Parser::consume(TokenType type, const std::string &message) {
    if (current().type == type) {
        advance();
        return;
    }
    throw Report(message, current().location);
}

void Parser::consume(TokenType type) {
    if (current().type == type) {
        advance();
    } else {
        throw Report("Expected " + tokenTypeToString(type) + " but got " + tokenTypeToString(current().type),
                     current().location);
    }
}

void Parser::check(TokenType type, const std::string &message) {
    if (current().type != type) {
        throw Report(message, current().location);
    }
}