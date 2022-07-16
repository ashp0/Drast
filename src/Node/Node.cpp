//
// Created by Ashwin Paudel on 2022-06-03.
//

#include "Node.h"

#define ADVANCE_TAB \
    for (int i = 0; i < indent; i++) { \
        result += '\t';                \
    }

static int indent = 0;

std::string Compound::toString() const {
    std::string result;
    for (const auto &stmt: statements) {
        result += stmt->toString() + "\n";
    }
    return result;
}

std::string Compound::generate() const {
    std::string result;
    for (const auto &stmt: statements) {
        result += stmt->generate();
        if (stmt->type == NodeType::VARIABLE_DECLARATION) {
            result += ";";
        }
        result += "\n\n";
    }
    return result;
}

std::string Block::toString() const {
    std::string result;
    indent += 1;
    for (const auto &stmt: statements) {
        ADVANCE_TAB
        result += stmt->toString() + '\n';
    }
    indent -= 1;
    return result;
}

std::string Block::generate() const {
    std::string result;

    indent += 1;
    for (auto &statement: statements) {
        ADVANCE_TAB
        result += statement->generate();
        if (!(statement->type == NodeType::IF_CONDITION || statement->type == NodeType::FUNCTION_DECLARATION ||
              statement->type == NodeType::WHILE_STATEMENT || statement->type == NodeType::IF_STATEMENT)) {
            result += ";";
        }
        result += "\n";
    }
    indent -= 1;

    return result;
}

std::string Import::toString() const {
    if (is_library) {
        return "import " + module_name;
    } else {
        return "import '" + module_name + "'";
    }
}

std::string Import::generate() const {
    return "#include " + module_name;
}

VariableDeclaration *StructDeclaration::locate_variable(const std::string &variable_name) {
    for (const auto &item: variables) {
        if (item->variable_name == variable_name) {
            return item;
        }
    }

    return nullptr;
}

std::string StructDeclaration::toString() const {
    std::string result = "struct " + name + ":\n";
    indent += 1;

    for (const auto &variable: variables) {
        ADVANCE_TAB
        result += variable->toString() + "\n";
    }

    for (const auto &function: functions) {
        ADVANCE_TAB
        result += function->toString() + "\n";
    }

    indent -= 1;

    return result;
}

std::string StructDeclaration::generate() const {
    std::string result = "struct " + name + " {\n";

    indent += 1;

    for (const auto &variable: variables) {
        ADVANCE_TAB
        result += variable->generate() + ";\n";
    }

    for (auto &function: functions) {
        ADVANCE_TAB
        if (function->return_type) {
            result += function->return_type.value()->generate();
        } else {
            result += "void";
        }
        result += " (*";
        result += function->name;
        result += ")(";
        for (const auto &argument: function->arguments) {
            result += argument->generate();
            result += ", ";
        }
        if (!function->arguments.empty()) {
            result.resize(result.size() - 2);
        }
        result += ")";

        result += ";\n";
    }

    indent -= 1;

    ADVANCE_TAB
    result += "};\n\n";

    for (auto &function: functions) {
        auto temp = function->name;
        function->name = function->mangled_name;
        result += function->generate();
        function->name = temp;
        result += "\n\n";
    }

    return result;
}

std::string EnumDeclaration::toString() const {
    std::string result = "enum " + name + ":\n";
    indent += 1;
    for (const auto &case_: cases) {
        ADVANCE_TAB
        result += case_ + "\n";
    }

    return result;
}

std::string EnumDeclaration::generate() const {
    std::string result = "enum " + name + " {\n";
    indent += 1;
    for (const auto &case_: cases) {
        ADVANCE_TAB
        result += case_ + ",\n";
    }

    indent -= 1;
    ADVANCE_TAB
    result += "};";

    return result;
}

std::string EnumDot::toString() const {
    std::string result = ".";
    result += case_name;

    return result;
}

std::string EnumDot::generate() const {
    return case_name;
}

std::string FunctionDeclaration::toString() const {
    std::string result = "fn ";
    if (is_struct_function) {
        result += '.';
    }
    result += name + "(";
    for (const auto &arg: arguments) {
        result += arg->toString() + ", ";
    }
    if (!arguments.empty()) {
        result.resize(result.size() - 2);
    }
    result += ")";

    if (return_type) {
        result += " -> ";
        if (is_constant) {
            result += "const ";
        }
        result += return_type.value()->toString();
    }

    result += ":\n";
    result += block->toString();
    return result;
}

std::string FunctionDeclaration::generate() const {
    std::string result;

    if (is_constant) {
        result += "const ";
    }
    if (return_type) {
        result += return_type.value()->generate();
        result += " ";
    } else {
        result += "void ";
    }

    result += name;
    result += "(";

    for (auto &arg: arguments) {
        result += arg->generate();
        result += ", ";
    }

    if (!arguments.empty()) {
        result.resize(result.size() - 2);
    }

    result += ") {\n";

    result += block->generate();

    ADVANCE_TAB
    result += "}";

    return result;
}

std::string Argument::toString() const {
    return name + ": " + arg_type->toString();
}

std::string Argument::generate() const {
    return arg_type->generate() + " " + name;
}

std::string IfStatement::toString() const {
    std::string result = if_condition->toString();
    result += "\n";
    for (const auto &condition: elif_conditions) {
        ADVANCE_TAB
        result += condition->toString();
        result += "\n";
    }
    if (else_block) {
        ADVANCE_TAB
        result += "else:\n";
        result += else_block.value()->toString();
    }

    return result;
}

std::string IfStatement::generate() const {
    std::string result = if_condition->generate();
    result += "\n";
    for (const auto &condition: elif_conditions) {
        ADVANCE_TAB
        result += condition->generate();
        result += "\n";
    }
    if (else_block) {
        ADVANCE_TAB
        result += "else {\n";
        result += else_block.value()->generate();
        ADVANCE_TAB
        result += "}\n";
    }

    return result;
}

std::string IfCondition::toString() const {
    std::string result = is_if ? "if " : "elif ";
    result += expr->toString();
    result += ":\n";
    result += block->toString();

    return result;
}

std::string IfCondition::generate() const {
    std::string result = is_if ? "if (" : "else if (";
    result += expr->generate();
    result += ") {\n";
    result += block->generate();
    ADVANCE_TAB
    result += "}\n";

    return result;
}

std::string WhileStatement::toString() const {
    std::string result = "while ";
    result += condition->toString();
    result += ":\n";
    result += body->toString();

    return result;
}

std::string WhileStatement::generate() const {
    std::string result = "while (";
    result += condition->generate();
    result += ") {\n";
    result += body->generate();
    ADVANCE_TAB
    result += "}\n";

    return result;
}

std::string ForLoop::toString() const {
    std::string result = "for ";
    result += variable_name;
    result += " in ";
    result += array->toString();
    result += ":\n";
    result += body->toString();

    return result;
}

std::string ForLoop::generate() const {
    return "// Cannot generate for loops: TODO";
}

std::string VariableDeclaration::toString() const {
    std::string result = variable_name;

    if (variable_type) {
        result += ": ";
        result += variable_type.value()->toString();
        if (expr) {
            result += " = ";
        }
    } else {
        result += " := ";
    }

    if (expr) {
        result += expr.value()->toString();
    }

    return result;
}

std::string VariableDeclaration::generate() const {
    std::string result;
    if (is_const) {
        result += "const ";
    }

    if (variable_type) {

        result += variable_type.value()->generate();
        result += " ";
    }

    result += variable_name;
    if (expr) {
        if (expr.value()->type == NodeType::ARRAY) {
            result += "[]";
        }
    }
    if (expr) {
        result += " = ";
        result += expr.value()->generate();
    }

    return result;
}

std::string ReturnStatement::toString() const {
    std::string result = "return ";
    result += expr->toString();

    return result;
}

std::string ReturnStatement::generate() const {
    std::string result = "return ";
    result += expr->generate();

    return result;
}

std::string Binary::toString() const {
    std::string result = left->toString();
    result += " ";
    result += tokenTypeToString(op);
    result += " ";
    result += right->toString();

    return result;
}

std::string Binary::generate() const {
    std::string result = left->generate();
    result += " ";
    result += tokenGenerate(op);
    result += " ";
    result += right->generate();

    return result;
}

std::string Assign::toString() const {
    std::string result = name->toString();
    result += " = ";
    result += value->toString();
    return result;
}

std::string Assign::generate() const {
    std::string result = name->generate();
    result += " = ";
    result += value->generate();
    return result;
}

std::string Range::toString() const {
    std::string result = from->toString();
    result += "..";
    result += to->toString();

    return result;
}

std::string Range::generate() const {
    return "(ERROR: Range)";
}

std::string Unary::toString() const {
    std::string result = tokenTypeToString(op);
    result += expr->toString();

    return result;
}

std::string Unary::generate() const {
    std::string result = tokenGenerate(op);
    result += expr->toString();

    return result;
}

std::string Call::toString() const {
    std::string result = expr->toString();
    result += "(";
    for (const auto &argument: arguments) {
        result += argument->toString();
        result += ", ";
    }
    if (!arguments.empty()) {
        result.resize(result.size() - 2);
    }
    result += ")";

    return result;
}

std::string Call::generate() const {
    std::string result;
    if (original_struct) {
        result += "{\n";

        indent += 1;
        for (int index = 0; index < original_struct->variables.size(); index++) {
            ADVANCE_TAB
            result += ".";
            result += original_struct->variables.at(index)->variable_name;
            result += " = ";
            result += arguments.at(index)->generate();
            result += ",\n";
        }

        for (auto &item: original_struct->functions) {
            ADVANCE_TAB
            result += ".";
            result += item->name;
            result += " = ";
            result += item->mangled_name;
            result += ",\n";
        }
        indent -= 1;

        if (!arguments.empty()) {
            result.resize(result.size() - 2);
        }

        result += '\n';

        ADVANCE_TAB
        result += "}";
    } else {
        result += expr->generate();
        result += "(";
        for (const auto &argument: arguments) {
            result += argument->generate();
            result += ", ";
        }
        if (!arguments.empty()) {
            result.resize(result.size() - 2);
        }
        result += ")";
    }

    return result;
}

std::string Get::toString() const {
    std::string result;
    result += expr->toString();
    result += ".";
    result += second->toString();

    return result;
}

std::string Get::generate() const {
    std::string result;
    result += expr->generate();
    result += ".";
    result += second->generate();

    return result;
}

std::string Literal::toString() const {
    if (literal_type == TokenType::LV_STRING) {
        std::string result;
        result += "\"";
        result += literal_value;
        result += "\"";
        return result;
    } else if (literal_type == TokenType::LV_CHAR) {
        std::string result;
        result += "'";
        result += literal_value;
        result += "'";
        return result;
    }

    return literal_value;
}

std::string Literal::generate() const {
    if (literal_type == TokenType::LV_STRING) {
        std::string result;
        result += "\"";
        result += literal_value;
        result += "\"";
        return result;
    } else if (literal_type == TokenType::LV_CHAR) {
        std::string result;
        result += "'";
        result += literal_value;
        result += "'";
        return result;
    } else if (literal_type == TokenType::LV_TRUE) {
        return "1";
    } else if (literal_type == TokenType::LV_FALSE) {
        return "0";
    }

    return literal_value;
}

std::string Array::toString() const {
    std::string result = "[";
    for (const auto &item: items) {
        result += item->toString();
        result += ", ";
    }

    if (!items.empty()) {
        result.resize(result.size() - 2);
    }

    result += "]";
    return result;
}

std::string Array::generate() const {
    std::string result = "{";
    for (const auto &item: items) {
        result += item->generate();
        result += ", ";
    }

    if (!items.empty()) {
        result.resize(result.size() - 2);
    }

    result += "}";
    return result;
}

std::string Grouping::toString() const {
    std::string result = "(";
    result += expr->toString();
    result += ")";

    return result;
}

std::string Grouping::generate() const {
    std::string result = "(";
    result += expr->generate();
    result += ")";

    return result;
}

std::string LiteralTokenType::toString() const {
    return tokenTypeToString(literal_type);
}

std::string LiteralTokenType::generate() const {
    if (literal_type == TokenType::SELF) {
        return literal_value;
    }
    return tokenGenerate(literal_type);
}

std::string TypeNode::toString() const {
    std::string result;
    if (is_array) {
        result += "[";
    }
    if (node_type == TokenType::LV_IDENTIFIER) {
        result += identifier_name;
    } else {
        result += tokenTypeToString(node_type);
    }
    if (is_array) {
        result += "]";
    }

    return result;
}

std::string TypeNode::generate() const {
    std::string result;

    if (node_type == TokenType::STRUCT) {
        result += "struct ";
        result += identifier_name;
    } else if (node_type == TokenType::LV_IDENTIFIER) {
        result += identifier_name;
    } else {
        result += tokenGenerate(node_type);
    }
    if (is_array) {
        result += "*";
    }

    return result;
}