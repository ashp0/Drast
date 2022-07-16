//
// Created by Ashwin Paudel on 2022-07-15.
//

#include "TypeChecker.h"
#include <iostream>

void TypeChecker::check() {
    checkStatements(root->statements);
}

void TypeChecker::checkStatements(std::vector<Node *> &statements) {
    for (auto &statement: statements) {
        checkStatement(statement);
    }
}

void TypeChecker::checkStatement(Node *statement) {
    switch (statement->type) {
        case NodeType::IMPORT:
            checkImportStatement(dynamic_cast<Import *>(statement));
            break;
        case NodeType::STRUCT_DECLARATION:
            checkStructDeclaration(dynamic_cast<StructDeclaration *>(statement));
            break;
        case NodeType::FUNCTION_DECLARATION:
            checkFunctionStatement(dynamic_cast<FunctionDeclaration *>(statement));
            break;
        default:
            break;
    }
}

void TypeChecker::checkImportStatement(Import *import) {
    if (!import->is_library) {
        if (!fileExists(import->module_name.c_str())) {
            throw Report("Invalid import");
        }

        import->module_name.insert(0, 1, '"');
        import->module_name.erase(import->module_name.rfind('.'));
        import->module_name += ".h";
        import->module_name += "\"";
    } else {
        auto lib_name = import->module_name;
        import->module_name = "<" + lib_name + ".h>";
    }
}

void TypeChecker::checkStructDeclaration(StructDeclaration *struct_decl) {
    for (auto &func: struct_decl->functions) {
        checkFunctionStatement(func, struct_decl);
    }
}

void TypeChecker::checkFunctionStatement(FunctionDeclaration *function_decl, StructDeclaration *struct_decl) {
    if (struct_decl) {
        function_decl->mangled_name = "__" + struct_decl->name + "__" + function_decl->name + "__" +
                                      (function_decl->is_struct_function ? "__built_in_func__" : "");

        // Add the struct argument
        auto argument = new Argument(Location(0, 0));
        argument->name = "__" + struct_decl->name + "__";
        argument->arg_type = new TypeNode(TokenType::STRUCT, struct_decl->name, Location(0, 0));
        function_decl->arguments.push_back(argument);
    }

    checkBlock(function_decl->block, function_decl, struct_decl);
}

void TypeChecker::checkBlock(Block *block, FunctionDeclaration *function_declaration,
                             StructDeclaration *struct_declaration) {
    for (auto &statement: block->statements) {
        switch (statement->type) {
            default:
                checkExpression(statement, function_declaration, struct_declaration);
                break;
        }
    }
}

void TypeChecker::checkExpression(Expression *expression, FunctionDeclaration *function_declaration,
                                  StructDeclaration *struct_declaration) {
    switch (expression->type) {
        case NodeType::ASSIGN:
            checkAssign(dynamic_cast<Assign *>(expression), function_declaration, struct_declaration);
            break;
        case NodeType::BINARY:
            checkBinary(dynamic_cast<Binary *>(expression), function_declaration, struct_declaration);
            break;
        case NodeType::LITERAL:
            checkLiteral(dynamic_cast<Literal *>(expression), function_declaration, struct_declaration);
            break;
        default:
            break;
    }
}

void TypeChecker::checkAssign(Assign *assign, FunctionDeclaration *function_declaration,
                              StructDeclaration *struct_declaration) {
    checkExpression(assign->name, function_declaration, struct_declaration);
    checkExpression(assign->value, function_declaration, struct_declaration);
}

void TypeChecker::checkBinary(Binary *binary, FunctionDeclaration *function_declaration,
                              StructDeclaration *struct_declaration) {
    checkExpression(binary->left, function_declaration, struct_declaration);
    checkExpression(binary->right, function_declaration, struct_declaration);
}

void TypeChecker::checkLiteral(Literal *literal, FunctionDeclaration *function_declaration,
                               StructDeclaration *struct_declaration) {
    if (literal->literal_type == TokenType::LV_IDENTIFIER) {
        if (!locateVariable(literal->literal_value, function_declaration, struct_declaration)) {
            if (struct_declaration) {
                auto temp = literal->literal_value;
                literal->literal_value = function_declaration->arguments.back()->name + "->" + temp;
            } else {
                throw Report(("Undefined variable: " + literal->literal_value).c_str());
            }
        }
    }
}

bool TypeChecker::locateVariable(const std::string &variable_name, FunctionDeclaration *function_declaration,
                                 StructDeclaration *struct_declaration) {
    for (auto &stmt: function_declaration->block->statements) {
        if (stmt->type == NodeType::VARIABLE_DECLARATION) {
            auto var = dynamic_cast<VariableDeclaration *>(stmt);
            return var->variable_name == variable_name;
        }
    }

    for (auto &arg: function_declaration->arguments) {
        return variable_name == arg->name;
    }

    return false;
}