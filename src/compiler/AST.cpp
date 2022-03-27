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

    if (!arguments.empty()) {
        function.resize(function.size() - 2);
    }

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
    function_call += "(";
    for (auto &argument : this->arguments) {
        function_call += argument->toString();
        function_call += ",";
    }

    if (!arguments.empty()) {
        function_call.pop_back();
    }
    function_call += ")";
    return function_call;
}

std::string Type::toString() {
    std::string type;
    type += this->literal_value;
    type += (this->is_pointer ? "*" : "");
    type += (this->is_array ? "[]" : "");
    type += (this->is_optional ? "? " : "");

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

std::string ForLoop::toString() {
    std::string for_loop;
    for_loop += "for (";
    for_loop += this->first_statement->toString();
    for_loop += ", ";
    for_loop += this->second_statement->toString();
    for_loop += ", ";
    for_loop += this->third_statement->toString();
    for_loop += ") ";
    for_loop += this->body->toString();
    return for_loop;
}

std::string WhileLoop::toString() {
    std::string while_loop;
    while_loop += "while (";
    while_loop += this->expression->toString();
    while_loop += ") ";

    while_loop += this->body->toString();

    return while_loop;
}

std::string Return::toString() {
    if (this->expression) {
        return "return " + this->expression.value()->toString();
    }
    return "return";
}

std::string If::toString() {
    std::string if_;
    if_ += "if (";
    if_ += this->if_condition->toString();
    if_ += ") ";
    if_ += this->if_body->toString();

    for (int i = 0; i < this->else_if_bodies.size(); i++) {
        if_ += " else if ";
        if_ += "(";
        if_ += else_if_conditions[i]->toString();
        if_ += ") ";
        if_ += else_if_bodies[i]->toString();
    }

    if (this->else_body) {
        if_ += " else ";
        if_ += this->else_body.value()->toString();
    }

    return if_;
}

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

std::string GOTO::toString() {
    std::string goto_;
    if (is_goto_token) {
        goto_ += "goto ";
        goto_ += this->label;
    } else {
        goto_ += "\bLABEL :: ";
        goto_ += this->label;
        goto_ += ":";
    }
    return goto_;
}

std::string SwitchStatement::toString() {
    std::string switch_statement;
    switch_statement += "switch (";
    switch_statement += this->expression->toString();
    switch_statement += ") {\n";

    indent += 1;
    for (auto &case_ : this->cases) {
        ADD_INDENTS(switch_statement)
        switch_statement += case_->toString();
        switch_statement += "\n";
    }
    indent -= 1;
    ADD_INDENTS(switch_statement)
    switch_statement += "}";

    return switch_statement;
}

std::string SwitchCase::toString() {
    std::string case_;

    if (is_case) {
        case_ += "case ";
        case_ += this->expression.value()->toString();
    } else {
        case_ += "default";
    }

    case_ += ": ";

    case_ += this->body->toString();

    return case_;
}

std::string BinaryExpression::toString() {
    std::string binary_expression;
    binary_expression += this->left->toString();
    binary_expression += " ";
    binary_expression += tokenTypeAsLiteral(this->op);
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
    if (string_value) {
        literal_expression += string_value.value();
    } else {
        literal_expression += this->value;
    }

    return literal_expression;
}

std::string GroupingExpression::toString() {
    std::string grouping_expression;
    grouping_expression += "(";
    grouping_expression += this->expr->toString();
    grouping_expression += ")";
    return grouping_expression;
}
