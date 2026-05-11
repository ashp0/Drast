#pragma once

#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "Ast.h"

namespace drast {

class Codegen final {
public:
    void setNoRuntime(bool value) { noRuntime_ = value; }
    std::string emit(const Program &program);

private:
    std::ostringstream out_;
    std::set<std::string> imports_;
    int indent_ = 0;
    bool runtimeNeeded_ = false;
    bool noRuntime_ = false;
    bool emittingCallee_ = false;

    // Tracks struct/enum/protocol names → known user types (for ctor disambiguation).
    std::set<std::string> userTypes_;
    std::set<std::string> protocolNames_;
    // Nested enum names: "Outer.Inner" → owned by Outer.
    std::vector<EnumDecl> nestedEnums_;
    // Map enumName → fully-qualified C++ name.
    std::unordered_map<std::string, std::string> enumQualifiedName_;
    std::unordered_map<std::string, std::set<std::string>> enumVariants_;
    std::unordered_map<std::string, std::string> variantToEnum_;
    std::set<std::string> dataEnumNames_;
    std::unordered_map<std::string, std::unordered_map<std::string, std::vector<StructField>>> dataEnumFields_;
    std::unordered_map<std::string, std::set<std::string>> boxedStructFields_;
    std::unordered_map<std::string, std::unordered_map<std::string, std::set<std::string>>> boxedEnumFields_;
    std::unordered_set<std::string> functionNames_;
    std::unordered_map<std::string, std::string> functionReturnTypes_;

    struct FieldInfo {
        std::string storageName;
        std::string typeName;
        bool isPreview = false;
        bool isPrivate = false;
        bool isAutoBoxed = false;
    };
    struct MethodRef {
        size_t implIndex = 0;
        size_t methodIndex = 0;
    };
    std::unordered_map<std::string, std::unordered_map<std::string, FieldInfo>> structFields_;
    std::unordered_map<std::string, std::vector<MethodRef>> methodsByStruct_;
    std::unordered_map<std::string, std::set<std::string>> methodNamesByStruct_;

    // Per-method emission state — current impl-target host type, used to
    // rewrite bare identifier calls in method bodies into `this->name(...)`.
    std::string currentImplTarget_;
    std::set<std::string> currentImplMethods_;

    // For operator bodies — positional arg names (;1, ;2 → param[0], param[1]).
    std::vector<std::string> positionalParamNames_;

    // Stack of declared names per nested function/method scope. A redeclaration
    // (`name = expr` where `name` already exists) is emitted as a re-assignment.
    std::vector<std::set<std::string>> localScopes_;
    std::vector<std::unordered_map<std::string, std::string>> localTypeScopes_;

    void emitUses(const Program &program);
    bool detectRuntimeNeed(const Program &program) const;
    void emitForwardDecls(const Program &program);
    void emitProtocol(const ProtocolDecl &p);
    void emitStruct(const Program &program, const StructDecl &s);
    void emitNestedEnums(const StructDecl &s);
    void emitDataEnum(const EnumDecl &e);
    void emitGlobalVar(const Stmt &stmt);
    void emitImpl(const ImplBlock &impl);
    void emitFreeEnum(const EnumDecl &e);
    void emitFunction(const Function &function);
    void emitMethodDefinition(const Function &function);

    void emitStmt(const Stmt &stmt);
    void emitBlock(const std::vector<StmtPtr> &block);
    void preScanBlockTypeHints(const std::vector<StmtPtr> &block);
    void emitIfChain(const Stmt &stmt);
    void emitForIn(const Stmt &stmt);
    void emitMatch(const Stmt &stmt);
    bool emitDataEnumMatch(const Stmt &stmt);
    void emitTryCatch(const Stmt &stmt);

    void emitExpr(const Expr &expr);
    void emitCall(const Expr &call);
    void emitConstructorCall(const Expr &call);
    void emitBracketCallBatch(const Expr &call);

    std::string typeText(const Type &type) const;
    std::string boxedTypeText(const Type &type) const;
    std::string typeTextMaybeOptional(const Type &type) const;
    std::string nameForBuiltinType(const std::string &name) const;
    std::string functionReturnText(const Function &fn);
    std::string parameterListText(const std::vector<Parameter> &params,
                                  bool emitDefaults);

    void writeIndent();
    bool hasUse(const std::string &path) const;
    bool isKnownFunction(const std::string &name) const;
    bool isKnownTypeName(const std::string &name) const;
    bool isVectorType(const std::string &type) const;
    bool isOptionalType(const std::string &type) const;
    bool isHeapPointerType(const std::string &type) const;
    std::string pointeeType(const std::string &type) const;
    bool isDataEnumVariant(const std::string &enumName, const std::string &variantName) const;
    bool isBuiltinTypeName(const std::string &name) const;
    bool exprIsStringLike(const Expr &expr) const;
    bool exprIsIntLike(const Expr &expr) const;
    bool exprIsTypeIdentifier(const Expr &expr) const;
    bool isMethodName(const std::string &typeName, const std::string &name) const;
    void computeAutoBoxing(const Program &program);
    bool isAutoBoxedStructField(const std::string &owner, const std::string &field) const;
    bool isAutoBoxedEnumField(const std::string &owner, const std::string &variant,
                              const std::string &field) const;
    std::string fieldStorageType(const std::string &owner, const StructField &field) const;
    std::string enumFieldStorageType(const std::string &owner, const std::string &variant,
                                     const StructField &field) const;
    std::vector<std::string> userTypeRefsIn(const Type &type) const;
    std::string lookupVarType(const std::string &name) const;
    std::string inferExprType(const Expr &expr) const;
    std::string inferArrayElementType(const Expr &expr) const;
    std::string vectorTypeForArrayLiteral(const Expr &expr) const;
    std::string storageNameForField(const std::string &field) const;
    void rememberLocalType(const std::string &name, const std::string &type);
    void emitTemplatePrefix(const std::vector<std::string> &typeParams);
    void emitGenericArgs(const std::vector<Type> &typeArgs);

    static std::string escapeStringLiteral(const std::string &text);
    static std::string escapeCharLiteral(const std::string &text);
    static std::string qualifyDottedName(const std::string &name);

    // Mangle operator symbols for C++ identifier suffixes when needed.
    static std::string operatorSpelling(const std::string &symbol);
};

} // namespace drast
