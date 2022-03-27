//
// Created by Ashwin Paudel on 2022-03-26.
//

#ifndef DRAST_PARSER_H
#define DRAST_PARSER_H

#include "AST.h"
#include "Print.h"
#include "Token.h"
#include <vector>

class Parser {
  private:
    std::vector<Token> &tokens;
    uint32_t index;
    Print printer;

  public:
    Parser(std::string &file_name, std::vector<Token> &tokens,
           std::string &source)
        : tokens(tokens), printer(file_name, source), index(0){};

    void parse();

  private: /* Parser functions */
    CompoundStatement *compound();

    AST *statement();

    ImportStatement *import();

    StructDeclaration *
    struct_declaration(const std::vector<TokenType> &qualifiers = {});

    EnumDeclaration *
    enum_declaration(const std::vector<TokenType> &qualifiers = {});

    std::vector<EnumCase *> enum_cases();

    AST *
    function_or_variable_declaration(const std::vector<TokenType> &qualifiers);

    FunctionDeclaration *
    function_declaration(AST *&return_type,
                         const std::vector<TokenType> &qualifiers = {});

    std::vector<FunctionArgument *> function_arguments();

    AST *variable_declaration(AST *&variable_type,
                              const std::vector<TokenType> &qualifiers = {});

    ForLoop *for_loop();

    WhileLoop *while_loop();

    Return *return_statement();

    If *if_statement();

    std::pair<AST *, CompoundStatement *> if_else_statements();

    ASM *inline_assembly();

    GOTO *goto_statement();

    SwitchStatement *switch_statement();

    std::vector<SwitchCase *> switch_cases();

    SwitchCase *switch_case();

    AST *expression();

    AST *equality();

    AST *comparison();

    AST *term();

    AST *factor();

    AST *unary();

    AST *primary(bool parses_goto = true);

    AST *function_call(std::string_view function_name);

    AST *token();

    AST *type();

    AST *qualifiers();

    std::vector<TokenType> getQualifiers();

  private: /* Utilities */
    [[nodiscard]] Token &current() const { return this->tokens[this->index]; }

    void advance();

    void advance(TokenType type);

    bool advanceIf(TokenType type);

    Token *getAndAdvance();

    Token *getAndAdvance(TokenType type);

    Token &peek(int offset = 1);

    template <class ast_type, class... Args>
    ast_type *create_declaration(Args &&...args);

    static int throw_error(const char *message);
};

#endif // DRAST_PARSER_H
