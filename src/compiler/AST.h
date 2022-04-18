//
// Created by Ashwin Paudel on 2022-03-26.
//

#ifndef DRAST_AST_H
#define DRAST_AST_H

#include "Token.h"
#include "Types.h"
#include <map>
#include <string>
#include <utility>
#include <vector>

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

class AST {
  public:
    Location &location;
    ASTType type;

  protected:
    AST(ASTType type, Location &location) : location(location), type(type) {}
    virtual ~AST() = default;

  public:
    virtual std::string toString() = 0;
};

class CompoundStatement : public AST {
  private:
    using FirstClassFunction = class FirstClassFunction;

  public:
    std::vector<AST *> statements;
    std::optional<FirstClassFunction *> first_class_function;
    std::vector<std::pair<std::string_view, Location>> declaration_names;

  public:
    CompoundStatement(std::vector<AST *> &statements, Location &location)
        : AST(ASTType::COMPOUND, location), statements(std::move(statements)) {}

    explicit CompoundStatement(Location &location)
        : AST(ASTType::COMPOUND, location) {}

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
    using TemplateDeclaration = class TemplateDeclaration;

  public:
    std::vector<TokenType> qualifiers;
    std::optional<AST *> return_type;
    std::string_view name;
    std::vector<FunctionArgument *> arguments;
    std::optional<CompoundStatement *> body = std::nullopt;
    std::optional<TemplateDeclaration *> template_declaration = std::nullopt;

  public:
    FunctionDeclaration(
        std::vector<TokenType> qualifiers, std::optional<AST *> return_type,
        std::string_view name, std::vector<FunctionArgument *> arguments,
        CompoundStatement *body,
        std::optional<TemplateDeclaration *> template_declaration,
        Location location)
        : AST(ASTType::FUNCTION_DECLARATION, location),
          qualifiers(std::move(qualifiers)), return_type(return_type),
          name(name), arguments(std::move(arguments)), body(body),
          template_declaration(template_declaration) {}

    FunctionDeclaration(std::vector<TokenType> qualifiers,
                        std::optional<AST *> return_type, std::string_view name,
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
    std::optional<AST *> argument_default_value = std::nullopt;
    std::optional<AST *> type = std::nullopt;
    bool is_vaarg = false;
    bool is_constant = false;

  public:
    FunctionArgument(std::optional<std::string_view> name,
                     std::optional<AST *> type,
                     std::optional<AST *> argument_default_value,
                     bool is_constant, Location location)
        : AST(ASTType::FUNCTION_ARGUMENT, location), name(name), type(type),
          argument_default_value(argument_default_value),
          is_constant(is_constant) {}

    explicit FunctionArgument(Location location)
        : AST(ASTType::FUNCTION_ARGUMENT, location) {}

    std::string toString() override;
};

class FunctionCall : public AST {
  public:
    std::string_view name;
    std::vector<AST *> arguments;
    std::optional<std::vector<AST *>> template_arguments = std::nullopt;

  public:
    FunctionCall(std::string_view name, std::vector<AST *> arguments,
                 std::optional<std::vector<AST *>> template_arguments,
                 Location location)
        : AST(ASTType::FUNCTION_CALL, location), name(name),
          arguments(std::move(arguments)),
          template_arguments(std::move(template_arguments)) {}

    std::string toString() override;
};

class FunctionCallNameBasedArgument : public AST {
  public:
    std::string_view name;
    AST *expression;

  public:
    FunctionCallNameBasedArgument(std::string_view name, AST *expression,
                                  Location location)
        : AST(ASTType::FUNCTION_CALL_NAME_BASED_ARGUMENT, location), name(name),
          expression(expression) {}

    std::string toString() override;
};

class Type : public AST {
  public:
    std::string_view literal_value;
    TokenType type_;
    bool is_pointer;
    bool is_array;
    bool is_optional;
    bool is_throw_statement;
    std::optional<std::vector<AST *>> template_values = std::nullopt;

  public:
    Type(TokenType type_, std::string_view literal_value, bool is_pointer,
         bool is_array, bool is_optional, bool is_throw_statement,
         Location location)
        : AST(ASTType::TYPE, location), type_(type_),
          literal_value(literal_value), is_pointer(is_pointer),
          is_array(is_array), is_optional(is_optional),
          is_throw_statement(is_throw_statement) {}

    std::string toString() override;
};

class StructDeclaration : public AST {
  private:
    using TemplateDeclaration = class TemplateDeclaration;

  public:
    std::string_view name;
    CompoundStatement *body;
    std::vector<TokenType> qualifiers;
    std::optional<TemplateDeclaration *> template_declaration;
    bool is_union = false;

  public:
    StructDeclaration(std::string_view name, std::vector<TokenType> qualifiers,
                      CompoundStatement *body,
                      std::optional<TemplateDeclaration *> template_declaration,
                      bool is_union, Location location)
        : AST(ASTType::STRUCT_DECLARATION, location), name(name), body(body),
          qualifiers(std::move(qualifiers)),
          template_declaration(template_declaration), is_union(is_union) {}

    std::string toString() override;
};

class StructMemberAccess : public AST {
  public:
    // my_struct.my_variable
    std::string_view struct_variable; // my_struct
    AST *struct_member;               // my_variable
    std::optional<FunctionCall *> function_call;

  public:
    StructMemberAccess(std::string_view struct_variable, AST *struct_member,
                       Location location)
        : AST(ASTType::STRUCT_MEMBER_ACCESS, location),
          struct_variable(struct_variable), struct_member(struct_member) {}

    StructMemberAccess(std::optional<FunctionCall *> function_call,
                       AST *struct_member, Location location)
        : AST(ASTType::STRUCT_MEMBER_ACCESS, location),
          function_call(function_call), struct_member(struct_member) {}

    std::string toString() override;
};

class StructInitializerCall : public AST {
  public:
    std::optional<std::vector<AST *>> arguments;
    bool is_deinit = false;
    std::optional<std::string_view> variable_name = std::nullopt;

  public:
    StructInitializerCall(std::vector<AST *> arguments, Location location)
        : AST(ASTType::STRUCT_INITIALIZER_CALL, location),
          arguments(std::move(arguments)) {}

    StructInitializerCall(std::string_view variable_name, bool is_deinit,
                          Location location)
        : AST(ASTType::STRUCT_INITIALIZER_CALL, location),
          variable_name(variable_name), is_deinit(is_deinit) {}

    StructInitializerCall(std::string_view variable_name,
                          std::vector<AST *> arguments, Location location)
        : AST(ASTType::STRUCT_INITIALIZER_CALL, location),
          variable_name(variable_name), arguments(std::move(arguments)) {}

    std::string toString() override;
};

class StructInitializerDeclaration : public AST {
  public:
    std::optional<std::vector<FunctionArgument *>> arguments = std::nullopt;
    CompoundStatement *body;

  public:
    StructInitializerDeclaration(std::vector<FunctionArgument *> arguments,
                                 CompoundStatement *body, Location location)
        : AST(ASTType::STRUCT_INITIALIZER_DECLARATION, location),
          arguments(std::move(arguments)), body(body) {}

    StructInitializerDeclaration(CompoundStatement *body, Location location)
        : AST(ASTType::STRUCT_INITIALIZER_DECLARATION, location), body(body) {}

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
        : AST(ASTType::ENUM_DECLARATION, location), name(name),
          cases(std::move(cases)), qualifiers(std::move(qualifiers)) {}

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

class EnumCaseAccess : public AST {
  public:
    std::string_view case_name;

  public:
    EnumCaseAccess(std::string_view case_name, Location location)
        : AST(ASTType::ENUM_CASE_ACCESS, location), case_name(case_name) {}

    std::string toString() override;
};

class VariableDeclaration : public AST {
  public:
    std::string_view name;
    std::optional<AST *> type;
    std::optional<AST *> value = std::nullopt;
    std::vector<TokenType> qualifiers;
    bool is_let;

  public:
    VariableDeclaration(std::string_view name, std::optional<AST *> type,
                        std::optional<AST *> value,
                        std::vector<TokenType> qualifiers, bool is_let,
                        Location location)
        : AST(ASTType::VARIABLE_DECLARATION, location), name(name), type(type),
          value(value), qualifiers(std::move(qualifiers)), is_let(is_let) {}

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

class RangeBasedForLoop : public AST {
  public:
    std::string_view name;
    AST *name2;
    std::optional<AST *> for_index;

    CompoundStatement *body;

  public:
    RangeBasedForLoop(std::string_view name, AST *name2,
                      std::optional<AST *> for_index, CompoundStatement *body,
                      Location location)
        : AST(ASTType::RANGE_BASED_FOR, location), name(name), name2(name2),
          for_index(for_index), body(body) {}

    std::string toString() override;
};

class WhileLoop : public AST {
  public:
    AST *expression;
    CompoundStatement *body;

  public:
    WhileLoop(AST *expression, CompoundStatement *body, Location location)
        : AST(ASTType::WHILE, location), expression(expression), body(body) {}

    std::string toString() override;
};

class Return : public AST {
  public:
    std::optional<AST *> expression;
    bool is_throw_statement;

  public:
    Return(std::optional<AST *> expression, bool is_throw_statement,
           Location location)
        : AST(ASTType::RETURN, location), expression(expression),
          is_throw_statement(is_throw_statement){};

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
        : AST(ASTType::IF, location), if_condition(if_condition),
          if_body(if_body), else_if_conditions(std::move(else_if_conditions)),
          else_if_bodies(std::move(else_if_bodies)), else_body(else_body){};

    std::string toString() override;
};

class ASM : public AST {
  public:
    std::vector<std::string_view> instructions;

  public:
    ASM(std::vector<std::string_view> instructions, Location location)
        : AST(ASTType::ASM, location), instructions(std::move(instructions)){};

    std::string toString() override;
};

class GOTO : public AST {
  public:
    std::string_view label;
    bool is_goto_token; // if it's a goto token or a label being defined

  public:
    GOTO(std::string_view label, bool is_goto_token, Location location)
        : AST(ASTType::GOTO, location), label(label),
          is_goto_token(is_goto_token){};

    std::string toString() override;
};

class SwitchStatement : public AST {
  private:
    using SwitchCase = class SwitchCase;

  public:
    AST *expression; // switch (...)
    std::vector<SwitchCase *> cases;

  public:
    SwitchStatement(AST *expression, std::vector<SwitchCase *> cases,
                    Location location)
        : AST(ASTType::SWITCH, location), expression(expression),
          cases(std::move(cases)) {}

    std::string toString() override;
};

class SwitchCase : public AST {
  public:
    std::optional<AST *> expression = std::nullopt; // case 40
    CompoundStatement *body;
    bool is_case; // could be default

  public:
    SwitchCase(AST *expression, CompoundStatement *body, bool is_case,
               Location location)
        : AST(ASTType::SWITCH_CASE, location), expression(expression),
          body(body), is_case(is_case) {}

    SwitchCase(CompoundStatement *body, bool is_case, Location location)
        : AST(ASTType::SWITCH_CASE, location), body(body), is_case(is_case) {}

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

class TryExpression : public AST {
  public:
    AST *expr;
    bool is_force_cast;    // try!
    bool is_optional_cast; // try?

  public:
    TryExpression(AST *expr, bool is_force_cast, bool is_optional_cast,
                  Location location)
        : AST(ASTType::TRY, location), expr(expr), is_force_cast(is_force_cast),
          is_optional_cast(is_optional_cast){};

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

class TernaryExpression : public AST {
  public:
    AST *bool_expression;
    AST *first_expression;
    AST *second_expression;

  public:
    TernaryExpression(AST *bool_expression, AST *first_expression,
                      AST *second_expression, Location location)
        : AST(ASTType::TERNARY_EXPRESSION, location),
          bool_expression(bool_expression), first_expression(first_expression),
          second_expression(second_expression) {}

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

class Typealias : public AST {
  public:
    std::string_view type_name;
    AST *type_value;

  public:
    Typealias(std::string_view type_name, AST *type_value, Location location)
        : AST(ASTType::TYPEALIAS, location), type_name(type_name),
          type_value(type_value) {}

    std::string toString() override;
};

class ASTToken : public AST {
  public:
    TokenType type;

  public:
    ASTToken(TokenType type, Location location)
        : AST(ASTType::TOKEN, location), type(type) {}

    std::string toString() override { return tokenTypeAsLiteral(this->type); }
};

class TemplateDeclaration : public AST {
  private:
    using StructDeclarationArgument = class TemplateDeclarationArgument;

  public:
    std::vector<StructDeclarationArgument *> arguments;

  public:
    TemplateDeclaration(std::vector<StructDeclarationArgument *> arguments,
                        Location location)
        : AST(ASTType::TOKEN, location), arguments(std::move(arguments)) {}

    std::string toString() override;
};

class TemplateDeclarationArgument : public AST {
  public:
    TokenType type;
    std::string_view name;

  public:
    TemplateDeclarationArgument(std::string_view name, TokenType type,
                                Location location)
        : AST(ASTType::TOKEN, location), name(name), type(type) {}

    std::string toString() override { return "elkamfalwkefmalkefmalwk"; }
};

class FirstClassFunction : public AST {
  public:
    std::optional<AST *> return_type;
    std::vector<FunctionArgument *> function_arguments;
    bool is_type = false;

  public:
    FirstClassFunction(std::optional<AST *> return_type,
                       std::vector<FunctionArgument *> function_arguments,
                       bool is_type, Location location)
        : AST(ASTType::FIRST_CLASS_FUNCTION, location),
          return_type(return_type),
          function_arguments(std::move(function_arguments)) {}

    std::string toString() override;
};

class DoCatchStatement : public AST {
  public:
    CompoundStatement *do_body;
    CompoundStatement *catch_body;
    std::optional<AST *> catch_expression;

  public:
    DoCatchStatement(CompoundStatement *do_body, CompoundStatement *catch_body,
                     std::optional<AST *> catch_expression, Location location)
        : AST(ASTType::DO_CATCH, location), do_body(do_body),
          catch_body(catch_body), catch_expression(catch_expression){};

    std::string toString() override;
};

class CastExpression : public AST {
  public:
    AST *cast_value;
    AST *cast_type;
    bool is_force_cast = false;
    bool is_optional_cast = false;

  public:
    CastExpression(AST *cast_value, AST *cast_type, bool is_force_cast,
                   bool is_optional_cast, Location location)
        : AST(ASTType::CAST, location), cast_value(cast_value),
          cast_type(cast_type), is_force_cast(is_force_cast),
          is_optional_cast(is_optional_cast) {}

    std::string toString() override;
};

class OperatorOverload : public AST {
  public:
    AST *return_type;
    std::vector<TokenType> operators;
    std::vector<FunctionArgument *> arguments;
    CompoundStatement *body;

  public:
    OperatorOverload(AST *return_type, std::vector<TokenType> operators,
                     std::vector<FunctionArgument *> arguments,
                     CompoundStatement *body, Location location)
        : AST(ASTType::OPERATOR_OVERLOAD, location), return_type(return_type),
          operators(std::move(operators)), arguments(std::move(arguments)),
          body(body) {}

    std::string toString() override;
};

class ArrayAccess : public AST {
  public:
    AST *index;
    std::optional<FunctionCall *> function_call;
    std::string_view variable_name;

  public:
    ArrayAccess(std::string_view variable_name, AST *index, Location location)
        : AST(ASTType::ARRAY_ACCESS, location), variable_name(variable_name),
          index(index) {}

    ArrayAccess(FunctionCall *function_call, AST *index, Location location)
        : AST(ASTType::ARRAY_ACCESS, location), function_call(function_call),
          index(index) {}

    std::string toString() override;
};

class ArrayCreation : public AST {
  public:
    std::vector<AST *> items;

  public:
    ArrayCreation(std::vector<AST *> &items, Location location)
        : AST(ASTType::ARRAY_CREATION, location), items(std::move(items)) {}

    std::string toString() override;
};

class OptionalUnwrapExpression : public AST {
  public:
    AST *first_expression;
    AST *if_nilled_value;

  public:
    OptionalUnwrapExpression(AST *first_expression, AST *if_nilled_value,
                             Location location)
        : AST(ASTType::OPTIONAL_UNWRAP, location),
          first_expression(first_expression), if_nilled_value(if_nilled_value) {
    }

    std::string toString() override;
};

class ForceUnwrapExpression : public AST {
  public:
    AST *expression;

  public:
    ForceUnwrapExpression(AST *expression, Location location)
        : AST(ASTType::FORCE_UNWRAP, location), expression(expression) {}

    std::string toString() override;
};

#endif // DRAST_AST_H
