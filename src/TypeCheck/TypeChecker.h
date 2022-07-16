//
// Created by Ashwin Paudel on 2022-07-15.
//

#ifndef DRAST_TYPECHECKER_H
#define DRAST_TYPECHECKER_H

#include "../Node/Node.h"
#include "../Common/FileUtils.h"
#include "../Common/Report.h"
#include <string>
#include <algorithm>

class TypeChecker {
public:
    Compound *root;
public:
    TypeChecker(Compound *root) : root(root) {}

    void check();

    void checkStatements(std::vector<Node *> &statements);

    void checkStatement(Node *statement);

    void checkImportStatement(Import *import);

    void checkStructDeclaration(StructDeclaration *struct_decl);

    void checkFunctionStatement(FunctionDeclaration *function_decl, StructDeclaration *struct_decl = nullptr);

    void checkVariableDeclaration(VariableDeclaration *var_decl, FunctionDeclaration *function_declaration = nullptr,
                                  StructDeclaration *struct_declaration = nullptr);

    void checkBlock(Block *block, FunctionDeclaration *function_declaration,
                    StructDeclaration *struct_declaration = nullptr);

    TypeNode *checkExpression(Expression *expression, FunctionDeclaration *function_declaration,
                              StructDeclaration *struct_declaration = nullptr);

    TypeNode *checkAssign(Assign *assign, FunctionDeclaration *function_declaration,
                          StructDeclaration *struct_declaration = nullptr);

    TypeNode *checkBinary(Binary *binary, FunctionDeclaration *function_declaration,
                          StructDeclaration *struct_declaration = nullptr);

    TypeNode *checkCall(Call *call, FunctionDeclaration *function_declaration,
                        StructDeclaration *struct_declaration = nullptr);

    TypeNode *
    checkGet(Get *get, FunctionDeclaration *function_declaration, StructDeclaration *struct_declaration = nullptr);

    TypeNode *checkLiteral(Literal *literal, FunctionDeclaration *function_declaration,
                           StructDeclaration *struct_declaration = nullptr);

    Node *locateVariable(std::string &variable_name, FunctionDeclaration *function_declaration,
                         StructDeclaration *struct_declaration = nullptr);

    std::pair<bool, Node *> locateDataStructure(std::string &data_structure_name);
};

#endif //DRAST_TYPECHECKER_H
