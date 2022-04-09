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
        (type) += "\t";

std::string CompoundStatement::toString() {
    std::string compound;
    if (this->first_class_function) {
        compound += "!";
    }
    compound += "{ ";
    if (this->first_class_function) {
        compound += this->first_class_function.value()->toString();
    }
    compound += "\n";
    indent += 1;
    for (auto &statement : this->statements) {
        ADD_INDENTS(compound)
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

    if (this->template_declaration) {
        function += ": ";
        function += this->template_declaration.value()->toString();
    }

    if (this->body) {
        function += this->body.value()->toString();
    }

    return function;
}

std::string FunctionArgument::toString() {
    std::string argument;

    if (!is_vaarg) {
        argument += this->type.value()->toString();
        if (name) {
            argument += " ";
            argument += this->name.value();
        }
    } else {
        argument += "...";
    }

    return argument;
}

std::string FunctionCall::toString() {
    std::string function_call;
    function_call += this->name;

    if (this->template_arguments) {
        function_call += "@(";
        for (auto &argument : this->template_arguments.value()) {
            function_call += argument->toString();
            function_call += ", ";
        }
        function_call.pop_back();
        function_call.pop_back();
        function_call += ")";
    }

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
    std::string type_string;
    type_string += this->literal_value;

    if (this->template_values) {
        type_string += "@";
        type_string += "(";
        for (auto &template_value : template_values.value()) {
            type_string += template_value->toString();
            type_string += ", ";
        }
        type_string.pop_back();
        type_string.pop_back();
        type_string += ")";
    }

    type_string += (this->is_pointer ? "*" : "");
    type_string += (this->is_array ? "[]" : "");
    type_string += (this->is_optional ? "? " : "");

    return type_string;
}

std::string StructDeclaration::toString() {
    std::string struct_declaration;

    PRINT_QUALIFIERS(struct_declaration)

    struct_declaration += "struct ";
    struct_declaration += this->name;
    struct_declaration += " ";

    if (this->template_declaration) {
        struct_declaration += ": ";
        struct_declaration += this->template_declaration.value()->toString();
    }

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

    PRINT_QUALIFIERS(variable_declaration)

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
        if (this->is_throw_statement) {
            return "throw " + this->expression.value()->toString();
        }
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

std::string TryExpression::toString() {
    std::string try_expression;
    try_expression += "try";
    if (this->is_optional_cast) {
        try_expression += "?";
    } else if (this->is_force_cast) {
        try_expression += "!";
    }

    try_expression += " ";
    try_expression += this->expr->toString();
    return try_expression;
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

std::string Typealias::toString() {
    std::string typealias;
    typealias += this->type_name;
    typealias += " = ";
    typealias += this->type_value->toString();
    return typealias;
}

std::string StructMemberAccess::toString() {
    std::string struct_member_access;
    struct_member_access += this->struct_variable;
    struct_member_access += ".";
    struct_member_access += this->struct_member->toString();
    return struct_member_access;
}

std::string EnumCaseAccess::toString() {
    std::string enum_case;
    enum_case += ".";
    enum_case += this->case_name;

    return enum_case;
}

std::string StructInitializerCall::toString() {
    std::string function_call;
    if (this->variable_name) {
        function_call += this->variable_name.value();
    }

    if (this->is_deinit) {
        function_call += ".deinit(";
    } else {
        function_call += ".init(";
        for (auto &argument : this->arguments.value()) {
            function_call += argument->toString();
            function_call += ",";
        }

        if (!arguments.value().empty()) {
            function_call.pop_back();
        }
    }
    function_call += ")";
    return function_call;
}

std::string StructInitializerDeclaration::toString() {
    std::string struct_initializer_declaration;

    if (arguments) {
        struct_initializer_declaration += "@(";

        for (auto &argument : arguments.value()) {
            struct_initializer_declaration += argument->toString();
            struct_initializer_declaration += ", ";
        }

        if (!arguments.value().empty()) {
            struct_initializer_declaration.resize(
                struct_initializer_declaration.size() - 2);
        }
    } else {
        struct_initializer_declaration += "~(";
    }

    struct_initializer_declaration += ") ";

    struct_initializer_declaration += this->body->toString();

    return struct_initializer_declaration;
}

std::string TemplateDeclaration::toString() {
    std::string template_declaration;
    template_declaration += "@(";

    for (auto &argument : this->arguments) {
        template_declaration += tokenTypeAsLiteral(argument->type);
        template_declaration += " ";
        template_declaration += argument->name;
        template_declaration += ", ";
    }

    template_declaration.pop_back();
    template_declaration.pop_back();
    template_declaration += ") ";

    return template_declaration;
}

std::string FirstClassFunction::toString() {
    std::string function_type;
    function_type += "$";
    if (this->return_type) {
        function_type += this->return_type.value()->toString();
    }
    function_type += "(";

    for (auto &argument : this->function_arguments) {
        function_type += argument->toString();
        function_type += ", ";
    }

    if (!function_arguments.empty()) {
        function_type.pop_back();
        function_type.pop_back();
    }

    function_type += ")";

    return function_type;
}

std::string DoCatchStatement::toString() {
    std::string do_catch_statement;
    do_catch_statement += "do ";
    do_catch_statement += this->do_body->toString();
    do_catch_statement += " catch ";
    if (catch_expression) {
        do_catch_statement += "(";
        do_catch_statement += this->catch_expression.value()->toString();
        do_catch_statement += ") ";
    }
    do_catch_statement += this->catch_body->toString();

    return do_catch_statement;
}

std::string CastExpression::toString() {
    std::string cast_expression;
    cast_expression += "cast";

    if (this->is_optional_cast) {
        cast_expression += "?";
    } else if (this->is_force_cast) {
        cast_expression += "!";
    }

    cast_expression += "(";
    cast_expression += this->cast_value->toString();
    cast_expression += ", ";
    cast_expression += this->cast_type->toString();
    cast_expression += ")";

    return cast_expression;
}