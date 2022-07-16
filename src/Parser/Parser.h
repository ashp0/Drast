//
// Created by Ashwin Paudel on 2022-06-03.
//

#ifndef DRAST_PARSER_H
#define DRAST_PARSER_H

#include "../Node/Node.h"
#include "../Lexer/Lexer.h"
#include "../Lexer/Token.h"
#include "../Common/Report.h"

class Parser {
private:
    Lexer lexer;
    size_t index = 0;

    size_t indent = 0;
    bool should_advance = true;

    bool variable_initialization = true;
public:
    Compound *root;

    std::vector<Token> tokens;
public:
    explicit Parser(Lexer &lexer);

    void parse();

private: /* Statements */
    void parseCompound();

    Block *parseBlock();

    Node *parseStatement();

    void parseStructStatement(StructDeclaration *struct_declaration);

    Import *parseImport();

    StructDeclaration *parseStruct();

    EnumDeclaration *parseEnum();

    FunctionDeclaration *parseFunctionDeclaration();

    IfStatement *parseIf();

    WhileStatement *parseWhile();

    ForLoop *parseForLoop();

    Node *parseIdentifier();

    VariableDeclaration *parseVariable();

    VariableDeclaration *parseConstant();

    VariableDeclaration *parseVariable(const std::string &variable_name);

    VariableDeclaration *parseVariable(const std::string &variable_name, TypeNode *type);

    ReturnStatement *parseReturn();

private: /* Expressions */
    Expression *parseExpression();

    Expression *parseAssignment();

    Expression *parseEquality();

    Expression *parseComparison();

    Expression *parseTerm();

    Expression *parseFactor();

    Expression *parseUnary();

    Expression *finishCall(Expression *expr);

    Expression *parseDot();

    Expression *parsePrimary();

    Argument *parseArgument();

    TypeNode *parseType();

    TypeNode *parseLiteralType(bool is_array = false);

    LiteralTokenType *parseLiteralTokenType();

private:
    [[nodiscard]] Token current() const;

    [[nodiscard]] Token peek() const;

    [[nodiscard]] Token peek(size_t offset) const;

    void advance();

    bool advanceIf(TokenType type);

    void advanceLines();

    bool evaluateTab();

    void consume(TokenType type, const std::string &message);

    void consume(TokenType type);

    void check(TokenType type, const std::string &message);
};


#endif //DRAST_PARSER_H
