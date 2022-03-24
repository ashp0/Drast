//
// Created by Ashwin Paudel on 2022-03-20.
//

#ifndef DRAST_PARSER_H
#define DRAST_PARSER_H

#include "AST.h"
#include "Print.h"
#include "Token.h"
#include <iostream>
#include <optional>
#include <vector>

class Parser {
  private:
    std::string &file_name;
    std::vector<std::unique_ptr<Token>> &tokens;

    size_t index;
    std::unique_ptr<Token> current;

  public:
    Parser(std::string &file_name, std::vector<std::unique_ptr<Token>> &tokens)
        : file_name(file_name), tokens(tokens), index(0),
          current(std::move(tokens[index])) {}

    std::unique_ptr<AST> parse();

  private:
    std::unique_ptr<CompoundStatement> compound();

    std::unique_ptr<AST> statement();

    std::unique_ptr<Import> import();

    std::unique_ptr<EnumDeclaration> enumDeclaration();

    std::vector<std::unique_ptr<EnumCase>> enumCases();

    std::unique_ptr<AST>
    functionOrVariableDeclaration(std::vector<TokenType> modifiers = {});

    std::unique_ptr<AST>
    functionDeclaration(std::unique_ptr<AST> &return_type,
                        std::vector<TokenType> modifiers = {});

    std::vector<std::unique_ptr<FunctionArgument>> functionArguments();

    std::unique_ptr<AST>
    variableDeclaration(std::unique_ptr<AST> &variable_type,
                        std::vector<TokenType> modifiers = {});

    std::unique_ptr<Return> returnStatement();

    std::unique_ptr<If> ifStatements();

    std::pair<std::unique_ptr<AST>, std::unique_ptr<CompoundStatement>>
    ifOrElseStatement();

    std::unique_ptr<ASM> inlineAssembly();

    std::unique_ptr<AST> expression();

    std::unique_ptr<AST> equality();

    std::unique_ptr<AST> comparison();

    std::unique_ptr<AST> term();

    std::unique_ptr<AST> factor();

    std::unique_ptr<AST> unary();

    std::unique_ptr<AST> primary();

    std::unique_ptr<AST> functionCall(std::string &name);

    std::unique_ptr<AST> type();

    std::unique_ptr<AST> modifiers();

    void advance(TokenType type);

    void advance();

    bool is(TokenType type);

    template <class ast_type, class... Args>
    std::unique_ptr<ast_type> create_declaration(Args &&...args);

    int throw_error(std::string message);
};

#endif // DRAST_PARSER_H
