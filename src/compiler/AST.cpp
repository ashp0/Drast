//
// Created by Ashwin Paudel on 2022-03-20.
//

#include "AST.h"

std::string CompoundStatement::toString() const {
    std::stringstream ss;
    ss << "CompoundStatement: {";
    for (auto &statement : statements) {
        ss << (*statement).toString() << "; ";
    }
    ss << "}";
    return ss.str();
}

std::string Import::toString() const {
    return "import '" + this->import_path + "'";
}

std::string FunctionDeclaration::toString() const {
    std::stringstream ss;
    ss << "FunctionDeclaration: ";
    for (auto &modifier : modifiers) {
        ss << tokenTypeAsLiteral(modifier) << " ";
    }
    ss << return_type->toString() << " :: " << name << "(";
    for (auto &argument : arguments) {
        ss << (*argument).toString() << ", ";
    }
    if (arguments.size() > 0) {
        ss.seekp(-2, ss.cur);
    }
    ss << ")";
    if (body) {
        ss << "{ " << *body->get() << "}";
    }

    return ss.str();
}

std::string FunctionArgument::toString() const {
    if (!is_vaarg) {
        return name.value() + ": " + type->get()->toString();
    }

    return "...";
}

std::string FunctionCall::toString() const {
    std::stringstream ss;
    ss << name << "(";
    for (auto &argument : arguments) {
        ss << argument->toString() << ", ";
    }
    if (arguments.size() > 0) {
        ss.seekp(-2, ss.cur);
    }
    ss << ")";
    return ss.str();
}

std::string Type::toString() const {
    return this->token.value + (is_array ? "[]" : "") +
           (is_pointer ? "*" : "") + (is_optional ? "?" : "");
}

std::string StructDeclaration::toString() const {
    std::stringstream ss;
    ss << "StructDeclaration: " << name << " {\n";
    for (auto &field : fields) {
        ss << field->toString() << "\n";
    }
    ss << "}";
    return ss.str();
}

std::string StructInitializerCall::toString() const {
    std::stringstream ss;
    ss << name << "(";
    for (auto &argument : arguments) {
        ss << argument->toString() << ", ";
    }
    ss << ")";
    return ss.str();
}

std::string EnumDeclaration::toString() const {
    std::stringstream ss;
    ss << "EnumDeclaration: " << name << " {";
    for (auto &case_ : cases) {
        ss << case_->toString() << ", ";
    }
    ss << "}";
    return ss.str();
}

std::string EnumCase::toString() const {
    return name + " = " + value->toString();
}

std::string VariableDeclaration::toString() const {
    if (value.has_value()) {
        return type->toString() + " " + name + " = " + value->get()->toString();
    } else {
        return type->toString() + " " + name;
    }
}

std::string ASM::toString() const {
    std::stringstream ss;
    ss << "InlineAssembly: (";
    for (auto &instruction : instructions) {
        ss << "`" << instruction << "`, ";
    }

    if (instructions.size() > 0) {
        ss.seekp(-2, ss.cur);
    }
    ss << ")";

    return ss.str();
}

std::string Return::toString() const {
    std::stringstream ss;
    ss << "ReturnStatement: ";
    if (expression.has_value()) {
        ss << expression->get()->toString();
    }
    return ss.str();
}

std::string BinaryExpression::toString() const {
    std::stringstream ss;
    ss << left->toString() << " " << tokenTypeAsLiteral(this->op) << " "
       << right->toString();
    return ss.str();
}

std::string UnaryExpression::toString() const {
    std::stringstream ss;
    ss << tokenTypeAsLiteral(this->op) << expr->toString();
    return ss.str();
}

std::string LiteralExpression::toString() const { return this->value; }

std::string GroupingExpression::toString() const {
    return "(" + this->expr->toString() + ")";
}