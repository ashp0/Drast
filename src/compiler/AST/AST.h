//
// AST.h
// Created by Ashwin Paudel on 2022-03-26.
//
// =============================================================================
///
/// \file
/// This file contains the declaration of the AST class. Which is used by the
/// parser and semantic analyzer to generate and analyze the AST.
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

#ifndef DRAST_AST_H
#define DRAST_AST_H

#include "../Common/LookupTable.h"
#include "../Common/Types.h"
#include "../Lexer/Token.h"
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace drast::AST {

class SemaTypes {
  private:
    using AST = class Node;

  public:
    enum Type {
        INT,
        FLOAT,
        BOOL,
        STRING,
        NIL,
        TYPE, // a user defined struct
    } type;

    std::string_view type_literal; // if it's a user defined type
    AST *type_node;                // the original node that represents the type

    SemaTypes(std::string_view type_literal, AST *type_node)
        : type_literal(type_literal), type(TYPE), type_node(type_node) {}

    SemaTypes(Type type, AST *type_node) : type(type), type_node(type_node) {}

    SemaTypes(Type type) : type(type) {}
};

constexpr SemaTypes::Type TokenTypeToSemaType(lexer::TokenType token) {
    switch (token) {
    case lexer::TokenType::V_INT:
        return SemaTypes::INT;

    case lexer::TokenType::V_FLOAT:
        return SemaTypes::FLOAT;

    case lexer::TokenType::TRUE:
    case lexer::TokenType::FALSE:
        return SemaTypes::BOOL;

    case lexer::TokenType::V_STRING:
        return SemaTypes::STRING;

    case lexer::TokenType::NIL:
        return SemaTypes::NIL;

    default:
        return SemaTypes::TYPE;
    }
}

enum class ASTType {
    COMPOUND, // { ... }

    IMPORT_STATEMENT, // import io

    FUNCTION_DECLARATION,              // int :: test(int a, int b) { ... }
    FUNCTION_ARGUMENT,                 // int a, int b
    FUNCTION_CALL,                     // test(1, 2)
    FUNCTION_CALL_NAME_BASED_ARGUMENT, // multiply(second_number: 50,
                                       // first_number: 30)
    FIRST_CLASS_FUNCTION,              //  $bool(int, int)
    OPERATOR_OVERLOAD, // int :: operator[](float offset) { ... }

    TYPE, // int, string, float, bool, etc.

    STRUCT_DECLARATION,             // struct Test { ... }
    STRUCT_MEMBER_ACCESS,           // test.integer
    STRUCT_INITIALIZER_CALL,        // .init(1, 2)
    STRUCT_INITIALIZER_DECLARATION, // @(int test, string test1) { ... }

    ENUM_DECLARATION, // enum Test { ... }
    ENUM_CASE,        // case A = 50, B = 100 etc.
    ENUM_CASE_ACCESS, // .red

    VARIABLE_DECLARATION, // int a = 1

    WHILE,           // while (a == 1) { ... }
    FOR,             // for (int i = 0; i < 10; i++) { ... }
    RANGE_BASED_FOR, // for (dog in dogs) { ... }

    SWITCH,      // switch (a) { case 1: ... }
    SWITCH_CASE, // case 1: ...

    DO_CATCH, // do { ... } catch (...) { ... }
    TRY,      // try myVariable = myFunction()

    RETURN,    // return 1
    IF,        // if (a == 1) { ... } else { ... }
    ASM,       // asm("mov rax, 1")
    GOTO,      // goto label
    TYPEALIAS, // typealias Test = int

    BINARY_EXPRESSION,   // 5 + 6;
    UNARY_EXPRESSION,    // -5;
    GROUPING_EXPRESSION, // (5 + 6)
    LITERAL_EXPRESSION,  // 5;
    TERNARY_EXPRESSION,  // ? 50 : 20
    ARRAY_ACCESS,        // myVariable[]
    ARRAY_CREATION,      // [50, 30+20]
    CAST,                // cast(5.50, int);
    OPTIONAL_UNWRAP,     // myOptionalFloat ?? 3.14
    FORCE_UNWRAP,        // myOptionalFloat!
    TOKEN,               // break, continue etc...
};

class Node {
  public:
    Location location;
    ASTType type;

  protected:
    Node(ASTType type, Location &location) : location(location), type(type) {}
    virtual ~Node() = default;

  public:
    virtual std::string toString() = 0;
};

class Compound : public Node {
  private:
    using FirstClassFunction = class FirstClassFunction;
    using VariableDeclaration = class VariableDeclaration;
    using FunctionDeclaration = class FunctionDeclaration;

  public:
    std::vector<Node *> statements;
    std::optional<FirstClassFunction *> first_class_function;
    LookupTable<std::string_view, VariableDeclaration *> variables = {};
    LookupTable<std::string_view, FunctionDeclaration *> function_declarations =
        {};

  public:
    Compound(std::vector<Node *> &statements, Location &location)
        : Node(ASTType::COMPOUND, location), statements(std::move(statements)) {
    }

    explicit Compound(Location &location) : Node(ASTType::COMPOUND, location) {}

    std::string toString() override;

    std::optional<std::pair<std::string_view, VariableDeclaration *>>
    searchForDuplicateVariables();

    std::optional<VariableDeclaration *> findVariable(std::string_view name);

    bool checkFunctions();
};

class Import : public Node {
  public:
    std::string_view module_name;
    bool is_library;

  public:
    Import(std::string_view module_name, bool is_library, Location location)
        : Node(ASTType::IMPORT_STATEMENT, location), module_name(module_name),
          is_library(is_library) {}

    std::string toString() override;
};

class FunctionDeclaration : public Node {
  private:
    using FunctionArgument = class FunctionArgument;
    using TemplateDeclaration = class TemplateDeclaration;

  public:
    std::vector<lexer::TokenType> qualifiers;
    std::optional<Node *> return_type;
    std::string_view name;
    std::vector<FunctionArgument *> arguments;
    std::optional<Compound *> body = std::nullopt;
    std::optional<TemplateDeclaration *> template_declaration = std::nullopt;

  public:
    FunctionDeclaration(
        std::vector<lexer::TokenType> qualifiers,
        std::optional<Node *> return_type, std::string_view name,
        std::vector<FunctionArgument *> arguments, Compound *body,
        std::optional<TemplateDeclaration *> template_declaration,
        Location location)
        : Node(ASTType::FUNCTION_DECLARATION, location),
          qualifiers(std::move(qualifiers)), return_type(return_type),
          name(name), arguments(std::move(arguments)), body(body),
          template_declaration(template_declaration) {}

    FunctionDeclaration(std::vector<lexer::TokenType> qualifiers,
                        std::optional<Node *> return_type,
                        std::string_view name,
                        std::vector<FunctionArgument *> arguments,
                        Location location)
        : Node(ASTType::FUNCTION_DECLARATION, location),
          qualifiers(std::move(qualifiers)), return_type(return_type),
          name(name), arguments(std::move(arguments)) {}

    std::string toString() override;
};

class FunctionArgument : public Node {
  public:
    std::optional<std::string_view> name = std::nullopt;
    std::optional<Node *> argument_default_value = std::nullopt;
    std::optional<Node *> type = std::nullopt;
    bool is_vaarg = false;
    bool is_constant = false;

  public:
    FunctionArgument(std::optional<std::string_view> name,
                     std::optional<Node *> type,
                     std::optional<Node *> argument_default_value,
                     bool is_constant, Location location)
        : Node(ASTType::FUNCTION_ARGUMENT, location), name(name), type(type),
          argument_default_value(argument_default_value),
          is_constant(is_constant) {}

    explicit FunctionArgument(Location location)
        : Node(ASTType::FUNCTION_ARGUMENT, location) {}

    std::string toString() override;
};

class FunctionCall : public Node {
  public:
    std::string_view name;
    std::vector<Node *> arguments;
    std::optional<std::vector<Node *>> template_arguments = std::nullopt;

  public:
    FunctionCall(std::string_view name, std::vector<Node *> arguments,
                 std::optional<std::vector<Node *>> template_arguments,
                 Location location)
        : Node(ASTType::FUNCTION_CALL, location), name(name),
          arguments(std::move(arguments)),
          template_arguments(std::move(template_arguments)) {}

    std::string toString() override;
};

class FunctionArgumentName : public Node {
  public:
    std::string_view name;
    Node *expression;

  public:
    FunctionArgumentName(std::string_view name, Node *expression,
                         Location location)
        : Node(ASTType::FUNCTION_CALL_NAME_BASED_ARGUMENT, location),
          name(name), expression(expression) {}

    std::string toString() override;
};

class Type : public Node {
  public:
    std::string_view literal_value;
    lexer::TokenType type_;
    bool is_pointer;
    bool is_array;
    bool is_optional;
    bool is_throw_statement;
    std::optional<std::vector<Node *>> template_values = std::nullopt;

  public:
    Type(lexer::TokenType type_, std::string_view literal_value,
         bool is_pointer, bool is_array, bool is_optional,
         bool is_throw_statement, Location location)
        : Node(ASTType::TYPE, location), type_(type_),
          literal_value(literal_value), is_pointer(is_pointer),
          is_array(is_array), is_optional(is_optional),
          is_throw_statement(is_throw_statement) {}

    std::string toString() override;
};

class StructDeclaration : public Node {
  private:
    using TemplateDeclaration = class TemplateDeclaration;

  public:
    std::string_view name;
    Compound *body;
    std::vector<lexer::TokenType> qualifiers;
    std::optional<TemplateDeclaration *> template_declaration;
    bool is_union = false;

  public:
    StructDeclaration(std::string_view name,
                      std::vector<lexer::TokenType> qualifiers, Compound *body,
                      std::optional<TemplateDeclaration *> template_declaration,
                      bool is_union, Location location)
        : Node(ASTType::STRUCT_DECLARATION, location), name(name), body(body),
          qualifiers(std::move(qualifiers)),
          template_declaration(template_declaration), is_union(is_union) {}

    std::string toString() override;
};

class StructMemberAccess : public Node {
  public:
    // my_struct.my_variable
    std::string_view struct_variable; // my_struct
    Node *struct_member;              // my_variable
    std::optional<FunctionCall *> function_call;

  public:
    StructMemberAccess(std::string_view struct_variable, Node *struct_member,
                       Location location)
        : Node(ASTType::STRUCT_MEMBER_ACCESS, location),
          struct_variable(struct_variable), struct_member(struct_member) {}

    StructMemberAccess(std::optional<FunctionCall *> function_call,
                       Node *struct_member, Location location)
        : Node(ASTType::STRUCT_MEMBER_ACCESS, location),
          function_call(function_call), struct_member(struct_member) {}

    std::string toString() override;
};

class StructInitializerCall : public Node {
  public:
    std::optional<std::vector<Node *>> arguments;
    bool is_deinit = false;
    std::optional<std::string_view> variable_name = std::nullopt;

  public:
    StructInitializerCall(std::vector<Node *> arguments, Location location)
        : Node(ASTType::STRUCT_INITIALIZER_CALL, location),
          arguments(std::move(arguments)) {}

    StructInitializerCall(std::string_view variable_name, bool is_deinit,
                          Location location)
        : Node(ASTType::STRUCT_INITIALIZER_CALL, location),
          variable_name(variable_name), is_deinit(is_deinit) {}

    StructInitializerCall(std::string_view variable_name,
                          std::vector<Node *> arguments, Location location)
        : Node(ASTType::STRUCT_INITIALIZER_CALL, location),
          variable_name(variable_name), arguments(std::move(arguments)) {}

    std::string toString() override;
};

class StructInitializerDeclaration : public Node {
  public:
    std::optional<std::vector<FunctionArgument *>> arguments = std::nullopt;
    Compound *body;

  public:
    StructInitializerDeclaration(std::vector<FunctionArgument *> arguments,
                                 Compound *body, Location location)
        : Node(ASTType::STRUCT_INITIALIZER_DECLARATION, location),
          arguments(std::move(arguments)), body(body) {}

    StructInitializerDeclaration(Compound *body, Location location)
        : Node(ASTType::STRUCT_INITIALIZER_DECLARATION, location), body(body) {}

    std::string toString() override;
};

class EnumDeclaration : public Node {
  private:
    using EnumCase = class EnumCase;

  public:
    std::string_view name;
    std::vector<EnumCase *> cases;
    std::vector<lexer::TokenType> qualifiers;

  public:
    EnumDeclaration(std::string_view name, std::vector<EnumCase *> cases,
                    std::vector<lexer::TokenType> qualifiers, Location location)
        : Node(ASTType::ENUM_DECLARATION, location), name(name),
          cases(std::move(cases)), qualifiers(std::move(qualifiers)) {}

    std::string toString() override;
};

class EnumCase : public Node {
  public:
    std::string_view name;
    Node *value;

  public:
    EnumCase(std::string_view name, Node *value, Location location)
        : Node(ASTType::ENUM_CASE, location), name(name), value(value){};

    std::string toString() override;
};

class EnumCaseAccess : public Node {
  public:
    std::string_view case_name;

  public:
    EnumCaseAccess(std::string_view case_name, Location location)
        : Node(ASTType::ENUM_CASE_ACCESS, location), case_name(case_name) {}

    std::string toString() override;
};

class VariableDeclaration : public Node {
  public:
    std::string_view name;
    std::optional<Node *> type;
    std::optional<Node *> value = std::nullopt;
    std::vector<lexer::TokenType> qualifiers;
    bool is_let;

    // the semantic analyzer will give a variable a type.
    // via type inferring
    std::optional<SemaTypes> sema_type = std::nullopt;

  public:
    VariableDeclaration(std::string_view name, std::optional<Node *> type,
                        std::optional<Node *> value,
                        std::vector<lexer::TokenType> qualifiers, bool is_let,
                        Location location)
        : Node(ASTType::VARIABLE_DECLARATION, location), name(name), type(type),
          value(value), qualifiers(std::move(qualifiers)), is_let(is_let) {}

    std::string toString() override;
};

class ForLoop : public Node {
  public:
    Node *first_statement;
    Node *second_statement;
    Node *third_statement;

    Compound *body;

  public:
    ForLoop(Node *first_statement, Node *second_statement,
            Node *third_statement, Compound *body, Location location)
        : Node(ASTType::FOR, location), first_statement(first_statement),
          second_statement(second_statement), third_statement(third_statement),
          body(body) {}

    std::string toString() override;
};

class RangeBasedForLoop : public Node {
  public:
    std::string_view name;
    Node *name2;
    std::optional<Node *> for_index;

    Compound *body;

  public:
    RangeBasedForLoop(std::string_view name, Node *name2,
                      std::optional<Node *> for_index, Compound *body,
                      Location location)
        : Node(ASTType::RANGE_BASED_FOR, location), name(name), name2(name2),
          for_index(for_index), body(body) {}

    std::string toString() override;
};

class WhileLoop : public Node {
  public:
    Node *expression;
    Compound *body;

  public:
    WhileLoop(Node *expression, Compound *body, Location location)
        : Node(ASTType::WHILE, location), expression(expression), body(body) {}

    std::string toString() override;
};

class Return : public Node {
  public:
    std::optional<Node *> expression;
    bool is_throw_statement;

  public:
    Return(std::optional<Node *> expression, bool is_throw_statement,
           Location location)
        : Node(ASTType::RETURN, location), expression(expression),
          is_throw_statement(is_throw_statement){};

    std::string toString() override;
};

class If : public Node {
  public:
    Node *if_condition;
    Compound *if_body;

    std::vector<Node *> else_if_conditions = {};
    std::vector<Compound *> else_if_bodies = {};

    std::optional<Compound *> else_body = std::nullopt;

  public:
    If(Node *if_condition, Compound *if_body,
       std::vector<Node *> else_if_conditions,
       std::vector<Compound *> else_if_bodies,
       std::optional<Compound *> else_body, Location location)
        : Node(ASTType::IF, location), if_condition(if_condition),
          if_body(if_body), else_if_conditions(std::move(else_if_conditions)),
          else_if_bodies(std::move(else_if_bodies)), else_body(else_body){};

    std::string toString() override;
};

class ASM : public Node {
  public:
    std::vector<std::string_view> instructions;

  public:
    ASM(std::vector<std::string_view> instructions, Location location)
        : Node(ASTType::ASM, location), instructions(std::move(instructions)){};

    std::string toString() override;
};

class GOTO : public Node {
  public:
    std::string_view label;
    bool is_goto_token; // if it's a goto token or a label being defined

  public:
    GOTO(std::string_view label, bool is_goto_token, Location location)
        : Node(ASTType::GOTO, location), label(label),
          is_goto_token(is_goto_token){};

    std::string toString() override;
};

class SwitchStatement : public Node {
  private:
    using SwitchCase = class SwitchCase;

  public:
    Node *expression; // switch (...)
    std::vector<SwitchCase *> cases;

  public:
    SwitchStatement(Node *expression, std::vector<SwitchCase *> cases,
                    Location location)
        : Node(ASTType::SWITCH, location), expression(expression),
          cases(std::move(cases)) {}

    std::string toString() override;
};

class SwitchCase : public Node {
  public:
    std::optional<Node *> expression = std::nullopt; // case 40
    Compound *body;
    bool is_case; // could be default

  public:
    SwitchCase(Node *expression, Compound *body, bool is_case,
               Location location)
        : Node(ASTType::SWITCH_CASE, location), expression(expression),
          body(body), is_case(is_case) {}

    SwitchCase(Compound *body, bool is_case, Location location)
        : Node(ASTType::SWITCH_CASE, location), body(body), is_case(is_case) {}

    std::string toString() override;
};

class BinaryExpression : public Node {
  public:
    Node *left;
    Node *right;
    lexer::TokenType op;

  public:
    BinaryExpression(Node *left, Node *right, lexer::TokenType op,
                     Location location)
        : Node(ASTType::BINARY_EXPRESSION, location), left(left), right(right),
          op(op){};

    std::string toString() override;
};

class TryExpression : public Node {
  public:
    Node *expr;
    bool is_force_cast;    // try!
    bool is_optional_cast; // try?

  public:
    TryExpression(Node *expr, bool is_force_cast, bool is_optional_cast,
                  Location location)
        : Node(ASTType::TRY, location), expr(expr),
          is_force_cast(is_force_cast), is_optional_cast(is_optional_cast){};

    std::string toString() override;
};

class UnaryExpression : public Node {
  public:
    Node *expr;
    lexer::TokenType op;

  public:
    UnaryExpression(Node *expr, lexer::TokenType op, Location location)
        : Node(ASTType::UNARY_EXPRESSION, location), expr(expr), op(op){};

    std::string toString() override;
};

class TernaryExpression : public Node {
  public:
    Node *bool_expression;
    Node *first_expression;
    Node *second_expression;

  public:
    TernaryExpression(Node *bool_expression, Node *first_expression,
                      Node *second_expression, Location location)
        : Node(ASTType::TERNARY_EXPRESSION, location),
          bool_expression(bool_expression), first_expression(first_expression),
          second_expression(second_expression) {}

    std::string toString() override;
};

class LiteralExpression : public Node {
  public:
    std::string_view value;
    std::optional<std::string> string_value = std::nullopt;
    lexer::TokenType literal_type;

  public:
    LiteralExpression(std::string_view value, lexer::TokenType literal_type,
                      Location location)
        : Node(ASTType::LITERAL_EXPRESSION, location), value(value),
          literal_type(literal_type){};

    LiteralExpression(std::string string_value, lexer::TokenType literal_type,
                      Location location)
        : Node(ASTType::LITERAL_EXPRESSION, location),
          string_value(string_value), literal_type(literal_type){};

    std::string toString() override;
};

class GroupingExpression : public Node {
  public:
    Node *expr;

  public:
    GroupingExpression(Node *&expr, Location location)
        : Node(ASTType::GROUPING_EXPRESSION, location), expr(expr){};

    std::string toString() override;
};

class Typealias : public Node {
  public:
    std::string_view type_name;
    Node *type_value;

  public:
    Typealias(std::string_view type_name, Node *type_value, Location location)
        : Node(ASTType::TYPEALIAS, location), type_name(type_name),
          type_value(type_value) {}

    std::string toString() override;
};

class ASTToken : public Node {
  public:
    lexer::TokenType type;

  public:
    ASTToken(lexer::TokenType type, Location location)
        : Node(ASTType::TOKEN, location), type(type) {}

    std::string toString() override {
        return lexer::tokenTypeAsLiteral(this->type);
    }
};

class TemplateDeclaration : public Node {
  private:
    using StructDeclarationArgument = class TemplateDeclarationArgument;

  public:
    std::vector<StructDeclarationArgument *> arguments;

  public:
    TemplateDeclaration(std::vector<StructDeclarationArgument *> arguments,
                        Location location)
        : Node(ASTType::TOKEN, location), arguments(std::move(arguments)) {}

    std::string toString() override;
};

class TemplateDeclarationArgument : public Node {
  public:
    lexer::TokenType type;
    std::string_view name;

  public:
    TemplateDeclarationArgument(std::string_view name, lexer::TokenType type,
                                Location location)
        : Node(ASTType::TOKEN, location), name(name), type(type) {}

    std::string toString() override { return "elkamfalwkefmalkefmalwk"; }
};

class FirstClassFunction : public Node {
  public:
    std::optional<Node *> return_type;
    std::vector<FunctionArgument *> function_arguments;
    bool is_type = false;

  public:
    FirstClassFunction(std::optional<Node *> return_type,
                       std::vector<FunctionArgument *> function_arguments,
                       bool is_type, Location location)
        : Node(ASTType::FIRST_CLASS_FUNCTION, location),
          return_type(return_type),
          function_arguments(std::move(function_arguments)) {}

    std::string toString() override;
};

class DoCatchStatement : public Node {
  public:
    Compound *do_body;
    Compound *catch_body;
    std::optional<Node *> catch_expression;

  public:
    DoCatchStatement(Compound *do_body, Compound *catch_body,
                     std::optional<Node *> catch_expression, Location location)
        : Node(ASTType::DO_CATCH, location), do_body(do_body),
          catch_body(catch_body), catch_expression(catch_expression){};

    std::string toString() override;
};

class CastExpression : public Node {
  public:
    Node *cast_value;
    Node *cast_type;
    bool is_force_cast = false;
    bool is_optional_cast = false;

  public:
    CastExpression(Node *cast_value, Node *cast_type, bool is_force_cast,
                   bool is_optional_cast, Location location)
        : Node(ASTType::CAST, location), cast_value(cast_value),
          cast_type(cast_type), is_force_cast(is_force_cast),
          is_optional_cast(is_optional_cast) {}

    std::string toString() override;
};

class OperatorOverload : public Node {
  public:
    Node *return_type;
    std::vector<lexer::TokenType> operators;
    std::vector<FunctionArgument *> arguments;
    Compound *body;

  public:
    OperatorOverload(Node *return_type, std::vector<lexer::TokenType> operators,
                     std::vector<FunctionArgument *> arguments, Compound *body,
                     Location location)
        : Node(ASTType::OPERATOR_OVERLOAD, location), return_type(return_type),
          operators(std::move(operators)), arguments(std::move(arguments)),
          body(body) {}

    std::string toString() override;
};

class ArrayAccess : public Node {
  public:
    Node *index;
    std::optional<FunctionCall *> function_call;
    std::string_view variable_name;

  public:
    ArrayAccess(std::string_view variable_name, Node *index, Location location)
        : Node(ASTType::ARRAY_ACCESS, location), variable_name(variable_name),
          index(index) {}

    ArrayAccess(FunctionCall *function_call, Node *index, Location location)
        : Node(ASTType::ARRAY_ACCESS, location), function_call(function_call),
          index(index) {}

    std::string toString() override;
};

class ArrayCreation : public Node {
  public:
    std::vector<Node *> items;

  public:
    ArrayCreation(std::vector<Node *> &items, Location location)
        : Node(ASTType::ARRAY_CREATION, location), items(std::move(items)) {}

    std::string toString() override;
};

class OptionalUnwrapExpression : public Node {
  public:
    Node *first_expression;
    Node *if_nilled_value;

  public:
    OptionalUnwrapExpression(Node *first_expression, Node *if_nilled_value,
                             Location location)
        : Node(ASTType::OPTIONAL_UNWRAP, location),
          first_expression(first_expression), if_nilled_value(if_nilled_value) {
    }

    std::string toString() override;
};

class ForceUnwrapExpression : public Node {
  public:
    Node *expression;

  public:
    ForceUnwrapExpression(Node *expression, Location location)
        : Node(ASTType::FORCE_UNWRAP, location), expression(expression) {}

    std::string toString() override;
};

} // namespace drast::AST

#endif // DRAST_AST_H
