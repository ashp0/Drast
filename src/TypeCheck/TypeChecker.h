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

    void checkBlock(Block *block, FunctionDeclaration *function_declaration,
                    StructDeclaration *struct_declaration = nullptr);

    void checkAssign(Assign *assign, FunctionDeclaration *function_declaration,
                     StructDeclaration *struct_declaration = nullptr);

    void checkExpression(Expression *expression, FunctionDeclaration *function_declaration,
                         StructDeclaration *struct_declaration = nullptr);

    void checkBinary(Binary *binary, FunctionDeclaration *function_declaration,
                     StructDeclaration *struct_declaration = nullptr);

    void checkLiteral(Literal *literal, FunctionDeclaration *function_declaration,
                      StructDeclaration *struct_declaration = nullptr);

    bool locateVariable(const std::string &variable_name, FunctionDeclaration *function_declaration,
                        StructDeclaration *struct_declaration = nullptr);
};

class TypeCheckerBlock {
public:
    Block *block;
    FunctionDeclaration *function_decl;
    bool is_struct = false;
public:
    TypeCheckerBlock(Block *block, FunctionDeclaration *function_decl) : block(block), function_decl(function_decl) {}

    TypeCheckerBlock(Block *block, FunctionDeclaration *function_decl, bool is_struct) : block(block),
                                                                                         function_decl(function_decl),
                                                                                         is_struct(is_struct) {}

    void checkBody();
};


#endif //DRAST_TYPECHECKER_H
