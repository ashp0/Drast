//
// Created by Ashwin Paudel on 2022-03-20.
//

#ifndef DRAST_AST_H
#define DRAST_AST_H

#include "Token.h"
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <unordered_map>
#include <utility>
#include <vector>

enum class ASTType {
    COMPOUND_STATEMENT, // { ... }

    IMPORT, // import io

    FUNCTION_DECLARATION, // int :: test(int a, int b) { ... }
    FUNCTION_ARGUMENT,    // int a, int b
    FUNCTION_CALL,        // test(1, 2)

    TYPE, // int, string, float, bool, etc.

    STRUCT_DECLARATION,      // struct Test { ... }
    STRUCT_INITIALIZER_CALL, // .init(1, 2)

    ENUM_DECLARATION, // enum Test { ... }
    ENUM_CASE,        // case A = 50, B = 100 etc.

    VARIABLE_DECLARATION, // int a = 1

    WHILE, // while (a == 1) { ... }
    FOR,   // for (int i = 0; i < 10; i++) { ... }

    SWITCH,      // switch (a) { case 1: ... }
    SWITCH_CASE, // case 1: ...

    DO,    // do { ... }
    CATCH, // catch (...) { ... }
    TRY,   // try myVariable = myFunction()

    RETURN,    // return 1
    IF,        // if (a == 1) { ... } else { ... }
    ASM,       // asm("mov rax, 1")
    TYPEALIAS, // typealias Test = int

    BINARY_EXPRESSION,   // 5 + 6;
    UNARY_EXPRESSION,    // -5;
    GROUPING_EXPRESSION, // (5 + 6)
    LITERAL_EXPRESSION,  // 5;
    CAST,                // cast(5.50, int);
};

class AST {
  public:
    ASTType ast_type;
    size_t line;

  public:
    constexpr AST(ASTType ast_type, size_t line)
        : ast_type(ast_type), line(line) {}

    virtual ~AST() = default;

    virtual std::string toString() const = 0;

    friend std::ostream &operator<<(std::ostream &out, AST const &ast) {
        out << "AST: " << ast.toString();
        return out;
    }
};

class CompoundStatement : public AST {
  public:
    std::vector<std::unique_ptr<AST>> statements;

  public:
    explicit CompoundStatement(size_t line)
        : AST(ASTType::COMPOUND_STATEMENT, line) {}

    void insertStatement(std::unique_ptr<AST> &statement) {
        statements.push_back(std::move(statement));
    }

    std::string toString() const override;
};

class Import : public AST {
  public:
    std::string import_path;
    bool is_library = false;

  public:
    Import(std::string import_path, size_t line, bool is_library)
        : AST(ASTType::IMPORT, line), import_path(std::move(import_path)),
          is_library(is_library) {}

    std::string toString() const override;
};

class FunctionDeclaration : public AST {
  private:
    using FunctionArgument = class FunctionArgument;

  public:
    std::vector<TokenType> modifiers;
    std::unique_ptr<AST> return_type;
    std::string name;
    std::vector<std::unique_ptr<FunctionArgument>> arguments;
    std::optional<std::unique_ptr<CompoundStatement>> body =
        std::nullopt; // AST CompoundStatement

  public:
    FunctionDeclaration(
        std::vector<TokenType> &modifiers, std::unique_ptr<AST> &return_type,
        std::string &name,
        std::vector<std::unique_ptr<FunctionArgument>> &arguments,
        std::unique_ptr<CompoundStatement> &body, size_t line)
        : AST(ASTType::FUNCTION_DECLARATION, line), modifiers(modifiers),
          return_type(std::move(return_type)), name(std::move(name)),
          arguments(std::move(arguments)), body(std::move(body)){};

    FunctionDeclaration(
        std::vector<TokenType> &modifiers, std::unique_ptr<AST> &return_type,
        std::string &name,
        std::vector<std::unique_ptr<FunctionArgument>> &arguments, size_t line)
        : AST(ASTType::FUNCTION_DECLARATION, line), modifiers(modifiers),
          return_type(std::move(return_type)), name(std::move(name)),
          arguments(std::move(arguments)){};

    std::string toString() const override;
};

class FunctionArgument : public AST {
  public:
    std::optional<std::string> name;
    std::optional<std::unique_ptr<AST>> type;
    bool is_vaarg = false;

  public:
    FunctionArgument(std::string name, std::unique_ptr<AST> &type, size_t line)
        : AST(ASTType::FUNCTION_ARGUMENT, line), name(name),
          type(std::move(type)){};

    FunctionArgument(size_t line)
        : AST(ASTType::FUNCTION_ARGUMENT, line), is_vaarg(true),
          name(std::nullopt), type(std::nullopt){};

    std::string toString() const override;
};

class FunctionCall : public AST {
  public:
    std::string name;
    std::vector<std::unique_ptr<AST>> arguments;

  public:
    FunctionCall(std::string &name,
                 std::vector<std::unique_ptr<AST>> &arguments, size_t line)
        : AST(ASTType::FUNCTION_CALL, line), name(std::move(name)),
          arguments(std::move(arguments)) {}

    std::string toString() const override;
};

class Type : public AST {
  public:
    Token token;
    bool is_pointer;
    bool is_array;
    bool is_optional;

  public:
    Type(Token token, bool is_pointer, bool is_array, bool is_optional,
         size_t line)
        : AST(ASTType::TYPE, line), token(std::move(token)),
          is_pointer(is_pointer), is_array(is_array),
          is_optional(is_optional){};

    std::string toString() const override;
};

class StructDeclaration : public AST {
  public:
    std::string &name;
    std::vector<std::unique_ptr<AST>>
        &fields; // Variable Or Function Declarations

  public:
    StructDeclaration(std::string &name,
                      std::vector<std::unique_ptr<AST>> &fields, size_t line)
        : AST(ASTType::STRUCT_DECLARATION, line), name(name), fields(fields) {}

    std::string toString() const override;
};

class StructInitializerCall : public AST {
  public:
    std::string &name;
    std::vector<std::unique_ptr<AST>> &arguments;

  public:
    StructInitializerCall(std::string &name,
                          std::vector<std::unique_ptr<AST>> &arguments,
                          size_t line)
        : AST(ASTType::STRUCT_INITIALIZER_CALL, line), name(name),
          arguments(arguments) {}

    std::string toString() const override;
};

class EnumDeclaration : public AST {
  private:
    using EnumCase = class EnumCase;

  public:
    std::string name;
    std::vector<std::unique_ptr<EnumCase>> cases;

  public:
    EnumDeclaration(std::string &name,
                    std::vector<std::unique_ptr<EnumCase>> &cases, size_t line)
        : AST(ASTType::ENUM_DECLARATION, line), name(std::move(name)),
          cases(std::move(cases)) {}

    std::string toString() const override;
};

class EnumCase : public AST {
  public:
    std::string name;
    std::unique_ptr<AST> value;

  public:
    EnumCase(std::string &name, std::unique_ptr<AST> &value, size_t line)
        : AST(ASTType::ENUM_CASE, line), name(std::move(name)),
          value(std::move(value)) {}

    std::string toString() const override;
};

class VariableDeclaration : public AST {
  public:
    std::vector<TokenType> modifiers;
    std::string name;
    std::unique_ptr<AST> type;
    std::optional<std::unique_ptr<AST>> value;

  public:
    VariableDeclaration(std::vector<TokenType> &modifiers, std::string &name,
                        std::unique_ptr<AST> &type, size_t line,
                        std::optional<std::unique_ptr<AST>> &value)
        : AST(ASTType::VARIABLE_DECLARATION, line), modifiers(modifiers),
          name(name), type(std::move(type)), value(std::move(value)){};

    std::string toString() const override;
};

class Return : public AST {
  public:
    std::optional<std::unique_ptr<AST>> expression;

  public:
    Return(std::optional<std::unique_ptr<AST>> &expression, size_t line)
        : AST(ASTType::RETURN, line), expression(std::move(expression)){};

    std::string toString() const override;
};

class ASM : public AST {
  public:
    std::vector<std::string> instructions;

  public:
    ASM(std::vector<std::string> &instructions, size_t line)
        : AST(ASTType::ASM, line), instructions(std::move(instructions)){};

    std::string toString() const override;
};

class BinaryExpression : public AST {
  public:
    std::unique_ptr<AST> left;
    std::unique_ptr<AST> right;
    TokenType op;

  public:
    BinaryExpression(std::unique_ptr<AST> &left, std::unique_ptr<AST> &right,
                     TokenType op, size_t line)
        : AST(ASTType::BINARY_EXPRESSION, line), left(std::move(left)),
          right(std::move(right)), op(op){};

    std::string toString() const override;
};

class UnaryExpression : public AST {
  public:
    std::unique_ptr<AST> expr;
    TokenType op;

  public:
    UnaryExpression(std::unique_ptr<AST> &expr, TokenType op, size_t line)
        : AST(ASTType::UNARY_EXPRESSION, line), expr(std::move(expr)), op(op){};

    std::string toString() const override;
};

class LiteralExpression : public AST {
  public:
    std::unique_ptr<Token> token;

  public:
    LiteralExpression(std::unique_ptr<Token> &token, size_t line)
        : AST(ASTType::LITERAL_EXPRESSION, line), token(std::move(token)){};

    std::string toString() const override;
};

class GroupingExpression : public AST {
  public:
    std::unique_ptr<AST> expr;

  public:
    GroupingExpression(std::unique_ptr<AST> &expr, size_t line)
        : AST(ASTType::GROUPING_EXPRESSION, line), expr(std::move(expr)){};

    std::string toString() const override;
};

#endif // DRAST_AST_H