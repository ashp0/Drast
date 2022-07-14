//
// Created by Ashwin Paudel on 2022-06-03.
//

#ifndef DRAST_NODE_H
#define DRAST_NODE_H

#include <string>
#include <utility>
#include <vector>
#include "../Common/Location.h"
#include "../Lexer/Token.h"
#include <algorithm>

class Node;

class FunctionDeclaration;

class VariableDeclaration;

class ConstantDeclaration;

class Argument;

class TypeNode;

class IfCondition;

using Expression = Node;

enum class NodeType {
    COMPOUND,
    BLOCK,
    IMPORT,
    STRUCT_DECLARATION,
    ENUM_DECLARATION,
    ENUM_DOT,
    FUNCTION_DECLARATION,
    ARGUMENT,
    IF_STATEMENT,
    IF_CONDITION,
    WHILE_STATEMENT,
    FOR_LOOP,
    CONSTANT_DECLARATION,
    VARIABLE_DECLARATION,
    RETURN_STATEMENT,
    BINARY,
    ASSIGN,
    RANGE,
    UNARY,
    CALL,
    GET,
    LITERAL,
    ARRAY,
    GROUPING,
    TYPE,
};


class Node {
public:
    Location location;
    NodeType type;
public:
    explicit Node(NodeType type, Location location) : type(type), location(location) {}

    virtual ~Node() = default;

    [[nodiscard]] virtual std::string toString() const = 0;

    [[nodiscard]] virtual std::string generate() const = 0;
};

class Compound : public Node {
public:
    std::vector<Node *> statements;
public:
    explicit Compound(Location location) : Node(NodeType::COMPOUND, location) {}

    [[nodiscard]] std::string toString() const override;

    [[nodiscard]] std::string generate() const override;
};

class Block final : public Node {
public:
    std::vector<Node *> statements;
public:
    explicit Block(Location location) : Node(NodeType::BLOCK, location) {}

    [[nodiscard]] std::string toString() const override;

    [[nodiscard]] std::string generate() const override;
};

class Import final : public Node {
public:
    std::string module_name;
    bool is_library = false;
public:
    Import(std::string module_name, Location location) : Node(NodeType::IMPORT, location),
                                                         module_name(std::move(module_name)) {}

    Import(std::string module_name, bool is_library, Location location) : Node(NodeType::IMPORT, location),
                                                                          module_name(std::move(module_name)),
                                                                          is_library(is_library) {}

    [[nodiscard]] std::string toString() const override;

    [[nodiscard]] std::string generate() const override;
};

class StructDeclaration final : public Node {
public:
    std::string name;

    std::vector<ConstantDeclaration *> constants;
    std::vector<VariableDeclaration *> variables;
    std::vector<FunctionDeclaration *> functions;
public:
    explicit StructDeclaration(Location location) : Node(NodeType::STRUCT_DECLARATION, location) {}

    [[nodiscard]] std::string toString() const override;

    [[nodiscard]] std::string generate() const override;
};

class TypeNode final : public Node {
public:
    TokenType node_type = TokenType::LV_IDENTIFIER;
    std::optional<std::string> identifier_name = std::nullopt;
    bool is_array;

public:
    TypeNode(TokenType node_type, bool is_array, Location location) : Node(NodeType::TYPE, location),
                                                                      node_type(node_type), is_array(is_array) {}

    TypeNode(std::string identifier_name, bool is_array, Location location) : Node(NodeType::TYPE, location),
                                                                              identifier_name(identifier_name),
                                                                              is_array(is_array) {}

    [[nodiscard]] std::string toString() const override;

    [[nodiscard]] std::string generate() const override;
};

class EnumDeclaration final : public Node {
public:
    std::string name;
    std::vector<std::string> cases;
public:
    explicit EnumDeclaration(Location location) : Node(NodeType::ENUM_DECLARATION, location) {}

    [[nodiscard]] std::string toString() const override;

    [[nodiscard]] std::string generate() const override;
};

class EnumDot final : public Node {
public:
    std::string case_name;
    std::string from_enum;
public:
    EnumDot(std::string &enum_name, Location location) : Node(NodeType::ENUM_DOT, location),
                                                         case_name(enum_name) {
        std::transform(this->case_name.begin(), this->case_name.end(), this->case_name.begin(), ::toupper);
    }

    [[nodiscard]] std::string toString() const override;

    [[nodiscard]] std::string generate() const override;
};

class FunctionDeclaration final : public Node {
public:
    std::string name;
    std::vector<Argument *> arguments;
    std::optional<TypeNode *> return_type;
    Block *block = nullptr;
    bool is_struct_function = false;
public:
    explicit FunctionDeclaration(Location location) : Node(NodeType::FUNCTION_DECLARATION, location) {}

    [[nodiscard]] std::string toString() const override;

    [[nodiscard]] std::string generate() const override;
};

class Argument final : public Node {
public:
    std::string name;
    TypeNode *arg_type = nullptr;
public:
    explicit Argument(Location location) : Node(NodeType::ARGUMENT, location) {}

    [[nodiscard]] std::string toString() const override;

    [[nodiscard]] std::string generate() const override;
};


class IfCondition final : public Node {
public:
    Expression *expr = nullptr;
    Block *block = nullptr;
    bool is_if = false;
public:
    explicit IfCondition(Location location) : Node(NodeType::IF_CONDITION, location) {}

    [[nodiscard]] std::string toString() const override;

    [[nodiscard]] std::string generate() const override;
};

class IfStatement final : public Node {
public:
    IfCondition *if_condition = nullptr;
    std::vector<IfCondition *> elif_conditions;
    std::optional<Block *> else_block = std::nullopt;
public:
    explicit IfStatement(Location location) : Node(NodeType::IF_STATEMENT, location) {}

    [[nodiscard]] std::string toString() const override;

    [[nodiscard]] std::string generate() const override;
};

class WhileStatement final : public Node {
public:
    Expression *condition = nullptr;
    Block *body = nullptr;

public:
    explicit WhileStatement(Location location) : Node(NodeType::WHILE_STATEMENT, location) {}

    [[nodiscard]] std::string toString() const override;

    [[nodiscard]] std::string generate() const override;
};

class ForLoop final : public Node {
public:
    std::string variable_name;
    std::string by;
    Expression *array = nullptr;
    Block *body = nullptr;
public:
    explicit ForLoop(Location location) : Node(NodeType::FOR_LOOP, location) {}

    [[nodiscard]] std::string toString() const override;

    [[nodiscard]] std::string generate() const override;
};

class ConstantDeclaration final : public Node {
public:
    std::string const_name;
    std::optional<TypeNode *> const_type = std::nullopt;
    Expression *value = nullptr;
public:
    explicit ConstantDeclaration(Location location) : Node(NodeType::CONSTANT_DECLARATION, location) {}

    [[nodiscard]] std::string toString() const override;

    [[nodiscard]] std::string generate() const override;
};

class VariableDeclaration final : public Node {
public:
    const std::string variable_name;
    std::optional<TypeNode *> variable_type = std::nullopt;
    std::optional<Expression *> expr = std::nullopt;

public:
    VariableDeclaration(std::string variable_name, Expression *expr, Location location) : Node(
            NodeType::VARIABLE_DECLARATION, location), variable_name(std::move(variable_name)), expr(expr) {}

    VariableDeclaration(std::string variable_name, TypeNode *variable_type, Location location) : Node(
            NodeType::VARIABLE_DECLARATION, location), variable_name(std::move(variable_name)), variable_type(
            variable_type) {}

    [[nodiscard]] std::string toString() const override;

    [[nodiscard]] std::string generate() const override;
};

class ReturnStatement final : public Node {
public:
    Expression *expr = nullptr;
public:
    explicit ReturnStatement(Location location) : Node(NodeType::RETURN_STATEMENT, location) {}

    [[nodiscard]] std::string toString() const override;

    [[nodiscard]] std::string generate() const override;
};

class Binary : public Node {
public:
    Expression *left;
    TokenType op;
    Expression *right;
public:
    Binary(Expression *left, TokenType op, Expression *right, Location location) : Node(NodeType::BINARY, location),
                                                                                   left(left), op(op),
                                                                                   right(right) {}


    [[nodiscard]] std::string toString() const override;

    [[nodiscard]] std::string generate() const override;
};

class Assign final : public Node {
public:
    Expression *name;
    Expression *value;
public:
    Assign(Expression *name, Expression *value, Location location) : Node(NodeType::ASSIGN, location),
                                                                     name(name), value(value) {}

    [[nodiscard]] std::string toString() const override;

    [[nodiscard]] std::string generate() const override;
};


class Range final : public Node {
public:
    Expression *from;
    Expression *to;
public:
    Range(Expression *from, Expression *to, Location location) : Node(NodeType::RANGE, location), from(from),
                                                                 to(to) {}

    [[nodiscard]] std::string toString() const override;

    [[nodiscard]] std::string generate() const override;
};

class Unary final : public Node {
public:
    Expression *expr;
    TokenType op;
public:
    Unary(Expression *expr, TokenType op, Location location) : Node(NodeType::UNARY, location), expr(expr),
                                                               op(op) {}

    [[nodiscard]] std::string toString() const override;

    [[nodiscard]] std::string generate() const override;
};

class Call final : public Node {
public:
    Expression *expr;
    std::vector<Expression *> arguments;
public:
    Call(Expression *expr, Location location) : Node(NodeType::CALL, location),
                                                expr(expr) {}

    [[nodiscard]] std::string toString() const override;

    [[nodiscard]] std::string generate() const override;
};

class Get final : public Node {
public:
    Expression *expr;
    const Expression *second;
public:
    Get(Expression *expr, Expression *second, Location location) : Node(NodeType::GET, location), expr(expr),
                                                                   second(second) {}

    [[nodiscard]] std::string toString() const override;

    [[nodiscard]] std::string generate() const override;
};

class Literal final : public Node {
public:
    TokenType literal_type;
    const std::string literal_value;
public:
    Literal(TokenType literal_type, std::string literal_value, Location location) : Node(NodeType::LITERAL, location),
                                                                                    literal_type(literal_type),
                                                                                    literal_value(
                                                                                            std::move(literal_value)) {}

    [[nodiscard]] std::string toString() const override;

    [[nodiscard]] std::string generate() const override;
};

class Array final : public Node {
public:
    std::vector<Expression *> items;
public:
    explicit Array(Location location) : Node(NodeType::ARRAY, location) {}

    [[nodiscard]] std::string toString() const override;

    [[nodiscard]] std::string generate() const override;
};

class Grouping final : public Node {
public:
    Expression *expr;
public:
    Grouping(Expression *expr, Location location) : Node(NodeType::GROUPING, location), expr(expr) {}

    [[nodiscard]] std::string toString() const override;

    [[nodiscard]] std::string generate() const override;
};

class LiteralTokenType final : public Node {
public:
    TokenType literal_type;
public:
    LiteralTokenType(TokenType literal_type, Location location) : Node(NodeType::LITERAL, location),
                                                                  literal_type(literal_type) {}

    [[nodiscard]] std::string toString() const override;

    [[nodiscard]] std::string generate() const override;
};


#endif //DRAST_NODE_H
