//
// Created by Ashwin Paudel on 2022-03-26.
//

#ifndef DRAST_AST_H
#define DRAST_AST_H

#include "Token.h"
#include "Types.h"
#include <string>
#include <vector>

enum class ASTType {
    COMPOUND, // { ... }

    IMPORT_STATEMENT, // import io

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
    GOTO,      // goto label
    TYPEALIAS, // typealias Test = int

    BINARY_EXPRESSION,   // 5 + 6;
    UNARY_EXPRESSION,    // -5;
    GROUPING_EXPRESSION, // (5 + 6)
    LITERAL_EXPRESSION,  // 5;
    CAST,                // cast(5.50, int);
};

class AST {
  public:
    Location &location;
    ASTType type;

  public:
    AST(ASTType type, Location &location) : location(location), type(type) {}
    virtual ~AST() = default;
    virtual std::string toString() = 0;
};

class CompoundStatement : public AST {
  public:
    std::vector<AST *> statements;

  public:
    CompoundStatement(std::vector<AST *> &statements, Location &location)
        : AST(ASTType::COMPOUND, location), statements(std::move(statements)) {}

    CompoundStatement(Location &location) : AST(ASTType::COMPOUND, location) {}

    std::string toString() override;
};

class ImportStatement : public AST {
  public:
    std::string_view module_name;
    bool is_library;

  public:
    ImportStatement(std::string_view module_name, bool is_library,
                    Location location)
        : AST(ASTType::IMPORT_STATEMENT, location), module_name(module_name),
          is_library(is_library) {}

    std::string toString() override;
};

class FunctionDeclaration : public AST {
  private:
    using FunctionArgument = class FunctionArgument;

  public:
    std::vector<TokenType> qualifiers;
    AST *return_type;
    std::string_view name;
    std::vector<FunctionArgument *> arguments;
    std::optional<CompoundStatement *> body = std::nullopt;

  public:
    FunctionDeclaration(std::vector<TokenType> qualifiers, AST *return_type,
                        std::string_view name,
                        std::vector<FunctionArgument *> arguments,
                        CompoundStatement *body, Location location)
        : AST(ASTType::FUNCTION_DECLARATION, location),
          qualifiers(std::move(qualifiers)), return_type(return_type),
          name(name), arguments(std::move(arguments)), body(body) {}

    FunctionDeclaration(std::vector<TokenType> qualifiers, AST *return_type,
                        std::string_view name,
                        std::vector<FunctionArgument *> arguments,
                        Location location)
        : AST(ASTType::FUNCTION_DECLARATION, location),
          qualifiers(std::move(qualifiers)), return_type(return_type),
          name(name), arguments(std::move(arguments)) {}

    std::string toString() override;
};

class FunctionArgument : public AST {
  public:
    std::optional<std::string_view> name = std::nullopt;
    std::optional<AST *> type = std::nullopt;
    bool is_vaarg = false;

  public:
    FunctionArgument(std::string_view name, std::optional<AST *> type,
                     Location location)
        : AST(ASTType::FUNCTION_ARGUMENT, location), name(name),
          type(std::move(type)) {}

    FunctionArgument(Location location)
        : AST(ASTType::FUNCTION_ARGUMENT, location) {}

    std::string toString() override;
};

class FunctionCall : public AST {
  public:
    std::string_view name;
    std::vector<AST *> arguments;

  public:
    FunctionCall(std::string_view name, std::vector<AST *> arguments,
                 Location location)
        : AST(ASTType::FUNCTION_CALL, location), name(name),
          arguments(arguments) {}

    std::string toString() override;
};

class Type : public AST {
  public:
    std::string_view literal_value;
    TokenType type;
    bool is_pointer;
    bool is_array;
    bool is_optional;

  public:
    Type(TokenType type, std::string_view literal_value, bool is_pointer,
         bool is_array, bool is_optional, Location location)
        : AST(ASTType::TYPE, location), type(type),
          literal_value(literal_value), is_pointer(is_pointer),
          is_array(is_array), is_optional(is_optional) {}

    std::string toString() override;
};

class StructDeclaration : public AST {
  public:
    std::string_view name;
    CompoundStatement *body;
    std::vector<TokenType> qualifiers;

  public:
    StructDeclaration(std::string_view name, std::vector<TokenType> qualifiers,
                      CompoundStatement *body, Location location)
        : AST(ASTType::STRUCT_DECLARATION, location), name(name), body(body),
          qualifiers(qualifiers) {}

    std::string toString() override;
};

class EnumDeclaration : public AST {
  private:
    using EnumCase = class EnumCase;

  public:
    std::string_view name;
    std::vector<EnumCase *> cases;
    std::vector<TokenType> qualifiers;

  public:
    EnumDeclaration(std::string_view name, std::vector<EnumCase *> cases,
                    std::vector<TokenType> qualifiers, Location location)
        : AST(ASTType::ENUM_DECLARATION, location), name(name), cases(cases),
          qualifiers(qualifiers) {}

    std::string toString() override;
};

class EnumCase : public AST {
  public:
    std::string_view name;
    AST *value;

  public:
    EnumCase(std::string_view name, AST *value, Location location)
        : AST(ASTType::ENUM_CASE, location), name(name), value(value){};

    std::string toString() override;
};

class VariableDeclaration : public AST {
    std::string_view name;
    AST *type;
    std::optional<AST *> value = std::nullopt;
    std::vector<TokenType> qualifiers;

  public:
    VariableDeclaration(std::string_view name, AST *type,
                        std::optional<AST *> value,
                        std::vector<TokenType> qualifiers, Location location)
        : AST(ASTType::VARIABLE_DECLARATION, location), name(name), type(type),
          value(value), qualifiers(qualifiers) {}

    std::string toString() override;
};

class ForLoop : public AST {
  public:
    AST *first_statement;
    AST *second_statement;
    AST *third_statement;

    CompoundStatement *body;

  public:
    ForLoop(AST *first_statement, AST *second_statement, AST *third_statement,
            CompoundStatement *body, Location location)
        : AST(ASTType::FOR, location), first_statement(first_statement),
          second_statement(second_statement), third_statement(third_statement),
          body(body) {}

    std::string toString() override;
};

class Return : public AST {
  public:
    std::optional<AST *> expression;

  public:
    Return(std::optional<AST *> expression, Location location)
        : AST(ASTType::RETURN, location), expression(std::move(expression)){};

    std::string toString() override;
};

class If : public AST {
  public:
    AST *if_condition;
    CompoundStatement *if_body;

    std::vector<AST *> else_if_conditions = {};
    std::vector<CompoundStatement *> else_if_bodies = {};

    std::optional<CompoundStatement *> else_body = std::nullopt;

  public:
    If(AST *if_condition, CompoundStatement *if_body,
       std::vector<AST *> else_if_conditions,
       std::vector<CompoundStatement *> else_if_bodies,
       std::optional<CompoundStatement *> else_body, Location location)
        : AST(ASTType::IF, location), if_condition(std::move(if_condition)),
          if_body(std::move(if_body)),
          else_if_conditions(std::move(else_if_conditions)),
          else_if_bodies(std::move(else_if_bodies)),
          else_body(std::move(else_body)){};

    std::string toString() override;
};

class ASM : public AST {
  public:
    std::vector<std::string_view> instructions;

  public:
    ASM(std::vector<std::string_view> instructions, Location location)
        : AST(ASTType::ASM, location), instructions(instructions){};

    std::string toString() override;
};

class GOTO : public AST {
  public:
    std::string_view label;

  public:
    GOTO(std::string_view label, Location location)
        : AST(ASTType::GOTO, location), label(label){};

    std::string toString() override;
};

class BinaryExpression : public AST {
  public:
    AST *left;
    AST *right;
    TokenType op;

  public:
    BinaryExpression(AST *left, AST *right, TokenType op, Location location)
        : AST(ASTType::BINARY_EXPRESSION, location), left(left), right(right),
          op(op){};

    std::string toString() override;
};

class UnaryExpression : public AST {
  public:
    AST *expr;
    TokenType op;

  public:
    UnaryExpression(AST *expr, TokenType op, Location location)
        : AST(ASTType::UNARY_EXPRESSION, location), expr(expr), op(op){};

    std::string toString() override;
};

class LiteralExpression : public AST {
  public:
    std::string_view value;
    std::optional<std::string> string_value = std::nullopt;
    TokenType literal_type;

  public:
    LiteralExpression(std::string_view value, TokenType literal_type,
                      Location location)
        : AST(ASTType::LITERAL_EXPRESSION, location), value(value),
          literal_type(literal_type){};

    LiteralExpression(std::string string_value, TokenType literal_type,
                      Location location)
        : AST(ASTType::LITERAL_EXPRESSION, location),
          string_value(string_value), literal_type(literal_type){};

    std::string toString() override;
};

class GroupingExpression : public AST {
  public:
    AST *expr;

  public:
    GroupingExpression(AST *&expr, Location location)
        : AST(ASTType::GROUPING_EXPRESSION, location), expr(expr){};

    std::string toString() override;
};

#endif // DRAST_AST_H
