//
// Created by Ashwin Paudel on 2022-03-26.
//

#ifndef DRAST_PARSER_H
#define DRAST_PARSER_H

#include "AST.h"
#include "Lexer.h"
#include "Token.h"
#include <map>
#include <utility>
#include <vector>

class Parser {
  private:
    Lexer lexer;
    Error error;

    size_t index = 0;

    bool parses_goto_labels = true;
    bool inside_function_body = false; // myFunction(!{return true})
    bool is_parsing_struct = false;
    bool should_check_duplicates = true;
    CompoundStatement *current_compound = nullptr;

  public:
    Parser(std::string &file_name, Lexer &lexer, Error error)
        : lexer(lexer), error(std::move(error)) {}

    void parse();

  private: /* Parser functions */
    CompoundStatement *compound();

    AST *statement();

    ImportStatement *import();

    StructDeclaration *
    structDeclaration(const std::vector<TokenType> &qualifiers = {});

    AST *structMemberAccess();

    AST *structMemberAccess(FunctionCall *function_call);

    AST *initOrEnumCase();

    StructInitializerDeclaration *structInitializerDeclaration();

    AST *structDestructorDeclaration();

    EnumDeclaration *
    enumDeclaration(const std::vector<TokenType> &qualifiers = {});

    std::vector<EnumCase *> enumCases();

    AST *functionDeclaration(const std::vector<TokenType> &qualifiers = {});

    AST *variableDeclaration(const std::vector<TokenType> &qualifiers = {},
                             bool is_let = false);

    RangeBasedForLoop *rangeBasedForLoop();

    AST *forLoop();

    WhileLoop *whileLoop();

    Return *returnStatement(bool is_throw_statement);

    If *ifStatement();

    std::pair<AST *, CompoundStatement *> ifElseStatements();

    ASM *inlineAssembly();

    GOTO *gotoStatement();

    SwitchStatement *switchStatement();

    std::vector<SwitchCase *> switchCases();

    SwitchCase *switchCase();

    DoCatchStatement *doCatchStatement();

    OperatorOverload *operatorOverload();

    AST *expression();

    AST *tryExpression();

    AST *castExpression();

    TernaryExpression *ternaryExpression(AST *bool_expression);

    OptionalUnwrapExpression *optionalUnwrap(AST *expression);

    ForceUnwrapExpression *forceUnwrap(AST *expression);

    AST *equality();

    AST *comparison();

    AST *term();

    AST *factor();

    AST *unary();

    AST *primary();

    AST *functionCall(std::string_view function_name,
                      const std::optional<std::vector<AST *>>
                          &template_arguments = std::nullopt);

    AST *arrayAccess();

    AST *arrayAccess(FunctionCall *function_call);

    AST *arrayCreation();

    std::vector<AST *> functionCallArguments();

    AST *typealias();

    AST *token();

    TemplateDeclaration *templateDeclaration();

    std::vector<TemplateDeclarationArgument *> templateArguments();

    std::vector<AST *> templateCallArguments();

    std::vector<FunctionArgument *> functionArguments();

    FirstClassFunction *firstClassFunction();

    AST *type();

    AST *qualifiers();

    std::vector<TokenType> getQualifiers();

  private: /* Utilities */
    void advance();

    void reverse();

    void advance(TokenType type, const char *error_message = "");

    bool advanceIf(TokenType type);

    void advanceLines();

    TokenType getAndAdvanceType();

    std::string_view getAndAdvance();

    std::string_view getAndAdvance(TokenType type, const char *message = "");

    Token &current() { return lexer.tokens[index]; }

    Token &peek() { return lexer.tokens[index + 1]; }

    template <class ast_type, class... Args>
    ast_type *makeDeclaration(Args &&...args);

    int throwError(const char *message);

    int throwError(const char *message, Location location);
};

#endif // DRAST_PARSER_H
