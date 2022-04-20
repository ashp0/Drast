//
// Created by Ashwin Paudel on 2022-03-26.
//

#ifndef DRAST_PARSER_H
#define DRAST_PARSER_H

#include "../AST/AST.h"
#include "../Lexer/Lexer.h"
#include "../Lexer/Token.h"
#include "../Lexer/TokenUtils.h"
#include <map>
#include <utility>
#include <vector>

namespace drast::parser {

class Parser {
  private:
    lexer::Lexer lexer;
    Error error;

    size_t index = 0;

    bool parses_goto_labels = true;
    bool inside_function_body = false; // myFunction(!{return true})
    bool is_parsing_struct = false;
    bool should_check_duplicates = true;
    AST::CompoundStatement *current_compound = nullptr;

  public:
    Parser(std::string &file_name, lexer::Lexer &lexer, Error error)
        : lexer(lexer), error(std::move(error)) {}

    AST::CompoundStatement *parse();

  private: /* Parser functions */
    AST::CompoundStatement *compound();

    AST::Node *statement();

    AST::ImportStatement *import();

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

    std::pair<AST::Node *, AST::CompoundStatement *> ifElseStatements();

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

  private: /* Utilities */
    void advance();

    void reverse();

    void advance(lexer::TokenType type, const char *error_message = "");

    bool advanceIf(lexer::TokenType type);

    void advanceLines();

    lexer::TokenType getAndAdvanceType();

    std::string_view getAndAdvance();

    std::string_view getAndAdvance(lexer::TokenType type,
                                   const char *message = "");

    lexer::Token &current() { return lexer.tokens[index]; }

    lexer::Token &peek() { return lexer.tokens[index + 1]; }

    template <class ast_type, class... Args>
    ast_type *makeDeclaration(Args &&...args);

    int throwError(const char *message);

    int throwError(const char *message, Location location);
};

} // namespace drast::parser

#endif // DRAST_PARSER_H
