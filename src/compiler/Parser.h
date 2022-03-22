//
// Created by Ashwin Paudel on 2022-03-20.
//

#ifndef DRAST_PARSER_H
#define DRAST_PARSER_H

#include "AST.h"
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

    std::unique_ptr<AST>
    functionOrVariableDeclaration(std::vector<TokenType> modifiers = {});

    std::unique_ptr<AST>
    functionDeclaration(std::unique_ptr<AST> &return_type,
                        std::vector<TokenType> modifiers = {});

    std::vector<std::unique_ptr<FunctionArgument>> functionArguments();

    std::unique_ptr<AST>
    variableDeclaration(std::unique_ptr<AST> &variable_type,
                        std::vector<TokenType> modifiers = {});

    std::unique_ptr<AST> expression();

    std::unique_ptr<AST> equality();

    std::unique_ptr<AST> comparison();

    std::unique_ptr<AST> term();

    std::unique_ptr<AST> factor();

    std::unique_ptr<AST> unary();

    std::unique_ptr<AST> primary();

    std::unique_ptr<AST> type();

    std::unique_ptr<AST> modifiers();

    void advance(TokenType type);

    void advance();
};

#endif // DRAST_PARSER_H
