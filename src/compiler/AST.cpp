//
// Created by Ashwin Paudel on 2022-03-20.
//

#include "AST.h"

std::string CompoundStatement::toString() const
{
    std::stringstream ss;
    ss << "CompoundStatement: {";
    for (auto &statement : statements)
    {
        ss << (*statement).toString() << "; ";
    }
    ss << "}";
    return ss.str();
}

std::string Import::toString() const
{
    return "import '" + this->import_path + "'";
}

std::string FunctionDeclaration::toString() const
{
    std::stringstream ss;
    ss << "FunctionDeclaration: " << name << "(";
    for (auto &argument : arguments)
    {
        ss << (*argument).toString() << ", ";
    }
    ss << "\b\b) {";
    ss << *body << "}";
    return ss.str();
}

std::string FunctionArgument::toString() const
{
    if (!is_vaarg)
    {
        return name.value() + ": " + type->get()->toString();
    }

    return "...";
}

std::string FunctionCall::toString() const
{
    std::stringstream ss;
    ss << name << "(";
    for (auto &argument : arguments)
    {
        ss << argument->toString() << ", ";
    }
    ss << ")";
    return ss.str();
}

std::string Type::toString() const
{
    return this->token.value + (is_array ? "[]" : "") + (is_pointer ? "*" : "") + (is_optional ? "?" : "");
}

std::string StructDeclaration::toString() const
{
    std::stringstream ss;
    ss << "StructDeclaration: " << name << " {\n";
    for (auto &field : fields)
    {
        ss << field->toString() << "\n";
    }
    ss << "}";
    return ss.str();
}

std::string StructInitializerCall::toString() const
{
    std::stringstream ss;
    ss << name << "(";
    for (auto &argument : arguments)
    {
        ss << argument->toString() << ", ";
    }
    ss << ")";
    return ss.str();
}

std::string EnumDeclaration::toString() const
{
    std::stringstream ss;
    ss << "EnumDeclaration: " << name << " {\n";
    for (auto &case_ : cases)
    {
        ss << case_->toString() << "\n";
    }
    ss << "}";
    return ss.str();
}

std::string EnumCase::toString() const
{
    return "case " + name + " = " + value->toString();
}

std::string VariableDeclaration::toString() const
{
    if (has_value)
    {
        return type->toString() + " " + name + " = " + value->toString();
    }
    else
    {
        return type->toString() + " " + name;
    }
}