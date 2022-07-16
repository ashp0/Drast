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
        case NodeType::VARIABLE_DECLARATION:
            checkVariableDeclaration(dynamic_cast<VariableDeclaration *>(statement));
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

void TypeChecker::checkVariableDeclaration(VariableDeclaration *var_decl, FunctionDeclaration *function_declaration,
                                           StructDeclaration *struct_declaration) {
    if (var_decl->expr) {
        auto type = checkExpression(var_decl->expr.value(), function_declaration, struct_declaration);

        if (!var_decl->variable_type) {
            var_decl->variable_type = dynamic_cast<TypeNode *>(type);
        }
    }
}

void TypeChecker::checkBlock(Block *block, FunctionDeclaration *function_declaration,
                             StructDeclaration *struct_declaration) {
    for (auto &statement: block->statements) {
        switch (statement->type) {
            case NodeType::FUNCTION_DECLARATION:
                checkFunctionStatement(dynamic_cast<FunctionDeclaration *>(statement));
                break;
            case NodeType::VARIABLE_DECLARATION:
                checkVariableDeclaration(dynamic_cast<VariableDeclaration *>(statement), function_declaration,
                                         struct_declaration);
                break;
            case NodeType::FOR_LOOP:
            case NodeType::WHILE_STATEMENT:
            case NodeType::IF_STATEMENT:
                break;
            default:
                checkExpression(statement, function_declaration, struct_declaration);
                break;
        }
    }
}

TypeNode *TypeChecker::checkExpression(Expression *expression, FunctionDeclaration *function_declaration,
                                       StructDeclaration *struct_declaration) {
    switch (expression->type) {
        case NodeType::ASSIGN:
            return checkAssign(dynamic_cast<Assign *>(expression), function_declaration, struct_declaration);
        case NodeType::BINARY:
            return checkBinary(dynamic_cast<Binary *>(expression), function_declaration, struct_declaration);
        case NodeType::LITERAL:
            return checkLiteral(dynamic_cast<Literal *>(expression), function_declaration, struct_declaration);
        case NodeType::CALL:
            return checkCall(dynamic_cast<Call *>(expression), function_declaration, struct_declaration);
        case NodeType::GET:
            return checkGet(dynamic_cast<Get *>(expression), function_declaration, struct_declaration);
        case NodeType::UNARY:
            return checkExpression(dynamic_cast<Unary *>(expression)->expr, function_declaration, struct_declaration);
        case NodeType::GROUPING:
            return checkExpression(dynamic_cast<Grouping *>(expression)->expr, function_declaration,
                                   struct_declaration);
        case NodeType::ARRAY: {
            auto type = checkExpression(dynamic_cast<Array *>(expression)->items.front(), function_declaration,
                                        struct_declaration);
            return type;
        }
        case NodeType::RETURN_STATEMENT:
            return checkExpression(dynamic_cast<ReturnStatement *>(expression)->expr, function_declaration,
                                   struct_declaration);
        default:
            std::cout << expression->toString() << std::endl;
            throw Report("TypeChecker: Unable to check type");
    }
}


bool checkOperators(TokenType left, TokenType right) {
    if (left == TokenType::INT || left == TokenType::LV_INT &&
                                  right == TokenType::INT || right == TokenType::LV_INT) {
        return true;
    }

    if (left == TokenType::CHAR || left == TokenType::LV_CHAR &&
                                   right == TokenType::CHAR || right == TokenType::LV_CHAR) {
        return true;
    }

    if (left == TokenType::STRING || left == TokenType::LV_STRING &&
                                     right == TokenType::STRING || right == TokenType::LV_STRING) {
        return true;
    }

    if (left != right) {
        return false;
    }
}

TypeNode *TypeChecker::checkAssign(Assign *assign, FunctionDeclaration *function_declaration,
                                   StructDeclaration *struct_declaration) {
    auto name = checkExpression(assign->name, function_declaration, struct_declaration);
    auto value = checkExpression(assign->value, function_declaration, struct_declaration);


    if (!checkOperators(name->node_type, value->node_type)) {
        throw Report("Invalid assignment expression");
    }

    return name;
}

TypeNode *TypeChecker::checkBinary(Binary *binary, FunctionDeclaration *function_declaration,
                                   StructDeclaration *struct_declaration) {
    auto left = checkExpression(binary->left, function_declaration, struct_declaration);
    auto right = checkExpression(binary->right, function_declaration, struct_declaration);

    if (!checkOperators(left->node_type, right->node_type)) {
        throw Report("Invalid expression");
    }

    return right;
}

TypeNode *TypeChecker::checkCall(Call *call, FunctionDeclaration *function_declaration,
                                 StructDeclaration *struct_declaration) {
    auto call_node = dynamic_cast<Call *>(call);

    for (const auto &item: call_node->arguments) {
        checkExpression(item, function_declaration, struct_declaration);
    }

    if (call_node->expr->type == NodeType::LITERAL) {
        auto lit_value = dynamic_cast<Literal *>(call_node->expr);

        if (lit_value->literal_type == TokenType::LV_IDENTIFIER) {
            auto data_struct = locateDataStructure(lit_value->literal_value);
            if (data_struct.first) {
                if (data_struct.second->type == NodeType::STRUCT_DECLARATION) {
                    auto struct_ = dynamic_cast<StructDeclaration *>(data_struct.second);
                    call_node->original_struct = dynamic_cast<StructDeclaration *>(data_struct.second);
                    return new TypeNode(TokenType::STRUCT, struct_->name);
                } else if (data_struct.second->type == NodeType::FUNCTION_DECLARATION) {
                    auto func = dynamic_cast<FunctionDeclaration *>(data_struct.second);
                    return new TypeNode(
                            func->return_type.has_value() ? func->return_type.value()->node_type : TokenType::T_EOF,
                            function_declaration->name);
                }
            } else {
                return new TypeNode(TokenType::T_EOF, function_declaration->name);
            }
        }
    }

    throw Report("TypeChecker failed: checkCall()");
}

TypeNode *
TypeChecker::checkGet(Get *get, FunctionDeclaration *function_declaration, StructDeclaration *struct_declaration) {
    if (get->expr->type == NodeType::LITERAL_TOKEN) {
        if (struct_declaration == nullptr) {
            throw Report("`self` may only be used inside of a struct");
        }
        get->expr = new Literal(TokenType::LV_IDENTIFIER, function_declaration->arguments.back()->name, {0, 0});
    }
    if (get->expr->type == NodeType::LITERAL) {
        auto cast = dynamic_cast<Literal *>(get->expr);

        std::string name;
        if (cast->literal_type == TokenType::LV_IDENTIFIER) {
            auto var = locateVariable(cast->literal_value, function_declaration, struct_declaration);

            if (var == nullptr) {
                throw Report(("Cannot find variable: " + cast->literal_value).c_str());
            }
            TypeNode *type;
            if (var->type == NodeType::VARIABLE_DECLARATION) {
                auto var_cast = dynamic_cast<VariableDeclaration *>(var);
                name = var_cast->variable_name;
                if (var_cast->variable_type) {
                    type = dynamic_cast<VariableDeclaration *>(var)->variable_type.value();
                } else {
                    throw Report("TypeChecker: Using variable before definition");
                }
            } else {
                auto arg_cast = dynamic_cast<Argument *>(var);
                type = arg_cast->arg_type;
                name = arg_cast->name;
            }

            if (type->node_type == TokenType::STRUCT) {
                if (get->second->type == NodeType::CALL) {
                    auto cast_call = dynamic_cast<Call *>(get->second);
                    auto literal = new Literal(TokenType::LV_IDENTIFIER, name, {0, 0});
                    cast_call->arguments.push_back(literal);
                }
            }
        }
    }
    return nullptr;
}

TypeNode *TypeChecker::checkLiteral(Literal *literal, FunctionDeclaration *function_declaration,
                                    StructDeclaration *struct_declaration) {
    if (literal->literal_type == TokenType::LV_IDENTIFIER) {
        auto locate = locateVariable(literal->literal_value, function_declaration, struct_declaration);
        if (!locate) {
            if (struct_declaration) {
                auto temp = literal->literal_value;
                auto var = struct_declaration->locate_variable(temp);
                if (var == nullptr) {
                    throw Report(("Cannot find variable" + temp).c_str());
                }
                auto arg_name = function_declaration->arguments.back()->name;
                literal->literal_value = arg_name + "." + temp;
                return new TypeNode(var->variable_type.value()->node_type, arg_name);
            } else {
                throw Report(("Undefined variable: " + literal->literal_value).c_str());
            }
        }

        if (locate->type == NodeType::VARIABLE_DECLARATION) {
            auto cast = dynamic_cast<VariableDeclaration *>(locate);

            return new TypeNode(cast->variable_type.value()->node_type, cast->variable_name);
        } else if (locate->type == NodeType::ARGUMENT) {
            auto cast = dynamic_cast<Argument *>(locate);
            return new TypeNode(cast->arg_type->node_type, cast->name);
        }

        std::cout << "cannot locate: " << literal->literal_value;
        throw Report("TypeChecker failed: checkLiteral()");
    }

    return new TypeNode(literal->literal_type, literal->literal_value);
}

Node *TypeChecker::locateVariable(std::string &variable_name, FunctionDeclaration *function_declaration,
                                  StructDeclaration *struct_declaration) {
    for (auto &stmt: function_declaration->block->statements) {
        if (stmt->type == NodeType::VARIABLE_DECLARATION) {
            auto var = dynamic_cast<VariableDeclaration *>(stmt);
            if (var->variable_name == variable_name) {
                return var;
            }
        }
    }

    for (auto &arg: function_declaration->arguments) {
        if (arg->name == variable_name) {
            return arg;
        }
    }

    return nullptr;
}

std::pair<bool, Node *> TypeChecker::locateDataStructure(std::string &data_structure_name) {
    for (auto &item: root->statements) {
        if (item->type == NodeType::STRUCT_DECLARATION) {
            auto struct_decl = dynamic_cast<StructDeclaration *>(item);
            if (struct_decl->name == data_structure_name) {
                return {true, item};
            }
            continue;
        }

        if (item->type == NodeType::FUNCTION_DECLARATION) {
            auto func_decl = dynamic_cast<FunctionDeclaration *>(item);
            if (func_decl->name == data_structure_name) {
                return {true, item};
            }
            continue;
        }
    }

    return {false, nullptr};
}
