//
// Parser.h
// Created by Ashwin Paudel on 2022-03-26.
//
// =============================================================================
///
/// \file
/// This file contains the declaration of the Parser class. Which is used by the
/// semantic analyzer to turn the tokens into an Abstract Syntax Tree.
///
// =============================================================================
//
// Copyright (c) 2022, Drast Programming Language Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.
//
// =============================================================================
//
// Contributed by:
//  - Ashwin Paudel <ashwonixer123@gmail.com>
//
// =============================================================================

#ifndef DRAST_PARSER_H
#define DRAST_PARSER_H

#include "../AST/AST.h"
#include "../Lexer/Lexer.h"
#include "../Lexer/Token.h"
#include "../Lexer/TokenUtils.h"
#include <iostream>
#include <iterator>
#include <map>
#include <utility>
#include <vector>

namespace drast::parser {

class Parser {
  private:
    lexer::Lexer &lexer;
    size_t index = 0;

    Error &error;
    bool did_encounter_error = false;

    bool parses_goto_labels = true;
    bool inside_function_body = false; // myFunction(!{return true})
    bool is_parsing_struct = false;
    bool should_check_duplicates = true;

    std::vector<lexer::Token> tokens;
    AST::Compound *current_compound = nullptr;

  public:
    Parser(lexer::Lexer &lexer, Error &error) : lexer(lexer), error(error) {
        tokens.push_back(lexer.getToken());
        tokens.push_back(lexer.getToken());
    }

    AST::Node *parse();

  private:
    AST::Compound *compound();

    AST::Node *statement();

    AST::Import *import();

    AST::StructDeclaration *
    structDeclaration(const std::vector<lexer::TokenType> &qualifiers = {});

    AST::Node *structMemberAccess();

    AST::Node *structMemberAccess(AST::FunctionCall *function_call);

    AST::Node *initOrEnumCase();

    AST::StructInitializerDeclaration *structInitializerDeclaration();

    AST::Node *structDestructorDeclaration();

    AST::EnumDeclaration *
    enumDeclaration(const std::vector<lexer::TokenType> &qualifiers = {});

    std::vector<AST::EnumCase *> enumCases();

    AST::Node *
    functionDeclaration(const std::vector<lexer::TokenType> &qualifiers = {});

    AST::Node *
    variableDeclaration(const std::vector<lexer::TokenType> &qualifiers = {},
                        bool is_let = false);

    AST::RangeBasedForLoop *rangeBasedForLoop();

    AST::Node *forLoop();

    AST::WhileLoop *whileLoop();

    AST::Return *returnStatement(bool is_throw_statement);

    AST::If *ifStatement();

    std::pair<AST::Node *, AST::Compound *> ifElseStatements();

    AST::ASM *inlineAssembly();

    AST::GOTO *gotoStatement();

    AST::SwitchStatement *switchStatement();

    std::vector<AST::SwitchCase *> switchCases();

    AST::SwitchCase *switchCase();

    AST::DoCatchStatement *doCatchStatement();

    AST::OperatorOverload *operatorOverload();

    AST::Node *expression();

    AST::Node *tryExpression();

    AST::Node *castExpression();

    AST::TernaryExpression *ternaryExpression(AST::Node *bool_expression);

    AST::OptionalUnwrapExpression *optionalUnwrap(AST::Node *expression);

    AST::ForceUnwrapExpression *forceUnwrap(AST::Node *expression);

    AST::Node *equality();

    AST::Node *comparison();

    AST::Node *term();

    AST::Node *factor();

    AST::Node *unary();

    AST::Node *primary();

    AST::Node *functionCall(std::string_view function_name,
                            const std::optional<std::vector<AST::Node *>>
                                &template_arguments = std::nullopt);

    AST::Node *arrayAccess();

    AST::Node *arrayAccess(AST::FunctionCall *function_call);

    AST::Node *arrayCreation();

    std::vector<AST::Node *> functionCallArguments();

    AST::Node *typealias();

    AST::Node *token();

    AST::TemplateDeclaration *templateDeclaration();

    std::vector<AST::TemplateDeclarationArgument *> templateArguments();

    std::vector<AST::Node *> templateCallArguments();

    std::vector<AST::FunctionArgument *> functionArguments();

    AST::FirstClassFunction *firstClassFunction();

    AST::Node *type();

    AST::Node *qualifiers();

    std::vector<lexer::TokenType> getQualifiers();

  private:
    void advance();

    inline void reverse();

    void advance(lexer::TokenType type, const char *message = "");

    inline lexer::Token &peek() { return tokens[index + 1]; }

    inline lexer::Token &current() { return tokens[index]; }

    void advanceLines();

    bool advanceIf(lexer::TokenType type);

    lexer::TokenType getAndAdvanceType();

    std::string_view getAndAdvance();

    std::string_view getAndAdvance(lexer::TokenType type,
                                   const char *message = "");

    template <class ast_type, class... Args>
    ast_type *makeDeclaration(Args &&...args);

    int throwError(const char *message);

    int throwError(const char *message, Location location);
};

} // namespace drast::parser

#endif // DRAST_PARSER_H
