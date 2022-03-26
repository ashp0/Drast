//
// Created by Ashwin Paudel on 2022-03-26.
//

#include "AST.h"

#define PRINT_QUALIFIERS(type)                                                 \
    for (auto &qualifier : this->qualifiers) {                                 \
        type += tokenTypeAsLiteral(qualifier);                                 \
        type += " ";                                                           \
    }

uint32_t indent = 0;

#define ADD_INDENTS(type)                                                      \
    for (int i = 0; i < indent; i++)                                           \
        type += "\t";

std::string CompoundStatement::toString() {
    std::string compound = "{\n";
    indent += 1;
    for (auto &statement : this->statements) {
        ADD_INDENTS(compound);
        //        compound += "\t";
        compound += statement->toString();
        compound += "\n";
    }

    indent -= 1;
    ADD_INDENTS(compound)

    compound += "}";
    return compound;
}
std::string ImportStatement::toString() {
    std::string import("import ");
    import += this->module_name;
    return import;
}

std::string FunctionDeclaration::toString() {
    std::string function;
    PRINT_QUALIFIERS(function)

    function += this->return_type->toString();

    function += " :: ";

    function += this->name;

    function += "(";

    for (auto &argument : arguments) {
        function += argument->toString();
        function += ", ";
    }

    function.resize(function.size() - 2);
    function += ") ";

    if (this->body) {
        function += this->body.value()->toString();
    }

    return function;
}

std::string FunctionArgument::toString() {
    std::string argument;

    if (!is_vaarg) {
        argument += this->type.value()->toString();
        argument += " ";
        argument += this->name.value();
    } else {
        argument += "...";
    }

    return argument;
}

std::string FunctionCall::toString() {
    std::string function_call;
    function_call += this->name;
    function_call += "()";
    return function_call;
}

std::string Type::toString() {
    std::string type;
    // type += tokenTypeAsLiteral(this->type);
    // type += "(";
    type += this->literal_value;
    // type += ")";

    return type;
}

std::string StructDeclaration::toString() {
    std::string struct_declaration;

    PRINT_QUALIFIERS(struct_declaration)

    struct_declaration += "struct ";
    struct_declaration += this->name;
    struct_declaration += " ";
    struct_declaration += this->body->toString();

    return struct_declaration;
}

std::string EnumDeclaration::toString() {
    std::string enum_declaration;

    PRINT_QUALIFIERS(enum_declaration)

    enum_declaration += "enum ";
    enum_declaration += this->name;
    enum_declaration += " {\n";

    indent += 1;
    for (auto &case_ : this->cases) {
        ADD_INDENTS(enum_declaration)
        enum_declaration += case_->toString();
        enum_declaration += ",\n";
    }
    indent -= 1;
    ADD_INDENTS(enum_declaration)
    enum_declaration += "}";

    return enum_declaration;
}

std::string EnumCase::toString() {
    std::string enum_case;
    enum_case += this->name;
    enum_case += " = ";
    enum_case += this->value->toString();

    return enum_case;
}

std::string VariableDeclaration::toString() {
    std::string variable_declaration;
    variable_declaration += this->type->toString();
    variable_declaration += " ";

    variable_declaration += this->name;

    if (this->value) {
        variable_declaration += " = ";
        variable_declaration += this->value.value()->toString();
    }

    return variable_declaration;
}

std::string ForLoop::toString() { return "FOR LOOP"; }

std::string Return::toString() { return "RETURN"; }

std::string If::toString() { return "IF"; }

std::string ASM::toString() {
    std::string asm_;

    asm_ += "ASM (\n";

    indent += 1;
    for (auto &instruction : this->instructions) {
        ADD_INDENTS(asm_)
        asm_ += instruction;
        asm_ += "\n";
    }
    indent -= 1;
    ADD_INDENTS(asm_)
    asm_ += ")";
    return asm_;
}

std::string GOTO::toString() { return "GOTO"; }

std::string BinaryExpression::toString() {
    std::string binary_expression;
    binary_expression += this->left->toString();
    binary_expression += " ";
    binary_expression += this->right->toString();

    return binary_expression;
}

std::string UnaryExpression::toString() {
    std::string unary_expression;
    unary_expression += tokenTypeAsLiteral(this->op);
    unary_expression += this->expr->toString();
    return unary_expression;
}

std::string LiteralExpression::toString() {
    std::string literal_expression;
    literal_expression += tokenTypeAsLiteral(this->type);
    return literal_expression;
}

std::string GroupingExpression::toString() { return "GroupingExpression"; }
