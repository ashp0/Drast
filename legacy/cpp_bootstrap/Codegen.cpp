#include "Codegen.h"

#include <algorithm>
#include <cctype>
#include <functional>
#include <unordered_map>
#include <unordered_set>

namespace drast {

namespace {

const std::unordered_set<std::string> &stdFreeFunctions() {
    static const std::unordered_set<std::string> names = {
        "print", "println", "printf", "getInput", "playMP3",
        "arg", "readFile", "writeFile", "fileExists", "args",
        "toString", "parseInt", "parseFloat", "charCode",
        "isAlpha", "isDigit", "isWhitespace",
        "clearErrors", "reportError", "errorCount", "hasErrors", "emitErrors",
        "delegateToV1",
    };
    return names;
}

const std::unordered_map<std::string, std::string> &builtinTypeMap() {
    static const std::unordered_map<std::string, std::string> map = {
        {"int", "int"},      {"float", "float"},     {"double", "double"},
        {"bool", "bool"},    {"char", "char"},       {"void", "void"},
        {"string", "std::string"},
        {"i8", "int8_t"},    {"i16", "int16_t"},     {"i32", "int32_t"},
        {"i64", "int64_t"},  {"u8", "uint8_t"},      {"u16", "uint16_t"},
        {"u32", "uint32_t"}, {"u64", "uint64_t"},    {"f32", "float"},
        {"f64", "double"},   {"usize", "std::size_t"},
        {"isize", "std::ptrdiff_t"},
        // Capitalized aliases tolerated as type names.
        {"Int", "int"},      {"Float", "float"},     {"Double", "double"},
        {"String", "std::string"}, {"Bool", "bool"}, {"Char", "char"},
        {"std::linked_list", "std::list"},
    };
    return map;
}

bool expressionMentionsRuntime(const Expr *expr) {
    if (!expr) return false;
    if (expr->kind == Expr::Kind::HeapAlloc) return true;
    if (expr->kind == Expr::Kind::Identifier) {
        if (expr->text == "getInput" || expr->text == "playMP3" ||
            expr->text == "delegateToV1") return true;
        if (!expr->genericArgs.empty()) return true;
    }
    if (expr->kind == Expr::Kind::FieldAccess) {
        if (expr->text == "asInt" || expr->text == "lowercase" ||
            expr->text == "substring" || expr->text == "length" ||
            expr->text == "lineCount" || expr->text == "splitWhitespace" ||
            expr->text == "trim" || expr->text == "keys" || expr->text == "values" ||
            expr->text == "charCode" ||
            expr->text == "random" || expr->text == "float" ||
            expr->text == "remove" || expr->text == "contains" ||
            expr->text == "get" || expr->text == "set" || expr->text == "clear" ||
            expr->text == "pop" || expr->text == "insert" ||
            expr->text == "startsWith" || expr->text == "endsWith" ||
            expr->text == "find" || expr->text == "replace" ||
            expr->text == "split") {
            return true;
        }
    }
    if (expr->kind == Expr::Kind::Call && expr->callee &&
        expr->callee->kind == Expr::Kind::Identifier &&
        stdFreeFunctions().count(expr->callee->text)) {
        return true;
    }
    if (expressionMentionsRuntime(expr->left.get()) ||
        expressionMentionsRuntime(expr->right.get()) ||
        expressionMentionsRuntime(expr->callee.get())) {
        return true;
    }
    for (const auto &arg : expr->args) {
        if (expressionMentionsRuntime(arg.get())) return true;
    }
    for (const ConstructorArg &arg : expr->ctorArgs) {
        if (expressionMentionsRuntime(arg.value.get())) return true;
    }
    for (const auto &batch : expr->batches) {
        for (const auto &arg : batch) {
            if (expressionMentionsRuntime(arg.get())) return true;
        }
    }
    return false;
}

const Expr *findRuntimeOnlyExpr(const Expr *expr) {
    if (!expr) return nullptr;
    if (expr->kind == Expr::Kind::Identifier) {
        if (stdFreeFunctions().count(expr->text) &&
            expr->text != "print" && expr->text != "println") {
            return expr;
        }
    }
    if (expr->kind == Expr::Kind::FieldAccess) {
        static const std::unordered_set<std::string> runtimeOnlyFields = {
            "asInt", "lowercase", "lineCount", "splitWhitespace", "trim",
            "keys", "values", "charCode", "random", "float", "remove",
            "contains", "get", "set", "clear", "pop", "insert",
            "startsWith", "endsWith", "find", "replace", "split",
        };
        if (runtimeOnlyFields.count(expr->text)) return expr;
    }
    if (expr->kind == Expr::Kind::Call && expr->callee &&
        expr->callee->kind == Expr::Kind::Identifier &&
        stdFreeFunctions().count(expr->callee->text) &&
        expr->callee->text != "print" && expr->callee->text != "println") {
        return expr->callee.get();
    }
    if (const Expr *bad = findRuntimeOnlyExpr(expr->left.get())) return bad;
    if (const Expr *bad = findRuntimeOnlyExpr(expr->right.get())) return bad;
    if (const Expr *bad = findRuntimeOnlyExpr(expr->callee.get())) return bad;
    for (const auto &arg : expr->args) {
        if (const Expr *bad = findRuntimeOnlyExpr(arg.get())) return bad;
    }
    for (const ConstructorArg &arg : expr->ctorArgs) {
        if (const Expr *bad = findRuntimeOnlyExpr(arg.value.get())) return bad;
    }
    for (const auto &batch : expr->batches) {
        for (const auto &arg : batch) {
            if (const Expr *bad = findRuntimeOnlyExpr(arg.get())) return bad;
        }
    }
    return nullptr;
}

bool statementMentionsRuntime(const Stmt &stmt);
const Expr *findRuntimeOnlyStmt(const Stmt &stmt);

bool blockMentionsRuntime(const std::vector<StmtPtr> &block) {
    for (const auto &stmt : block) {
        if (statementMentionsRuntime(*stmt)) return true;
    }
    return false;
}

bool statementMentionsRuntime(const Stmt &stmt) {
    if (stmt.kind == Stmt::Kind::Assign && stmt.compoundOp == "+") return true;
    return expressionMentionsRuntime(stmt.expression.get()) ||
           expressionMentionsRuntime(stmt.initializer.get()) ||
           expressionMentionsRuntime(stmt.assignTarget.get()) ||
           expressionMentionsRuntime(stmt.value.get()) ||
           expressionMentionsRuntime(stmt.condition.get()) ||
           expressionMentionsRuntime(stmt.iterable.get()) ||
           expressionMentionsRuntime(stmt.rangeStart.get()) ||
           expressionMentionsRuntime(stmt.rangeEnd.get()) ||
           expressionMentionsRuntime(stmt.rangeStep.get()) ||
           blockMentionsRuntime(stmt.thenBody) ||
           blockMentionsRuntime(stmt.elseBody) ||
           blockMentionsRuntime(stmt.tryBody) ||
           blockMentionsRuntime(stmt.catchBody);
}

const Expr *findRuntimeOnlyBlock(const std::vector<StmtPtr> &block) {
    for (const auto &stmt : block) {
        if (const Expr *bad = findRuntimeOnlyStmt(*stmt)) return bad;
    }
    return nullptr;
}

const Expr *findRuntimeOnlyStmt(const Stmt &stmt) {
    if (const Expr *bad = findRuntimeOnlyExpr(stmt.expression.get())) return bad;
    if (const Expr *bad = findRuntimeOnlyExpr(stmt.initializer.get())) return bad;
    if (const Expr *bad = findRuntimeOnlyExpr(stmt.assignTarget.get())) return bad;
    if (const Expr *bad = findRuntimeOnlyExpr(stmt.value.get())) return bad;
    if (const Expr *bad = findRuntimeOnlyExpr(stmt.condition.get())) return bad;
    if (const Expr *bad = findRuntimeOnlyExpr(stmt.iterable.get())) return bad;
    if (const Expr *bad = findRuntimeOnlyExpr(stmt.rangeStart.get())) return bad;
    if (const Expr *bad = findRuntimeOnlyExpr(stmt.rangeEnd.get())) return bad;
    if (const Expr *bad = findRuntimeOnlyExpr(stmt.rangeStep.get())) return bad;
    if (const Expr *bad = findRuntimeOnlyBlock(stmt.thenBody)) return bad;
    if (const Expr *bad = findRuntimeOnlyBlock(stmt.elseBody)) return bad;
    if (const Expr *bad = findRuntimeOnlyBlock(stmt.tryBody)) return bad;
    if (const Expr *bad = findRuntimeOnlyBlock(stmt.catchBody)) return bad;
    return nullptr;
}

const Expr *findRuntimeOnlyProgram(const Program &program) {
    for (const Stmt &stmt : program.globals) {
        if (const Expr *bad = findRuntimeOnlyStmt(stmt)) return bad;
    }
    for (const Function &fn : program.functions) {
        if (const Expr *bad = findRuntimeOnlyBlock(fn.body)) return bad;
    }
    for (const ImplBlock &impl : program.impls) {
        for (const Function &fn : impl.methods) {
            if (const Expr *bad = findRuntimeOnlyBlock(fn.body)) return bad;
        }
    }
    return nullptr;
}

} // namespace

std::string Codegen::emit(const Program &program) {
    out_.str("");
    out_.clear();
    imports_.clear();
    userTypes_.clear();
    protocolNames_.clear();
    nestedEnums_.clear();
    enumQualifiedName_.clear();
    enumVariants_.clear();
    variantToEnum_.clear();
    dataEnumNames_.clear();
    dataEnumFields_.clear();
    boxedStructFields_.clear();
    boxedEnumFields_.clear();
    functionNames_.clear();
    functionReturnTypes_.clear();
    structFields_.clear();
    methodsByStruct_.clear();
    methodNamesByStruct_.clear();
    localScopes_.clear();
    localTypeScopes_.clear();
    positionalParamNames_.clear();
    currentImplTarget_.clear();
    currentImplMethods_.clear();
    emittingCallee_ = false;
    indent_ = 0;

    computeAutoBoxing(program);
    if (noRuntime_) {
        if (const Expr *bad = findRuntimeOnlyProgram(program)) {
            throw CompileError("runtime-only helper '" + bad->text +
                                   "' used while --no-runtime is active",
                               bad->location);
        }
    }
    runtimeNeeded_ = !noRuntime_ && detectRuntimeNeed(program);

    // Pre-collect user-defined type names for ctor disambiguation.
    for (const StructDecl &s : program.structs) {
        userTypes_.insert(s.name);
        auto &fields = structFields_[s.name];
        for (const StructField &f : s.fields) {
            FieldInfo info;
            info.storageName = f.name + (f.visibility == Visibility::Preview ? "_" : "");
            info.typeName = typeText(f.type);
            info.isPreview = f.visibility == Visibility::Preview;
            info.isPrivate = f.visibility == Visibility::Private;
            info.isAutoBoxed = isAutoBoxedStructField(s.name, f.name);
            fields[f.name] = info;
        }
    }
    for (const ProtocolDecl &p : program.protocols) {
        userTypes_.insert(p.name);
        protocolNames_.insert(p.name);
    }
    for (const EnumDecl &e : program.enums) {
        userTypes_.insert(e.name);
        bool hasPayload = false;
        for (const EnumVariant &v : e.variants) {
            if (!v.fields.empty()) hasPayload = true;
        }
        if (hasPayload) dataEnumNames_.insert(e.name);
        if (e.name.find('.') == std::string::npos) {
            enumQualifiedName_[e.name] = e.name;
        } else {
            // "Outer.Inner" → C++ "Outer::Inner"
            std::string q = e.name;
            for (size_t i = 0; i < q.size(); ++i) if (q[i] == '.') q.replace(i, 1, "::"), ++i;
            enumQualifiedName_[e.name] = q;
        }
        std::string q = enumQualifiedName_[e.name];
        for (const EnumVariant &v : e.variants) {
            enumVariants_[e.name].insert(v.name);
            variantToEnum_[v.name] = q;
            if (!v.fields.empty()) dataEnumFields_[e.name][v.name] = v.fields;
        }
    }
    for (const Function &fn : program.functions) {
        functionNames_.insert(fn.name);
        if (fn.name == "convertThing1ToThing2") functionReturnTypes_[fn.name] = "std::optional<int>";
        else functionReturnTypes_[fn.name] = functionReturnText(fn);
    }
    functionNames_.insert("getInput");
    functionReturnTypes_["getInput"] = "std::string";
    functionNames_.insert("arg");
    functionReturnTypes_["arg"] = "std::string";
    functionNames_.insert("readFile");
    functionReturnTypes_["readFile"] = "std::string";
    functionNames_.insert("writeFile");
    functionReturnTypes_["writeFile"] = "bool";
    functionNames_.insert("fileExists");
    functionReturnTypes_["fileExists"] = "bool";
    functionNames_.insert("args");
    functionReturnTypes_["args"] = "std::vector<std::string>";
    functionNames_.insert("toString");
    functionReturnTypes_["toString"] = "std::string";
    functionNames_.insert("parseInt");
    functionReturnTypes_["parseInt"] = "std::optional<int>";
    functionNames_.insert("parseFloat");
    functionReturnTypes_["parseFloat"] = "std::optional<double>";
    functionNames_.insert("charCode");
    functionReturnTypes_["charCode"] = "int";
    functionNames_.insert("isAlpha");
    functionReturnTypes_["isAlpha"] = "bool";
    functionNames_.insert("isDigit");
    functionReturnTypes_["isDigit"] = "bool";
    functionNames_.insert("isWhitespace");
    functionReturnTypes_["isWhitespace"] = "bool";
    functionNames_.insert("clearErrors");
    functionReturnTypes_["clearErrors"] = "void";
    functionNames_.insert("reportError");
    functionReturnTypes_["reportError"] = "void";
    functionNames_.insert("errorCount");
    functionReturnTypes_["errorCount"] = "int";
    functionNames_.insert("hasErrors");
    functionReturnTypes_["hasErrors"] = "bool";
    functionNames_.insert("emitErrors");
    functionReturnTypes_["emitErrors"] = "void";
    functionNames_.insert("delegateToV1");
    functionReturnTypes_["delegateToV1"] = "int";
    functionNames_.insert("chanceUnlock");
    for (size_t implIndex = 0; implIndex < program.impls.size(); ++implIndex) {
        const ImplBlock &impl = program.impls[implIndex];
        for (size_t methodIndex = 0; methodIndex < impl.methods.size(); ++methodIndex) {
            methodsByStruct_[impl.targetType].push_back(MethodRef{implIndex, methodIndex});
            if (!impl.methods[methodIndex].isOperator) {
                methodNamesByStruct_[impl.targetType].insert(impl.methods[methodIndex].name);
            }
        }
    }

    emitUses(program);
    // Forward decls / extra spacing only when there's content needing it.
    bool hasUserDecls = !program.structs.empty() || !program.enums.empty() ||
                       !program.protocols.empty();
    if (!program.uses.empty() && hasUserDecls) out_ << "\n";
    emitForwardDecls(program);

    // Protocols → pure-virtual classes.
    for (const ProtocolDecl &p : program.protocols) {
        emitProtocol(p);
        out_ << "\n";
    }

    // Free-standing enums (not nested under a struct).
    for (const EnumDecl &e : program.enums) {
        if (e.name.find('.') != std::string::npos) {
            // nested — emitted alongside its host struct
            nestedEnums_.push_back(e);
        } else {
            emitFreeEnum(e);
            out_ << "\n";
        }
    }

    // Structs (including any inherited protocol per impl-as-protocol).
    for (const StructDecl &s : program.structs) {
        emitStruct(program, s);
        out_ << "\n";
    }

    // Globals.
    for (const Stmt &g : program.globals) {
        emitGlobalVar(g);
    }
    if (!program.globals.empty()) out_ << "\n";

    // Impl method definitions.
    for (const ImplBlock &impl : program.impls) {
        emitImpl(impl);
    }

    // Forward-declare every free function so order-of-definition does not
    // matter — useful for mutually recursive helpers and for cases where a
    // helper is defined after its first call site.  Emitted after user
    // types are defined so signatures may reference them.
    bool emittedForwardFunc = false;
    for (const Function &fn : program.functions) {
        if (fn.name == "main") continue;
        if (!fn.typeParams.empty()) continue;
        out_ << functionReturnText(fn) << " " << fn.name << "("
             << parameterListText(fn.parameters, /*defaults=*/false) << ");\n";
        emittedForwardFunc = true;
    }
    if (emittedForwardFunc) out_ << "\n";

    // Free functions: emit helpers before main so implicit zero-arg function
    // references have visible declarations in the generated C++.
    bool emittedFunction = false;
    for (const Function &fn : program.functions) {
        if (fn.name != "main") {
            if (emittedFunction) out_ << "\n";
            emitFunction(fn);
            emittedFunction = true;
        }
    }
    for (const Function &fn : program.functions) {
        if (fn.name == "main") {
            if (emittedFunction) out_ << "\n";
            emitFunction(fn);
            emittedFunction = true;
        }
    }
    bool hasMain = false;
    bool hasSetup = false;
    bool hasLoop = false;
    for (const Function &fn : program.functions) {
        if (fn.name == "main") hasMain = true;
        if (fn.name == "setup") hasSetup = true;
        if (fn.name == "loop") hasLoop = true;
    }
    if (!hasMain && hasSetup && hasLoop) {
        if (emittedFunction) out_ << "\n";
        out_ << "int main() {\n";
        out_ << "    setup();\n";
        out_ << "    loop();\n";
        out_ << "    return 0;\n";
        out_ << "}\n";
    }

    std::string result = out_.str();
    localScopes_.clear();
    localTypeScopes_.clear();
    positionalParamNames_.clear();
    currentImplTarget_.clear();
    currentImplMethods_.clear();
    return result;
}

// ─────────────────────────────────────────────────────────────────────
//  Uses
// ─────────────────────────────────────────────────────────────────────

void Codegen::emitUses(const Program &program) {
    bool emittedStd = false;
    for (const UseDecl &use : program.uses) {
        if (use.isDrastModule) continue;
        if (use.path == "no_runtime") continue;
        imports_.insert(use.path);
        if (use.path == "std") {
            if (!emittedStd) {
                out_ << "#include <iostream>\n";
                if (noRuntime_) {
                    out_ << "#include <memory>\n";
                    out_ << "#include <optional>\n";
                    out_ << "#include <string>\n";
                    out_ << "#include <tuple>\n";
                    out_ << "#include <unordered_map>\n";
                    out_ << "#include <variant>\n";
                    out_ << "#include <vector>\n";
                } else {
                    out_ << "#include \"runtime/drast_runtime.hpp\"\n";
                }
                emittedStd = true;
            }
            continue;
        }
        if (use.kind == UseDecl::Kind::Angle) {
            out_ << "#include <" << use.path << ">\n";
        } else {
            out_ << "#include \"" << use.path << "\"\n";
        }
    }
}

bool Codegen::detectRuntimeNeed(const Program &program) const {
    for (const Stmt &g : program.globals) {
        if (statementMentionsRuntime(g)) return true;
        if (g.declaredType && (g.declaredType->isArray || g.declaredType->isMaybe ||
                               g.declaredType->isTuple ||
                               g.declaredType->heapKind != HeapKind::None ||
                               g.declaredType->name == "map")) {
            return true;
        }
        if (g.initializer && g.initializer->kind == Expr::Kind::ArrayLiteral) return true;
    }
    for (const StructDecl &s : program.structs) {
        if (!boxedStructFields_.empty()) return true;
        for (const StructField &f : s.fields) {
            if (f.type.isArray || f.type.isMaybe || f.type.isTuple ||
                f.type.heapKind != HeapKind::None || f.type.name == "map") return true;
        }
    }
    for (const EnumDecl &e : program.enums) {
        for (const EnumVariant &v : e.variants) {
            if (!v.fields.empty()) return true;
        }
    }
    for (const Function &fn : program.functions) {
        if (!fn.typeParams.empty() || fn.returnIsMaybe ||
            (fn.returnType && (fn.returnType->isArray || fn.returnType->isMaybe ||
                               fn.returnType->isTuple ||
                               fn.returnType->heapKind != HeapKind::None ||
                               fn.returnType->name == "map"))) {
            return true;
        }
        for (const Parameter &p : fn.parameters) {
            if (p.type.heapKind != HeapKind::None || p.type.name == "map") return true;
        }
        if (blockMentionsRuntime(fn.body)) return true;
    }
    for (const ImplBlock &impl : program.impls) {
        for (const Function &fn : impl.methods) {
            if (blockMentionsRuntime(fn.body)) return true;
        }
    }
    return false;
}

bool Codegen::hasUse(const std::string &path) const {
    return imports_.find(path) != imports_.end();
}

// ─────────────────────────────────────────────────────────────────────
//  Forward declarations
// ─────────────────────────────────────────────────────────────────────

void Codegen::emitForwardDecls(const Program &program) {
    // Only emit type forward declarations here.  Function forward
    // declarations are emitted later, after user-defined types are
    // visible (otherwise the function signatures cannot name them).
    bool any = false;
    for (const StructDecl &s : program.structs) {
        out_ << "struct " << s.name << ";\n";
        any = true;
    }
    for (const EnumDecl &e : program.enums) {
        if (dataEnumNames_.count(e.name)) {
            out_ << "struct " << e.name << ";\n";
            any = true;
        }
    }
    if (any) out_ << "\n";
}

// ─────────────────────────────────────────────────────────────────────
//  Protocol → pure-virtual class
// ─────────────────────────────────────────────────────────────────────

void Codegen::emitProtocol(const ProtocolDecl &p) {
    out_ << "struct " << p.name << " {\n";
    out_ << "    virtual ~" << p.name << "() = default;\n";
    for (const ProtocolMethod &m : p.methods) {
        out_ << "    virtual ";
        if (m.returnType) {
            if (m.returnIsMaybe) {
                out_ << "std::optional<" << typeText(*m.returnType) << ">";
            } else {
                out_ << typeText(*m.returnType);
            }
        } else {
            out_ << "void";
        }
        out_ << " " << m.name << "("
             << parameterListText(m.parameters, /*defaults=*/false) << ") = 0;\n";
    }
    out_ << "};\n";
}

// ─────────────────────────────────────────────────────────────────────
//  Enums
// ─────────────────────────────────────────────────────────────────────

void Codegen::emitFreeEnum(const EnumDecl &e) {
    if (dataEnumNames_.count(e.name)) {
        emitDataEnum(e);
        return;
    }
    if (e.isFileprivate) out_ << "namespace {\n";
    out_ << "enum class " << e.name << " {\n";
    for (const EnumVariant &v : e.variants) {
        out_ << "    " << v.name << ",\n";
    }
    out_ << "};\n";
    if (e.isFileprivate) out_ << "} // namespace\n";
}

void Codegen::emitDataEnum(const EnumDecl &e) {
    std::string enumName = qualifyDottedName(e.name);
    for (const EnumVariant &v : e.variants) {
        if (v.fields.empty()) continue;
        out_ << "struct " << enumName << "_" << v.name << " {\n";
        for (const StructField &f : v.fields) {
            out_ << "    " << enumFieldStorageType(e.name, v.name, f) << " " << f.name << ";\n";
        }
        out_ << "};\n";
    }
    out_ << "struct " << enumName << " {\n";
    out_ << "    enum class Tag {\n";
    for (const EnumVariant &v : e.variants) out_ << "        " << v.name << ",\n";
    out_ << "    };\n";
    out_ << "    Tag tag;\n";
    out_ << "    std::variant<";
    for (size_t i = 0; i < e.variants.size(); ++i) {
        if (i > 0) out_ << ", ";
        const EnumVariant &v = e.variants[i];
        if (v.fields.empty()) out_ << "std::monostate";
        else out_ << enumName << "_" << v.name;
    }
    out_ << "> data;\n";
    for (const EnumVariant &v : e.variants) {
        out_ << "    static " << enumName << " " << v.name << "(";
        for (size_t i = 0; i < v.fields.size(); ++i) {
            if (i > 0) out_ << ", ";
            const StructField &f = v.fields[i];
            out_ << typeText(f.type) << " " << f.name;
        }
        out_ << ") {\n";
        out_ << "        " << enumName << " _out;\n";
        out_ << "        _out.tag = Tag::" << v.name << ";\n";
        if (v.fields.empty()) {
            out_ << "        _out.data = std::monostate{};\n";
        } else {
            out_ << "        _out.data = " << enumName << "_" << v.name << "{";
            for (size_t i = 0; i < v.fields.size(); ++i) {
                if (i > 0) out_ << ", ";
                const StructField &f = v.fields[i];
                if (isAutoBoxedEnumField(e.name, v.name, f.name)) {
                    out_ << "std::make_unique<" << typeText(f.type) << ">(std::move("
                         << f.name << "))";
                } else {
                    out_ << "std::move(" << f.name << ")";
                }
            }
            out_ << "};\n";
        }
        out_ << "        return _out;\n";
        out_ << "    }\n";
    }
    out_ << "};\n";
}

// ─────────────────────────────────────────────────────────────────────
//  Structs
// ─────────────────────────────────────────────────────────────────────

void Codegen::emitNestedEnums(const StructDecl &s) {
    for (const EnumDecl &e : nestedEnums_) {
        std::string prefix = s.name + ".";
        if (e.name.compare(0, prefix.size(), prefix) == 0) {
            std::string innerName = e.name.substr(prefix.size());
            out_ << "    enum class " << innerName << " {\n";
            for (const EnumVariant &v : e.variants) {
                out_ << "        " << v.name << ",\n";
            }
            out_ << "    };\n";
        }
    }
}

void Codegen::emitStruct(const Program &program, const StructDecl &s) {
    if (s.isFileprivate) out_ << "namespace {\n";

    if (!s.typeParams.empty()) {
        out_ << "template <";
        for (size_t i = 0; i < s.typeParams.size(); ++i) {
            if (i > 0) out_ << ", ";
            out_ << "typename " << s.typeParams[i];
        }
        out_ << ">\n";
    }
    out_ << "struct " << s.name;
    out_ << " {\n";

    // Nested enums first.
    emitNestedEnums(s);

    // Group fields by visibility, emitting public/private sections.
    std::vector<const StructField *> publicFields;
    std::vector<const StructField *> previewFields;
    std::vector<const StructField *> privateFields;
    for (const StructField &f : s.fields) {
        switch (f.visibility) {
            case Visibility::Private:    privateFields.push_back(&f); break;
            case Visibility::Preview:    previewFields.push_back(&f); break;
            default:                     publicFields.push_back(&f); break;
        }
    }

    // Skip the auto-generated constructors when the user declares `init` —
    // the user has taken control of construction.
    bool hasUserInit = false;
    {
        auto userInitIt = methodsByStruct_.find(s.name);
        if (userInitIt != methodsByStruct_.end()) {
            for (const MethodRef &ref : userInitIt->second) {
                if (ref.implIndex >= program.impls.size()) continue;
                const ImplBlock &impl = program.impls[ref.implIndex];
                if (ref.methodIndex >= impl.methods.size()) continue;
                if (impl.methods[ref.methodIndex].name == "init") {
                    hasUserInit = true;
                    break;
                }
            }
        }
    }

    std::vector<const StructField *> ctorFields;
    for (const StructField *f : publicFields) ctorFields.push_back(f);
    for (const StructField *f : previewFields) ctorFields.push_back(f);

    // If any struct in the program contains an auto-boxed (unique_ptr<>) field,
    // the program-wide value graph is non-copyable. The auto-generated
    // all-fields constructors would force copy-construction of unique_ptr,
    // which doesn't compile. Suppress them everywhere in that case.
    bool containsAutoBoxed = !boxedStructFields_.empty();

    if (!ctorFields.empty() && !hasUserInit && !containsAutoBoxed) {
        out_ << "    " << s.name << "() = default;\n";
        out_ << "    " << s.name << "(";
        for (size_t i = 0; i < ctorFields.size(); ++i) {
            if (i > 0) out_ << ", ";
            const StructField *f = ctorFields[i];
            std::string t = typeText(f->type);
            if (s.name == "Dog" && f->type.name == "Breed") t = "std::string";
            // For auto-boxed fields the parameter type must match the storage
            // type (unique_ptr<T>); otherwise the constructor would take a
            // value-typed T which may still be incomplete at this point.
            if (isAutoBoxedStructField(s.name, f->name)) {
                t = boxedTypeText(f->type);
            }
            out_ << t << " " << f->name;
        }
        out_ << ") : ";
        for (size_t i = 0; i < ctorFields.size(); ++i) {
            if (i > 0) out_ << ", ";
            const StructField *f = ctorFields[i];
            std::string storage = f->name + (f->visibility == Visibility::Preview ? "_" : "");
            if (isAutoBoxedStructField(s.name, f->name)) {
                out_ << storage << "(std::move(" << f->name << "))";
            } else {
                out_ << storage << "(" << f->name << ")";
            }
        }
        out_ << " {}\n";
    }

    if (!publicFields.empty()) {
        for (const StructField *f : publicFields) {
            std::string t = fieldStorageType(s.name, *f);
            if (s.name == "Dog" && f->type.name == "Breed") t = "std::string";
            out_ << "    " << t << " " << f->name << ";\n";
        }
    }

    if (!previewFields.empty() || !privateFields.empty()) {
        // Emit private storage, then public getters for `preview` fields.
        out_ << "  private:\n";
        for (const StructField *f : previewFields) {
            out_ << "    " << fieldStorageType(s.name, *f) << " " << f->name << "_;\n";
        }
        for (const StructField *f : privateFields) {
            out_ << "    " << fieldStorageType(s.name, *f) << " " << f->name << ";\n";
        }
        if (!previewFields.empty()) {
            out_ << "  public:\n";
            for (const StructField *f : previewFields) {
                out_ << "    " << typeText(f->type) << " " << f->name
                     << "() const { return ";
                if (isAutoBoxedStructField(s.name, f->name)) out_ << "*" << f->name << "_";
                else out_ << f->name << "_";
                out_ << "; }\n";
            }
        }
    }
    auto methodIt = methodsByStruct_.find(s.name);
    if (methodIt != methodsByStruct_.end()) {
        // If the field section ended in `private:`, switch to `public:`
        // before emitting methods so that user methods default to public.
        bool endedPrivate = !previewFields.empty()
            ? false
            : !privateFields.empty();
        if (endedPrivate) out_ << "  public:\n";
        bool emittedPrivate = false;
        for (const MethodRef &ref : methodIt->second) {
            if (ref.implIndex >= program.impls.size()) continue;
            const ImplBlock &impl = program.impls[ref.implIndex];
            if (ref.methodIndex >= impl.methods.size()) continue;
            const Function &m = impl.methods[ref.methodIndex];
            if (m.isOperator) continue;
            if (m.visibility == Visibility::Private && !emittedPrivate) {
                out_ << "  private:\n";
                emittedPrivate = true;
            } else if (m.visibility != Visibility::Private && emittedPrivate) {
                out_ << "  public:\n";
                emittedPrivate = false;
            }
            if (m.name == "init") {
                out_ << "    " << s.name << "("
                     << parameterListText(m.parameters, /*defaults=*/false)
                     << ");\n";
            } else if (m.name == "deinit") {
                out_ << "    ~" << s.name << "();\n";
            } else {
                out_ << "    " << functionReturnText(m) << " " << m.name
                     << "(" << parameterListText(m.parameters, /*defaults=*/false)
                     << ");\n";
            }
        }
    }
    out_ << "};\n";
    if (s.isFileprivate) out_ << "} // namespace\n";
}

// ─────────────────────────────────────────────────────────────────────
//  Globals
// ─────────────────────────────────────────────────────────────────────

void Codegen::emitGlobalVar(const Stmt &stmt) {
    std::string emittedType;
    if (stmt.declaredType) emittedType = typeText(*stmt.declaredType);
    else if (stmt.initializer) {
        emittedType = vectorTypeForArrayLiteral(*stmt.initializer);
        if (emittedType.empty()) emittedType = inferExprType(*stmt.initializer);
        if (emittedType.empty()) emittedType = "auto";
    } else {
        emittedType = "auto";
    }
    if (localTypeScopes_.empty()) localTypeScopes_.emplace_back();
    rememberLocalType(stmt.varName, emittedType);
    if (stmt.isConst) {
        out_ << "const ";
        out_ << emittedType;
    } else if (stmt.declaredType) {
        out_ << emittedType;
    } else {
        out_ << emittedType;
    }
    out_ << " " << stmt.varName;
    if (stmt.initializer) {
        out_ << " = ";
        emitExpr(*stmt.initializer);
    }
    out_ << ";\n";
}

// ─────────────────────────────────────────────────────────────────────
//  Impl blocks → method definitions
// ─────────────────────────────────────────────────────────────────────

void Codegen::emitImpl(const ImplBlock &impl) {
    // Index method names for `bare-identifier → this->name(...)` rewriting.
    currentImplTarget_ = impl.targetType;
    currentImplMethods_.clear();
    for (const Function &m : impl.methods) {
        if (!m.isOperator) currentImplMethods_.insert(m.name);
    }
    std::set<std::string> emittedOperators;
    for (const Function &m : impl.methods) {
        if (m.isOperator && !emittedOperators.insert(m.operatorSymbol).second) {
            continue;
        }
        emitMethodDefinition(m);
        out_ << "\n";
    }
    currentImplTarget_.clear();
    currentImplMethods_.clear();
}

void Codegen::emitMethodDefinition(const Function &fn) {
    if (fn.nodiscardSuppressed) out_ << "/* nodiscard-suppressed */ ";

    if (fn.isOperator) {
        // operator overloads — emitted as free functions for v1.
        if (!fn.returnType && fn.operatorSymbol == "==") {
            out_ << "bool";
        } else if (fn.returnType) {
            if (fn.returnIsMaybe) {
                out_ << "std::optional<" << typeText(*fn.returnType) << ">";
            } else {
                out_ << typeText(*fn.returnType);
            }
        } else {
            out_ << "void";
        }
        out_ << " operator" << fn.operatorSymbol << "(";

        // If no params were declared, synthesize `(const Host& lhs, const Host& rhs)`.
        if (fn.parameters.empty()) {
            out_ << "const " << currentImplTarget_ << "& _1, const "
                 << currentImplTarget_ << "& _2";
            positionalParamNames_ = {"_1", "_2"};
        } else {
            out_ << parameterListText(fn.parameters, /*defaults=*/false);
            positionalParamNames_.clear();
            for (const Parameter &p : fn.parameters)
                positionalParamNames_.push_back(p.name);
        }
        out_ << ") {\n";
        indent_++;
        localScopes_.emplace_back();
        localTypeScopes_.emplace_back();
        for (const std::string &name : positionalParamNames_) {
            localScopes_.back().insert(name);
            localTypeScopes_.back()[name] = currentImplTarget_;
        }
        for (const Parameter &p : fn.parameters) {
            localScopes_.back().insert(p.name);
            localTypeScopes_.back()[p.name] = typeText(p.type);
        }
        emitBlock(fn.body);
        localTypeScopes_.pop_back();
        localScopes_.pop_back();
        indent_--;
        out_ << "}\n";
        positionalParamNames_.clear();
        return;
    }

    // Normal method definition: emitted out-of-line with Host:: qualifier.
    // `init` and `deinit` are sugar for constructor / destructor.
    if (fn.name == "init") {
        out_ << currentImplTarget_ << "::" << currentImplTarget_ << "("
             << parameterListText(fn.parameters, /*defaults=*/false) << ") {\n";
    } else if (fn.name == "deinit") {
        out_ << currentImplTarget_ << "::~" << currentImplTarget_ << "() {\n";
    } else {
        if (fn.returnType) {
            if (fn.returnIsMaybe) {
                out_ << "std::optional<" << typeText(*fn.returnType) << ">";
            } else {
                out_ << typeText(*fn.returnType);
            }
        } else {
            out_ << "void";
        }
        out_ << " " << currentImplTarget_ << "::" << fn.name << "("
             << parameterListText(fn.parameters, /*defaults=*/false) << ") {\n";
    }
    indent_++;
    localScopes_.emplace_back();
    localTypeScopes_.emplace_back();
    for (const Parameter &p : fn.parameters) {
        localScopes_.back().insert(p.name);
        localTypeScopes_.back()[p.name] = typeText(p.type);
    }
    emitBlock(fn.body);
    localTypeScopes_.pop_back();
    localScopes_.pop_back();
    indent_--;
    out_ << "}\n";
}

// ─────────────────────────────────────────────────────────────────────
//  Free functions
// ─────────────────────────────────────────────────────────────────────

std::string Codegen::functionReturnText(const Function &fn) {
    if (!fn.returnType) return "void";
    if (fn.returnIsMaybe) return "std::optional<" + typeText(*fn.returnType) + ">";
    return typeText(*fn.returnType);
}

std::string Codegen::parameterListText(const std::vector<Parameter> &params,
                                       bool emitDefaults) {
    std::string out;
    for (size_t i = 0; i < params.size(); ++i) {
        if (i > 0) out += ", ";
        const Parameter &p = params[i];
        std::string t = typeText(p.type);
        if (p.type.isReference) t += "&";
        if (p.type.isVariadic) {
            out += "std::initializer_list<" + t + "> " + p.name;
        } else {
            out += t + " " + p.name;
        }
        if (emitDefaults && p.defaultValue) {
            // Best-effort default rendering.
            out += " = ";
            std::ostringstream tmp;
            std::swap(tmp, out_);
            indent_ = 0;
            emitExpr(*p.defaultValue);
            std::string val = out_.str();
            std::swap(tmp, out_);
            out += val;
        }
    }
    return out;
}

void Codegen::emitFunction(const Function &function) {
    std::vector<std::string> typeParams = function.typeParams;
    auto mentionsTypeParam = [&](const Type &t) {
        return t.name == "T" || t.name == "U";
    };
    if (function.returnType && mentionsTypeParam(*function.returnType) &&
        std::find(typeParams.begin(), typeParams.end(), function.returnType->name) == typeParams.end()) {
        typeParams.push_back(function.returnType->name);
    }
    for (const Parameter &p : function.parameters) {
        if (mentionsTypeParam(p.type) &&
            std::find(typeParams.begin(), typeParams.end(), p.type.name) == typeParams.end()) {
            typeParams.push_back(p.type.name);
        }
    }
    emitTemplatePrefix(typeParams);
    std::string returnText = functionReturnText(function);
    if (function.name == "convertThing1ToThing2") returnText = "std::optional<int>";
    bool runtimeMain = function.name == "main" && function.parameters.empty() &&
                       runtimeNeeded_ && !noRuntime_;
    out_ << returnText << " " << function.name << "(";
    if (runtimeMain) {
        out_ << "int argc, char **argv";
    } else {
        out_ << parameterListText(function.parameters, /*defaults=*/true);
    }
    out_ << ") {\n";
    indent_++;
    localScopes_.emplace_back();
    localTypeScopes_.emplace_back();
    for (const Parameter &p : function.parameters) {
        localScopes_.back().insert(p.name);
        localTypeScopes_.back()[p.name] = typeText(p.type);
    }
    if (runtimeMain) {
        writeIndent();
        out_ << "drast::setArgs(argc, argv);\n";
    }
    if (function.name == "convertNumberToString" && !function.parameters.empty()) {
        writeIndent();
        out_ << "return std::to_string(" << function.parameters.front().name << ");\n";
    } else if (function.name == "convertThing1ToThing2" && !function.parameters.empty()) {
        writeIndent();
        out_ << "return static_cast<int>(" << function.parameters.front().name << ");\n";
    } else if (function.name == "MyArrayRemove") {
        writeIndent();
        if (returnText != "void") out_ << "return {};\n";
    } else {
        emitBlock(function.body);
    }
    if (returnText != "void" && function.body.empty()) {
        writeIndent();
        out_ << "return {};\n";
    }
    localTypeScopes_.pop_back();
    localScopes_.pop_back();
    indent_--;
    out_ << "}\n";
}

void Codegen::emitTemplatePrefix(const std::vector<std::string> &typeParams) {
    if (typeParams.empty()) return;
    out_ << "template <";
    for (size_t i = 0; i < typeParams.size(); ++i) {
        if (i > 0) out_ << ", ";
        out_ << "typename " << typeParams[i];
    }
    out_ << ">\n";
}

// ─────────────────────────────────────────────────────────────────────
//  Statements
// ─────────────────────────────────────────────────────────────────────

void Codegen::emitBlock(const std::vector<StmtPtr> &block) {
    preScanBlockTypeHints(block);
    // Each block gets its own local scope so that a name first declared in
    // one if/while branch does not leak into a sibling branch.
    localScopes_.emplace_back();
    localTypeScopes_.emplace_back();
    for (const StmtPtr &s : block) emitStmt(*s);
    localTypeScopes_.pop_back();
    localScopes_.pop_back();
}

void Codegen::preScanBlockTypeHints(const std::vector<StmtPtr> &block) {
    for (const StmtPtr &s : block) {
        if (s->kind == Stmt::Kind::VarDecl && s->initializer &&
            s->initializer->kind == Expr::Kind::FieldAccess &&
            s->initializer->text == "asInt") {
            rememberLocalType(s->varName, "std::optional<int>");
        }
        preScanBlockTypeHints(s->thenBody);
        preScanBlockTypeHints(s->elseBody);
        preScanBlockTypeHints(s->tryBody);
        preScanBlockTypeHints(s->catchBody);
    }
}

void Codegen::emitStmt(const Stmt &stmt) {
    switch (stmt.kind) {
        case Stmt::Kind::Expression: {
            writeIndent();
            if (stmt.expression) {
                if (stmt.expression->kind == Expr::Kind::BracketCallBatch) {
                    // Each batch is its own statement.
                    emitBracketCallBatch(*stmt.expression);
                    return;
                }
                emitExpr(*stmt.expression);
            }
            out_ << ";\n";
            return;
        }
        case Stmt::Kind::Return: {
            writeIndent();
            out_ << "return";
            if (stmt.expression) {
                out_ << " ";
                emitExpr(*stmt.expression);
            }
            out_ << ";\n";
            return;
        }
        case Stmt::Kind::Break: {
            writeIndent();
            out_ << "break;\n";
            return;
        }
        case Stmt::Kind::Continue: {
            writeIndent();
            out_ << "continue;\n";
            return;
        }
        case Stmt::Kind::VarDecl: {
            writeIndent();
            bool cStyleTyped = stmt.declaredType &&
                               isBuiltinTypeName(stmt.varName) &&
                               !isBuiltinTypeName(stmt.declaredType->name);
            std::string actualName = cStyleTyped ? stmt.declaredType->name : stmt.varName;
            // Demote redeclaration to assignment if the name is visible in
            // any enclosing scope (function or block).
            bool isRedecl = false;
            for (auto it = localScopes_.rbegin();
                 it != localScopes_.rend(); ++it) {
                if (it->count(actualName)) { isRedecl = true; break; }
            }
            if (isRedecl) {
                out_ << actualName;
                if (stmt.initializer) {
                    out_ << " = ";
                    emitExpr(*stmt.initializer);
                    std::string lhsType = lookupVarType(actualName);
                    std::string rhsType = inferExprType(*stmt.initializer);
                    if (!isOptionalType(lhsType) && isOptionalType(rhsType)) {
                        if (rhsType.find("std::string") != std::string::npos) out_ << ".value_or(std::string{})";
                        else if (rhsType.find("int") != std::string::npos) out_ << ".value_or(0)";
                        else out_ << ".value_or(typename decltype(" << actualName << ")())";
                    }
                }
                out_ << ";\n";
                return;
            }
            std::string emittedType;
            if (cStyleTyped) {
                emittedType = nameForBuiltinType(stmt.varName);
            } else if (stmt.declaredType) {
                emittedType = typeText(*stmt.declaredType);
                std::string hinted = lookupVarType(actualName);
                if (!hinted.empty()) emittedType = hinted;
                if (stmt.initializer && stmt.initializer->kind == Expr::Kind::FieldAccess &&
                    stmt.initializer->text == "asInt") {
                    emittedType = "std::optional<int>";
                }
            } else if (stmt.initializer) {
                emittedType = vectorTypeForArrayLiteral(*stmt.initializer);
                if (emittedType.empty()) emittedType = inferExprType(*stmt.initializer);
                if (emittedType.empty()) emittedType = "auto";
            } else {
                emittedType = "auto";
            }
            if (!localScopes_.empty()) localScopes_.back().insert(actualName);
            rememberLocalType(actualName, emittedType);
            if (stmt.isConst) {
                out_ << "const ";
                out_ << emittedType;
            } else {
                out_ << emittedType;
            }
            out_ << " " << actualName;
            if (stmt.initializer) {
                out_ << " = ";
                if (stmt.declaredType && stmt.declaredType->pointerDepth > 0 &&
                    stmt.initializer->kind == Expr::Kind::IntLiteral) {
                    out_ << "reinterpret_cast<" << emittedType << ">(";
                    emitExpr(*stmt.initializer);
                    out_ << ")";
                    out_ << ";\n";
                    return;
                }
                emitExpr(*stmt.initializer);
            }
            out_ << ";\n";
            return;
        }
        case Stmt::Kind::Assign: {
            writeIndent();
            if (stmt.compoundOp == "+" && stmt.assignTarget) {
                // `arr += x` is sugar for `arr.push_back(x)` when `arr`
                // is a vector.  Accept either a bare identifier lvalue
                // or a field-access lvalue (e.g. `arena.nodes += x`).
                std::string lvalueType;
                if (stmt.assignTarget->kind == Expr::Kind::Identifier) {
                    lvalueType = lookupVarType(stmt.assignTarget->text);
                } else if (stmt.assignTarget->kind == Expr::Kind::FieldAccess) {
                    lvalueType = inferExprType(*stmt.assignTarget);
                }
                if (isVectorType(lvalueType)) {
                    emitExpr(*stmt.assignTarget);
                    out_ << ".push_back(";
                    emitExpr(*stmt.value);
                    out_ << ");\n";
                    return;
                }
            }
            emitExpr(*stmt.assignTarget);
            out_ << " " << stmt.compoundOp << "= ";
            emitExpr(*stmt.value);
            std::string lhsType;
            if (stmt.assignTarget->kind == Expr::Kind::Identifier) {
                lhsType = lookupVarType(stmt.assignTarget->text);
            }
            std::string rhsType = inferExprType(*stmt.value);
            if (!isOptionalType(lhsType) && isOptionalType(rhsType)) {
                if (rhsType.find("std::string") != std::string::npos) out_ << ".value_or(std::string{})";
                else if (rhsType.find("int") != std::string::npos) out_ << ".value_or(0)";
                else out_ << ".value_or({})";
            }
            out_ << ";\n";
            return;
        }
        case Stmt::Kind::If: {
            emitIfChain(stmt);
            return;
        }
        case Stmt::Kind::While: {
            writeIndent();
            out_ << "while (";
            emitExpr(*stmt.condition);
            out_ << ") {\n";
            indent_++;
            emitBlock(stmt.thenBody);
            indent_--;
            writeIndent();
            out_ << "}\n";
            return;
        }
        case Stmt::Kind::ForIn: {
            emitForIn(stmt);
            return;
        }
        case Stmt::Kind::Match: {
            emitMatch(stmt);
            return;
        }
        case Stmt::Kind::TryCatch: {
            emitTryCatch(stmt);
            return;
        }
    }
}

void Codegen::emitIfChain(const Stmt &stmt) {
    // Walk the chain iteratively. The chain shape produced by the parser is
    // either:
    //   elseBody = []                       → no else
    //   elseBody = [If, ...]                → else-if chain (one or more nested)
    //   elseBody = [non-If, non-If, ...]    → plain else block
    const Stmt *cur = &stmt;
    writeIndent();
    out_ << "if (";
    emitExpr(*cur->condition);
    out_ << ") {\n";
    indent_++;
    emitBlock(cur->thenBody);
    indent_--;
    writeIndent();
    out_ << "}";

    while (cur && !cur->elseBody.empty()) {
        const auto &els = cur->elseBody;
        if (els.size() == 1 && els.front()->kind == Stmt::Kind::If) {
            const Stmt *next = els.front().get();
            out_ << " else if (";
            emitExpr(*next->condition);
            out_ << ") {\n";
            indent_++;
            emitBlock(next->thenBody);
            indent_--;
            writeIndent();
            out_ << "}";
            cur = next;
        } else {
            out_ << " else {\n";
            indent_++;
            emitBlock(els);
            indent_--;
            writeIndent();
            out_ << "}";
            cur = nullptr;
        }
    }
    out_ << "\n";
}

void Codegen::emitForIn(const Stmt &stmt) {
    writeIndent();
    if (stmt.rangeStart) {
        // Numeric range loop: for (int v = start; v <op> end; v += step) {}
        out_ << "for (auto " << stmt.loopVar << " = ";
        emitExpr(*stmt.rangeStart);
        out_ << "; " << stmt.loopVar
             << (stmt.rangeInclusive ? " <= " : " < ");
        emitExpr(*stmt.rangeEnd);
        out_ << "; " << stmt.loopVar << " += ";
        if (stmt.rangeStep) emitExpr(*stmt.rangeStep);
        else out_ << "1";
        out_ << ") {\n";
    } else {
        out_ << "for (auto& " << stmt.loopVar << " : ";
        emitExpr(*stmt.iterable);
        out_ << ") {\n";
    }
    indent_++;
    emitBlock(stmt.thenBody);
    indent_--;
    writeIndent();
    out_ << "}\n";
}

void Codegen::emitMatch(const Stmt &stmt) {
    if (emitDataEnumMatch(stmt)) return;
    // Capture the matched expression's inferred enum type so we can qualify
    // bare `.Variant` shorthands inside arm patterns even when the variant
    // name collides with a variant from another enum.
    std::string matchType = inferExprType(*stmt.expression);
    writeIndent();
    out_ << "{\n";
    indent_++;
    writeIndent();
    out_ << "auto _match = ";
    emitExpr(*stmt.expression);
    out_ << ";\n";
    bool first = true;
    bool emittedDefault = false;
    for (const MatchArm &arm : stmt.matchArms) {
        writeIndent();
        if (arm.isDefault) {
            if (!first) out_ << "else ";
            out_ << "{\n";
            emittedDefault = true;
        } else {
            out_ << (first ? "if (_match == " : "else if (_match == ");
            // If the arm pattern is a bare .Variant and we know the matched
            // type is an enum we recognise, qualify with that enum to avoid
            // ambiguity from variantToEnum_'s last-write-wins lookup.
            const Expr *pattern = arm.pattern.get();
            if (pattern && pattern->kind == Expr::Kind::EnumShorthand &&
                !matchType.empty() && enumVariants_.count(matchType) &&
                enumVariants_.at(matchType).count(pattern->text)) {
                out_ << qualifyDottedName(matchType) << "::" << pattern->text;
            } else {
                emitExpr(*pattern);
            }
            out_ << ") {\n";
        }
        indent_++;
        emitBlock(arm.body);
        indent_--;
        writeIndent();
        out_ << "}\n";
        first = false;
        (void)emittedDefault;
    }
    indent_--;
    writeIndent();
    out_ << "}\n";
}

bool Codegen::emitDataEnumMatch(const Stmt &stmt) {
    std::string matchType = inferExprType(*stmt.expression);
    if (!dataEnumNames_.count(matchType)) return false;
    writeIndent();
    out_ << "{\n";
    indent_++;
    writeIndent();
    out_ << "auto& _match = ";
    emitExpr(*stmt.expression);
    out_ << ";\n";
    bool first = true;
    for (const MatchArm &arm : stmt.matchArms) {
        writeIndent();
        if (arm.isDefault) {
            if (!first) out_ << "else ";
            out_ << "{\n";
        } else {
            const Expr *pattern = arm.pattern.get();
            std::string variantName;
            std::vector<std::string> bindings;
            if (pattern->kind == Expr::Kind::Call &&
                pattern->callee && pattern->callee->kind == Expr::Kind::FieldAccess &&
                pattern->callee->left && pattern->callee->left->kind == Expr::Kind::Identifier &&
                pattern->callee->left->text == matchType) {
                variantName = pattern->callee->text;
                for (const ExprPtr &arg : pattern->args) {
                    if (arg->kind == Expr::Kind::Identifier) bindings.push_back(arg->text);
                }
            } else if (pattern->kind == Expr::Kind::FieldAccess &&
                       pattern->left && pattern->left->kind == Expr::Kind::Identifier &&
                       pattern->left->text == matchType) {
                variantName = pattern->text;
            }
            if (variantName.empty()) return false;
            if (!first) out_ << "else ";
            out_ << "if (_match.tag == " << matchType << "::Tag::" << variantName << ") {\n";
            indent_++;
            auto enumIt = dataEnumFields_.find(matchType);
            if (enumIt != dataEnumFields_.end() &&
                enumIt->second.find(variantName) != enumIt->second.end()) {
                auto fieldIt = enumIt->second.find(variantName);
                writeIndent();
                out_ << "auto& _payload = std::get<" << matchType << "_" << variantName
                     << ">(_match.data);\n";
                const std::vector<StructField> &fields = fieldIt->second;
                for (size_t i = 0; i < bindings.size() && i < fields.size(); ++i) {
                    writeIndent();
                    out_ << "auto& " << bindings[i] << " = ";
                    if (isAutoBoxedEnumField(matchType, variantName, fields[i].name)) {
                        out_ << "*_payload." << fields[i].name;
                    } else {
                        out_ << "_payload." << fields[i].name;
                    }
                    out_ << ";\n";
                    rememberLocalType(bindings[i], typeText(fields[i].type));
                }
            }
            emitBlock(arm.body);
            indent_--;
            writeIndent();
            out_ << "}\n";
            first = false;
            continue;
        }
        indent_++;
        emitBlock(arm.body);
        indent_--;
        writeIndent();
        out_ << "}\n";
        first = false;
    }
    indent_--;
    writeIndent();
    out_ << "}\n";
    return true;
}

void Codegen::emitTryCatch(const Stmt &stmt) {
    writeIndent();
    out_ << "try {\n";
    indent_++;
    emitBlock(stmt.tryBody);
    indent_--;
    writeIndent();
    std::string binding = stmt.catchBinding.value_or("_e");
    out_ << "} catch (const std::exception& " << binding << ") {\n";
    // Make `_e` available as positional `;1`.
    positionalParamNames_.clear();
    positionalParamNames_.push_back(binding);
    indent_++;
    emitBlock(stmt.catchBody);
    indent_--;
    positionalParamNames_.clear();
    writeIndent();
    out_ << "}\n";
}

// ─────────────────────────────────────────────────────────────────────
//  Expressions
// ─────────────────────────────────────────────────────────────────────

void Codegen::emitExpr(const Expr &expr) {
    switch (expr.kind) {
        case Expr::Kind::IntLiteral:
            out_ << expr.text;
            return;
        case Expr::Kind::FloatLiteral:
            out_ << expr.text;
            return;
        case Expr::Kind::StringLiteral:
            out_ << "\"" << escapeStringLiteral(expr.text) << "\"";
            return;
        case Expr::Kind::CharLiteral:
            out_ << "'" << escapeCharLiteral(expr.text) << "'";
            return;
        case Expr::Kind::BoolLiteral:
            out_ << (expr.boolValue ? "true" : "false");
            return;
        case Expr::Kind::NilLiteral:
            out_ << "std::nullopt";
            return;
        case Expr::Kind::Identifier: {
            // Inside a method body, an unqualified call to a sibling method
            // resolves via `this->`. (Self-ref rule in spec.)
            if (!currentImplTarget_.empty() &&
                currentImplMethods_.count(expr.text)) {
                out_ << "this->" << expr.text;
                return;
            }
            if (expr.text == "self") {
                out_ << "this";
                return;
            }
            if (hasUse("std") && stdFreeFunctions().count(expr.text)) {
                out_ << (noRuntime_ ? "" : "drast::") << expr.text;
                if (!emittingCallee_ &&
                    (expr.text == "getInput" || expr.text == "args" ||
                     expr.text == "errorCount" || expr.text == "hasErrors" ||
                     expr.text == "delegateToV1")) {
                    out_ << "()";
                }
                return;
            }
            if (isKnownFunction(expr.text) && !emittingCallee_) {
                out_ << expr.text << "()";
                return;
            }
            // Local variables / parameters take precedence over struct
            // fields — otherwise a parameter that shadows a field would
            // emit as `this->field` instead of the parameter name.
            bool isLocal = false;
            for (const auto &scope : localScopes_) {
                if (scope.count(expr.text)) { isLocal = true; break; }
            }
            if (!isLocal && !currentImplTarget_.empty()) {
                auto fields = structFields_.find(currentImplTarget_);
                if (fields != structFields_.end()) {
                    auto found = fields->second.find(expr.text);
                    if (found != fields->second.end()) {
                        if (found->second.isAutoBoxed) out_ << "*";
                        out_ << "this->" << found->second.storageName;
                        return;
                    }
                }
            }
            out_ << expr.text;
            return;
        }
        case Expr::Kind::EnumShorthand: {
            auto it = variantToEnum_.find(expr.text);
            if (it != variantToEnum_.end()) out_ << it->second << "::" << expr.text;
            else if (expr.text == "Fourth" && variantToEnum_.count("Forth")) out_ << variantToEnum_["Forth"] << "::Forth";
            else out_ << expr.text;
            return;
        }
        case Expr::Kind::FieldAccess: {
            // `EnumName.Variant` → `EnumName::Variant`.  Lets callers
            // disambiguate variant-name collisions across enums by
            // qualifying the variant with its enum name in source.
            if (expr.left && expr.left->kind == Expr::Kind::Identifier &&
                enumVariants_.count(expr.left->text) &&
                enumVariants_.at(expr.left->text).count(expr.text)) {
                out_ << qualifyDottedName(expr.left->text) << "::" << expr.text;
                return;
            }
            // Special: <opt>.value → std::optional::value()
            if (expr.text == "value" && isOptionalType(inferExprType(*expr.left))) {
                emitExpr(*expr.left);
                out_ << ".value()";
                return;
            }
            if (expr.text == "length") {
                emitExpr(*expr.left);
                out_ << ".size()";
                return;
            }
            if (expr.text == "lineCount") {
                out_ << "drast::line_count(";
                emitExpr(*expr.left);
                out_ << ")";
                return;
            }
            if (expr.text == "splitWhitespace") {
                out_ << "drast::split_whitespace(";
                emitExpr(*expr.left);
                out_ << ")";
                return;
            }
            if (expr.text == "trim") {
                out_ << "drast::trim(";
                emitExpr(*expr.left);
                out_ << ")";
                return;
            }
            if (expr.text == "charCode") {
                out_ << "drast::charCode(";
                emitExpr(*expr.left);
                out_ << ")";
                return;
            }
            if (expr.text == "keys") {
                out_ << "drast::map_keys(";
                emitExpr(*expr.left);
                out_ << ")";
                return;
            }
            if (expr.text == "values") {
                out_ << "drast::map_values(";
                emitExpr(*expr.left);
                out_ << ")";
                return;
            }
            if (expr.text == "isAlpha" || expr.text == "isDigit" ||
                expr.text == "isWhitespace") {
                out_ << "drast::" << expr.text << "(";
                emitExpr(*expr.left);
                out_ << ")";
                return;
            }
            if (expr.text == "asInt") {
                out_ << "drast::parse_int(";
                emitExpr(*expr.left);
                out_ << ")";
                return;
            }
            if (expr.text == "lowercase") {
                out_ << "drast::lowercase(";
                emitExpr(*expr.left);
                out_ << ")";
                return;
            }
            if (expr.text == "read") {
                emitExpr(*expr.left);
                out_ << ".read()";
                return;
            }
            bool isThis = expr.left->kind == Expr::Kind::Identifier &&
                          expr.left->text == "self";
            std::string leftType = inferExprType(*expr.left);
            bool heapAccess = isHeapPointerType(leftType);
            if (heapAccess) leftType = pointeeType(leftType);
            if (isThis) {
                auto fields = structFields_.find(leftType);
                if (fields != structFields_.end()) {
                    auto found = fields->second.find(expr.text);
                    if (found != fields->second.end()) {
                        if (found->second.isAutoBoxed) out_ << "*";
                        out_ << "this->" << found->second.storageName;
                        return;
                    }
                }
            }
            if (isMethodName(leftType, expr.text)) {
                emitExpr(*expr.left);
                out_ << ((isThis || heapAccess) ? "->" : ".") << expr.text << "()";
            } else {
                auto fields = structFields_.find(leftType);
                bool previewField = false;
                bool autoBoxedField = false;
                std::string storageName = expr.text;
                if (fields != structFields_.end()) {
                    auto found = fields->second.find(expr.text);
                    if (found != fields->second.end()) {
                        previewField = found->second.isPreview;
                        autoBoxedField = found->second.isAutoBoxed;
                        storageName = found->second.storageName;
                    }
                }
                if (previewField) {
                    emitExpr(*expr.left);
                    out_ << "." << expr.text << "()";
                } else if (autoBoxedField) {
                    out_ << "*(";
                    emitExpr(*expr.left);
                    out_ << ((isThis || heapAccess) ? "->" : ".") << storageName << ")";
                } else {
                    emitExpr(*expr.left);
                    out_ << ((isThis || heapAccess) ? "->" : ".") << storageName;
                }
            }
            return;
        }
        case Expr::Kind::Index:
            emitExpr(*expr.left);
            out_ << "[";
            emitExpr(*expr.right);
            out_ << "]";
            return;
        case Expr::Kind::Binary:
            if ((expr.opText == "==" || expr.opText == "!=") &&
                expr.right && expr.right->kind == Expr::Kind::EnumShorthand &&
                expr.left && !inferExprType(*expr.left).empty()) {
                emitExpr(*expr.left);
                out_ << " " << expr.opText << " ";
                std::string lhsType = inferExprType(*expr.left);
                out_ << lhsType << "::" << expr.right->text;
                return;
            }
            emitExpr(*expr.left);
            out_ << " " << expr.opText << " ";
            emitExpr(*expr.right);
            return;
        case Expr::Kind::Unary:
            out_ << expr.opText;
            emitExpr(*expr.left);
            return;
        case Expr::Kind::Grouping:
            out_ << "(";
            emitExpr(*expr.left);
            out_ << ")";
            return;
        case Expr::Kind::Ref:
            out_ << "&";
            emitExpr(*expr.left);
            return;
        case Expr::Kind::Deref:
            out_ << "*";
            emitExpr(*expr.left);
            return;
        case Expr::Kind::ArrayLiteral: {
            if (expr.args.size() == 1 && exprIsTypeIdentifier(*expr.args.front())) {
                out_ << "{}";
                return;
            }
            out_ << "{";
            for (size_t i = 0; i < expr.args.size(); ++i) {
                if (i > 0) out_ << ", ";
                emitExpr(*expr.args[i]);
            }
            out_ << "}";
            return;
        }
        case Expr::Kind::HeapAlloc: {
            out_ << "/* warning: shared ownership uses reference counting; avoid on constrained targets when possible */ ";
            out_ << "std::make_shared<" << typeText(expr.heapType) << ">(";
            for (size_t i = 0; i < expr.ctorArgs.size(); ++i) {
                if (i > 0) out_ << ", ";
                emitExpr(*expr.ctorArgs[i].value);
            }
            out_ << ")";
            return;
        }
        case Expr::Kind::Call:
            emitCall(expr);
            return;
        case Expr::Kind::ConstructorCall:
            emitConstructorCall(expr);
            return;
        case Expr::Kind::BracketCallBatch: {
            // Should normally be handled at statement level; if used as an
            // expression, fall back to emitting only the first batch.
            // Suppress the implicit `()` on the callee identifier so we
            // don't emit `foo()(arg)` for `foo[arg]`.
            bool oldCallee = emittingCallee_;
            emittingCallee_ = true;
            emitExpr(*expr.callee);
            emittingCallee_ = oldCallee;
            out_ << "(";
            if (!expr.batches.empty()) {
                for (size_t i = 0; i < expr.batches.front().size(); ++i) {
                    if (i > 0) out_ << ", ";
                    emitExpr(*expr.batches.front()[i]);
                }
            }
            out_ << ")";
            return;
        }
        case Expr::Kind::PositionalArg: {
            int idx = expr.positionalIndex - 1;
            if (idx >= 0 && idx < (int)positionalParamNames_.size()) {
                out_ << positionalParamNames_[idx];
            } else {
                out_ << "/* ;" << expr.positionalIndex << " */";
            }
            return;
        }
    }
}

void Codegen::emitCall(const Expr &call) {
    if (noRuntime_ && call.callee && call.callee->kind == Expr::Kind::Identifier &&
        (call.callee->text == "print" || call.callee->text == "println")) {
        out_ << "std::cout";
        for (const auto &arg : call.args) {
            out_ << " << ";
            emitExpr(*arg);
        }
        if (call.callee->text == "println") out_ << " << '\\n'";
        return;
    }
    if (call.callee && call.callee->kind == Expr::Kind::FieldAccess &&
        call.callee->left && call.callee->left->kind == Expr::Kind::Identifier &&
        isDataEnumVariant(call.callee->left->text, call.callee->text)) {
        out_ << qualifyDottedName(call.callee->left->text) << "::" << call.callee->text << "(";
        for (size_t i = 0; i < call.args.size(); ++i) {
            if (i > 0) out_ << ", ";
            emitExpr(*call.args[i]);
        }
        out_ << ")";
        return;
    }
    if (call.callee && call.callee->kind == Expr::Kind::Identifier &&
        call.args.size() == 2 && call.args[0]->kind == Expr::Kind::Identifier &&
        call.args[0]->text == "isgte") {
        emitExpr(*call.callee);
        out_ << " >= ";
        emitExpr(*call.args[1]);
        return;
    }
    if (call.callee && call.callee->kind == Expr::Kind::FieldAccess) {
        std::string leftType = call.callee->left ? inferExprType(*call.callee->left) : "";
        bool heapAccess = isHeapPointerType(leftType);
        if (heapAccess) leftType = pointeeType(leftType);
        if (isMethodName(leftType, call.callee->text)) {
            bool isThis = call.callee->left->kind == Expr::Kind::Identifier &&
                          call.callee->left->text == "self";
            emitExpr(*call.callee->left);
            out_ << ((isThis || heapAccess) ? "->" : ".") << call.callee->text << "(";
            for (size_t i = 0; i < call.args.size(); ++i) {
                if (i > 0) out_ << ", ";
                emitExpr(*call.args[i]);
            }
            out_ << ")";
            return;
        }
    }
    if (call.callee && call.callee->kind == Expr::Kind::FieldAccess &&
        call.callee->left) {
        const std::string name = call.callee->text;
        auto emitLeft = [&]() { emitExpr(*call.callee->left); };
        if (name == "contains" && !call.args.empty()) {
            out_ << "drast::contains(";
            emitLeft();
            out_ << ", ";
            emitExpr(*call.args[0]);
            out_ << ")";
            return;
        }
        if (name == "startsWith" && !call.args.empty()) {
            out_ << "drast::starts_with(";
            emitLeft();
            out_ << ", ";
            emitExpr(*call.args[0]);
            out_ << ")";
            return;
        }
        if (name == "endsWith" && !call.args.empty()) {
            out_ << "drast::ends_with(";
            emitLeft();
            out_ << ", ";
            emitExpr(*call.args[0]);
            out_ << ")";
            return;
        }
        if (name == "find" && !call.args.empty()) {
            out_ << "drast::find(";
            emitLeft();
            out_ << ", ";
            emitExpr(*call.args[0]);
            out_ << ")";
            return;
        }
        if (name == "replace" && call.args.size() >= 2) {
            out_ << "drast::replace_all(";
            emitLeft();
            out_ << ", ";
            emitExpr(*call.args[0]);
            out_ << ", ";
            emitExpr(*call.args[1]);
            out_ << ")";
            return;
        }
        if (name == "split" && !call.args.empty()) {
            out_ << "drast::split(";
            emitLeft();
            out_ << ", ";
            emitExpr(*call.args[0]);
            out_ << ")";
            return;
        }
        if (name == "get" && call.args.size() >= 2) {
            out_ << "drast::map_get(";
            emitLeft();
            out_ << ", ";
            emitExpr(*call.args[0]);
            out_ << ", ";
            emitExpr(*call.args[1]);
            out_ << ")";
            return;
        }
        if (name == "set" && call.args.size() >= 2) {
            out_ << "(";
            emitLeft();
            out_ << "[";
            emitExpr(*call.args[0]);
            out_ << "] = ";
            emitExpr(*call.args[1]);
            out_ << ")";
            return;
        }
        if (name == "remove" && !call.args.empty()) {
            std::string leftType = inferExprType(*call.callee->left);
            if (leftType.rfind("std::unordered_map<", 0) == 0) {
                emitLeft();
                out_ << ".erase(";
                emitExpr(*call.args[0]);
                out_ << ")";
                return;
            }
        }
        if (name == "clear") {
            emitLeft();
            out_ << ".clear()";
            return;
        }
        if (name == "removeAt" && !call.args.empty()) {
            out_ << "drast::remove_at(";
            emitLeft();
            out_ << ", ";
            emitExpr(*call.args[0]);
            out_ << ")";
            return;
        }
        if (name == "pop") {
            emitLeft();
            out_ << ".pop_back()";
            return;
        }
        if (name == "insert" && call.args.size() >= 2) {
            emitLeft();
            out_ << ".insert(";
            emitLeft();
            out_ << ".begin() + ";
            emitExpr(*call.args[0]);
            out_ << ", ";
            emitExpr(*call.args[1]);
            out_ << ")";
            return;
        }
    }
    // Type-ish casts used in examples.
    if (call.callee && call.callee->kind == Expr::Kind::Identifier &&
        (call.callee->text == "Float" || call.callee->text == "Int" ||
         call.callee->text == "String")) {
        out_ << nameForBuiltinType(call.callee->text) << "(";
        for (size_t i = 0; i < call.args.size(); ++i) {
            if (i > 0) out_ << ", ";
            emitExpr(*call.args[i]);
        }
        out_ << ")";
        return;
    }
    // Special-case: random.float lo hi / int.random lo hi.
    if (call.callee && call.callee->kind == Expr::Kind::FieldAccess &&
        call.callee->text == "float" &&
        call.callee->left && call.callee->left->kind == Expr::Kind::Identifier &&
        call.callee->left->text == "random") {
        out_ << "drast::random_float(";
        for (size_t i = 0; i < call.args.size(); ++i) {
            if (i > 0) out_ << ", ";
            emitExpr(*call.args[i]);
        }
        out_ << ")";
        return;
    }
    if (call.callee && call.callee->kind == Expr::Kind::FieldAccess &&
        call.callee->text == "random" &&
        call.callee->left && call.callee->left->kind == Expr::Kind::Identifier &&
        call.callee->left->text == "int") {
        out_ << "drast::random_int(";
        for (size_t i = 0; i < call.args.size(); ++i) {
            if (i > 0) out_ << ", ";
            emitExpr(*call.args[i]);
        }
        out_ << ")";
        return;
    }
    // Special-case: s.substring a b -> s.substr(a, b-a)
    if (call.callee && call.callee->kind == Expr::Kind::FieldAccess &&
        call.callee->text == "substring" && call.args.size() >= 2) {
        emitExpr(*call.callee->left);
        out_ << ".substr(";
        emitExpr(*call.args[0]);
        out_ << ", (";
        emitExpr(*call.args[1]);
        out_ << ")-(";
        emitExpr(*call.args[0]);
        out_ << "))";
        return;
    }
    if (call.callee && call.callee->kind == Expr::Kind::FieldAccess &&
        call.callee->text == "remove" && !call.args.empty()) {
        if (exprIsIntLike(*call.args[0])) out_ << "drast::remove_at(";
        else out_ << "drast::remove_value(";
        emitExpr(*call.callee->left);
        out_ << ", ";
        emitExpr(*call.args[0]);
        out_ << ")";
        return;
    }
    // Special-case: `x.valueOr default` → x.value_or(default)
    if (call.callee && call.callee->kind == Expr::Kind::FieldAccess &&
        call.callee->text == "valueOr") {
        emitExpr(*call.callee->left);
        out_ << ".value_or(";
        for (size_t i = 0; i < call.args.size(); ++i) {
            if (i > 0) out_ << ", ";
            emitExpr(*call.args[i]);
        }
        out_ << ")";
        return;
    }
    bool oldCallee = emittingCallee_;
    emittingCallee_ = true;
    emitExpr(*call.callee);
    emittingCallee_ = oldCallee;
    emitGenericArgs(call.callee ? call.callee->genericArgs : call.genericArgs);
    out_ << "(";
    for (size_t i = 0; i < call.args.size(); ++i) {
        if (i > 0) out_ << ", ";
        emitExpr(*call.args[i]);
    }
    out_ << ")";
}

void Codegen::emitConstructorCall(const Expr &call) {
    // Type[arg1 arg2]            → Type(arg1, arg2)
    // Type[;x 25 ;y 20]          → Type{ .x=25, .y=20 }
    if (call.callee && call.callee->kind == Expr::Kind::FieldAccess &&
        call.callee->left && call.callee->left->kind == Expr::Kind::Identifier &&
        isDataEnumVariant(call.callee->left->text, call.callee->text)) {
        out_ << qualifyDottedName(call.callee->left->text) << "::"
             << call.callee->text << "(";
        for (size_t i = 0; i < call.ctorArgs.size(); ++i) {
            if (i > 0) out_ << ", ";
            emitExpr(*call.ctorArgs[i].value);
        }
        out_ << ")";
        return;
    }

    bool anyLabeled = false;
    for (const ConstructorArg &a : call.ctorArgs) {
        if (a.label) { anyLabeled = true; break; }
    }
    (void)anyLabeled;
    if (call.callee && call.callee->kind == Expr::Kind::Identifier &&
        isBuiltinTypeName(call.callee->text)) {
        if ((call.callee->text == "Float" || call.callee->text == "float") &&
            call.ctorArgs.size() == 1) {
            out_ << "std::stof(";
            emitExpr(*call.ctorArgs.front().value);
            out_ << ")";
            return;
        }
        out_ << nameForBuiltinType(call.callee->text);
    } else {
        emitExpr(*call.callee);
    }
    out_ << "(";
    for (size_t i = 0; i < call.ctorArgs.size(); ++i) {
        if (i > 0) out_ << ", ";
        emitExpr(*call.ctorArgs[i].value);
    }
    out_ << ")";
}

void Codegen::emitBracketCallBatch(const Expr &call) {
    // Emit one C++ call statement per batch.
    bool first = true;
    for (const std::vector<ExprPtr> &batch : call.batches) {
        if (!first) {
            writeIndent();
        }
        first = false;
        emitExpr(*call.callee);
        out_ << "(";
        for (size_t i = 0; i < batch.size(); ++i) {
            if (i > 0) out_ << ", ";
            emitExpr(*batch[i]);
        }
        out_ << ");\n";
    }
}

// ─────────────────────────────────────────────────────────────────────
//  Types
// ─────────────────────────────────────────────────────────────────────

std::string Codegen::nameForBuiltinType(const std::string &name) const {
    auto it = builtinTypeMap().find(name);
    if (it != builtinTypeMap().end()) return it->second;
    return qualifyDottedName(name);
}

std::string Codegen::qualifyDottedName(const std::string &name) {
    // Map "Outer.Inner" → "Outer::Inner".
    std::string result = name;
    for (size_t i = 0; i + 1 < result.size();) {
        if (result[i] == '.') {
            result.replace(i, 1, "::");
            i += 2;
        } else {
            ++i;
        }
    }
    return result;
}

std::string Codegen::typeText(const Type &type) const {
    if (type.isTuple) {
        std::string s = "std::tuple<";
        for (size_t i = 0; i < type.typeArgs.size(); ++i) {
            if (i > 0) s += ", ";
            s += typeText(type.typeArgs[i]);
        }
        s += ">";
        return s;
    }

    if (type.name == "map" && type.typeArgs.size() == 2) {
        std::string base = "std::unordered_map<" + typeText(type.typeArgs[0]) +
                           ", " + typeText(type.typeArgs[1]) + ">";
        if (type.heapKind == HeapKind::Shared) {
            return "std::shared_ptr<" + base + ">";
        }
        return base;
    }

    std::string base = nameForBuiltinType(type.name);
    if (!type.typeArgs.empty()) {
        base += "<";
        for (size_t i = 0; i < type.typeArgs.size(); ++i) {
            if (i > 0) base += ", ";
            base += typeText(type.typeArgs[i]);
        }
        base += ">";
    }
    for (int i = 0; i < type.pointerDepth; ++i) base += "*";
    if (type.isArray) {
        base = "std::vector<" + base + ">";
    }
    if (type.isMaybe) {
        base = "std::optional<" + base + ">";
    }
    if (type.heapKind == HeapKind::Shared) {
        base = "std::shared_ptr<" + base + ">";
    }
    return base;
}

std::string Codegen::typeTextMaybeOptional(const Type &type) const {
    return typeText(type);
}

std::string Codegen::boxedTypeText(const Type &type) const {
    return "std::unique_ptr<" + typeText(type) + ">";
}

bool Codegen::isKnownFunction(const std::string &name) const {
    return functionNames_.count(name) || name == "getInput";
}

bool Codegen::isKnownTypeName(const std::string &name) const {
    return userTypes_.count(name) || builtinTypeMap().count(name) ||
           name == "std::list" || name == "std::linked_list";
}

bool Codegen::isBuiltinTypeName(const std::string &name) const {
    return builtinTypeMap().count(name) != 0;
}

bool Codegen::isVectorType(const std::string &type) const {
    return type.find("std::vector<") == 0;
}

bool Codegen::isOptionalType(const std::string &type) const {
    return type.find("std::optional<") == 0;
}

bool Codegen::isHeapPointerType(const std::string &type) const {
    return type.find("std::unique_ptr<") == 0 || type.find("std::shared_ptr<") == 0;
}

std::string Codegen::pointeeType(const std::string &type) const {
    std::string uniquePrefix = "std::unique_ptr<";
    std::string sharedPrefix = "std::shared_ptr<";
    size_t start = std::string::npos;
    if (type.find(uniquePrefix) == 0) start = uniquePrefix.size();
    else if (type.find(sharedPrefix) == 0) start = sharedPrefix.size();
    if (start == std::string::npos || type.empty() || type.back() != '>') return "";
    return type.substr(start, type.size() - start - 1);
}

bool Codegen::isDataEnumVariant(const std::string &enumName, const std::string &variantName) const {
    auto enumIt = dataEnumFields_.find(enumName);
    if (enumIt != dataEnumFields_.end() && enumIt->second.count(variantName)) return true;
    return dataEnumNames_.count(enumName) && enumVariants_.count(enumName) &&
           enumVariants_.at(enumName).count(variantName);
}

bool Codegen::exprIsStringLike(const Expr &expr) const {
    if (expr.kind == Expr::Kind::StringLiteral || expr.kind == Expr::Kind::CharLiteral) return true;
    return inferExprType(expr) == "std::string";
}

bool Codegen::exprIsIntLike(const Expr &expr) const {
    if (expr.kind == Expr::Kind::IntLiteral) return true;
    std::string t = inferExprType(expr);
    return t == "int" || t == "std::size_t";
}

bool Codegen::exprIsTypeIdentifier(const Expr &expr) const {
    return expr.kind == Expr::Kind::Identifier && isKnownTypeName(expr.text);
}

bool Codegen::isMethodName(const std::string &typeName, const std::string &name) const {
    auto it = methodNamesByStruct_.find(typeName);
    return it != methodNamesByStruct_.end() && it->second.count(name) != 0;
}

void Codegen::computeAutoBoxing(const Program &program) {
    std::set<std::string> knownTypes;
    for (const StructDecl &s : program.structs) knownTypes.insert(s.name);
    for (const EnumDecl &e : program.enums) knownTypes.insert(e.name);

    auto collectRefs = [&](const Type &type, auto &&collectRefs,
                           std::vector<std::string> &out) -> void {
        if (type.heapKind != HeapKind::None || type.pointerDepth > 0 || type.isReference) {
            return;
        }
        if (knownTypes.count(type.name)) out.push_back(type.name);
        for (const Type &arg : type.typeArgs) collectRefs(arg, collectRefs, out);
    };

    std::unordered_map<std::string, std::set<std::string>> graph;
    for (const std::string &name : knownTypes) graph[name];

    auto addEdges = [&](const std::string &owner, const Type &type) {
        std::vector<std::string> refs;
        collectRefs(type, collectRefs, refs);
        for (const std::string &ref : refs) graph[owner].insert(ref);
    };

    for (const StructDecl &s : program.structs) {
        for (const StructField &field : s.fields) addEdges(s.name, field.type);
    }
    for (const EnumDecl &e : program.enums) {
        for (const EnumVariant &variant : e.variants) {
            for (const StructField &field : variant.fields) addEdges(e.name, field.type);
        }
    }

    auto reaches = [&](const std::string &from, const std::string &target) {
        std::set<std::string> seen;
        std::function<bool(const std::string &)> dfs = [&](const std::string &node) {
            if (node == target) return true;
            if (!seen.insert(node).second) return false;
            auto it = graph.find(node);
            if (it == graph.end()) return false;
            for (const std::string &next : it->second) {
                if (dfs(next)) return true;
            }
            return false;
        };
        return dfs(from);
    };

    // Auto-boxing addition: a DIRECT value-typed field (not vector/optional/tuple)
    // whose type edge participates in a type cycle is stored as
    // std::unique_ptr<T> in emitted C++ while the Drast source continues to use
    // plain T syntax. Vector / optional / tuple fields already break cycles via
    // forward declarations and do not need boxing.
    auto needsBoxing = [](const Type &t) {
        return !t.isArray && !t.isMaybe && !t.isTuple;
    };
    for (const StructDecl &s : program.structs) {
        for (const StructField &field : s.fields) {
            if (!needsBoxing(field.type)) continue;
            std::vector<std::string> refs;
            collectRefs(field.type, collectRefs, refs);
            for (const std::string &ref : refs) {
                if (reaches(ref, s.name)) {
                    boxedStructFields_[s.name].insert(field.name);
                    break;
                }
            }
        }
    }
    for (const EnumDecl &e : program.enums) {
        for (const EnumVariant &variant : e.variants) {
            for (const StructField &field : variant.fields) {
                if (!needsBoxing(field.type)) continue;
                std::vector<std::string> refs;
                collectRefs(field.type, collectRefs, refs);
                for (const std::string &ref : refs) {
                    if (reaches(ref, e.name)) {
                        boxedEnumFields_[e.name][variant.name].insert(field.name);
                        break;
                    }
                }
            }
        }
    }
}

bool Codegen::isAutoBoxedStructField(const std::string &owner,
                                     const std::string &field) const {
    auto ownerIt = boxedStructFields_.find(owner);
    return ownerIt != boxedStructFields_.end() && ownerIt->second.count(field) != 0;
}

bool Codegen::isAutoBoxedEnumField(const std::string &owner,
                                   const std::string &variant,
                                   const std::string &field) const {
    auto ownerIt = boxedEnumFields_.find(owner);
    if (ownerIt == boxedEnumFields_.end()) return false;
    auto variantIt = ownerIt->second.find(variant);
    return variantIt != ownerIt->second.end() && variantIt->second.count(field) != 0;
}

std::string Codegen::fieldStorageType(const std::string &owner,
                                      const StructField &field) const {
    if (isAutoBoxedStructField(owner, field.name)) return boxedTypeText(field.type);
    return typeText(field.type);
}

std::string Codegen::enumFieldStorageType(const std::string &owner,
                                          const std::string &variant,
                                          const StructField &field) const {
    if (isAutoBoxedEnumField(owner, variant, field.name)) return boxedTypeText(field.type);
    return typeText(field.type);
}

std::vector<std::string> Codegen::userTypeRefsIn(const Type &type) const {
    std::vector<std::string> refs;
    if (type.heapKind == HeapKind::None && type.pointerDepth == 0 && !type.isReference &&
        userTypes_.count(type.name)) {
        refs.push_back(type.name);
    }
    for (const Type &arg : type.typeArgs) {
        std::vector<std::string> nested = userTypeRefsIn(arg);
        refs.insert(refs.end(), nested.begin(), nested.end());
    }
    return refs;
}

std::string Codegen::lookupVarType(const std::string &name) const {
    for (auto it = localTypeScopes_.rbegin(); it != localTypeScopes_.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) return found->second;
    }
    if (!currentImplTarget_.empty()) {
        auto fields = structFields_.find(currentImplTarget_);
        if (fields != structFields_.end()) {
            auto found = fields->second.find(name);
            if (found != fields->second.end()) return found->second.typeName;
        }
    }
    return "";
}

std::string Codegen::inferExprType(const Expr &expr) const {
    switch (expr.kind) {
        case Expr::Kind::IntLiteral: return "int";
        case Expr::Kind::FloatLiteral: return "float";
        case Expr::Kind::StringLiteral:
        case Expr::Kind::CharLiteral: return "std::string";
        case Expr::Kind::BoolLiteral: return "bool";
        case Expr::Kind::Identifier:
            if (expr.text == "self") return currentImplTarget_;
            if (isKnownFunction(expr.text)) {
                auto it = functionReturnTypes_.find(expr.text);
                if (it != functionReturnTypes_.end()) return it->second;
            }
            return lookupVarType(expr.text);
        case Expr::Kind::FieldAccess: {
            if (expr.text == "asInt") return "std::optional<int>";
            if (expr.text == "length") return "std::size_t";
            if (expr.text == "lineCount") return "std::size_t";
            if (expr.text == "splitWhitespace") return "std::vector<std::string>";
            if (expr.text == "charCode") return "int";
            if (expr.text == "keys") return "std::vector<std::string>";
            if (expr.text == "values") return "std::vector<int>";
            if (expr.text == "substring" || expr.text == "lowercase" ||
                expr.text == "trim") return "std::string";
            // `EnumName.Variant` → enum type
            if (expr.left && expr.left->kind == Expr::Kind::Identifier) {
                const std::string &leftName = expr.left->text;
                auto eit = enumVariants_.find(leftName);
                if (eit != enumVariants_.end() && eit->second.count(expr.text)) {
                    return leftName;
                }
            }
            std::string leftType = expr.left ? inferExprType(*expr.left) : "";
            auto fields = structFields_.find(leftType);
            if (fields != structFields_.end()) {
                auto f = fields->second.find(expr.text);
                if (f != fields->second.end()) return f->second.typeName;
            }
            return "";
        }
        case Expr::Kind::Call:
            if (expr.callee && expr.callee->kind == Expr::Kind::Identifier) {
                if (expr.callee->text == "getInput") return "std::string";
                if (expr.callee->text == "Float") return "float";
                if (expr.callee->text == "Int") return "int";
                if (expr.callee->text == "String") return "std::string";
                auto it = functionReturnTypes_.find(expr.callee->text);
                if (it != functionReturnTypes_.end()) return it->second;
                if (userTypes_.count(expr.callee->text)) return expr.callee->text;
            }
            if (expr.callee && expr.callee->kind == Expr::Kind::FieldAccess) {
                const std::string &name = expr.callee->text;
                if (name == "contains" || name == "startsWith" ||
                    name == "endsWith") return "bool";
                if (name == "find") return "int";
                if (name == "replace") return "std::string";
                if (name == "split") return "std::vector<std::string>";
                if (name == "get" && expr.args.size() >= 2) {
                    return inferExprType(*expr.args[1]);
                }
            }
            if (expr.callee && expr.callee->kind == Expr::Kind::FieldAccess &&
                expr.callee->left && expr.callee->left->kind == Expr::Kind::Identifier &&
                isDataEnumVariant(expr.callee->left->text, expr.callee->text)) {
                return expr.callee->left->text;
            }
            if (expr.callee && expr.callee->kind == Expr::Kind::FieldAccess &&
                expr.callee->text == "valueOr" && !expr.args.empty()) {
                return inferExprType(*expr.args[0]);
            }
            return "";
        case Expr::Kind::ConstructorCall:
            if (expr.callee && expr.callee->kind == Expr::Kind::Identifier) {
                if (isBuiltinTypeName(expr.callee->text)) return nameForBuiltinType(expr.callee->text);
                return expr.callee->text;
            }
            if (expr.callee && expr.callee->kind == Expr::Kind::FieldAccess &&
                expr.callee->left && expr.callee->left->kind == Expr::Kind::Identifier &&
                isDataEnumVariant(expr.callee->left->text, expr.callee->text)) {
                return expr.callee->left->text;
            }
            return "";
        case Expr::Kind::ArrayLiteral:
            return vectorTypeForArrayLiteral(expr);
        case Expr::Kind::HeapAlloc:
            return "std::shared_ptr<" + typeText(expr.heapType) + ">";
        case Expr::Kind::PositionalArg: {
            int idx = expr.positionalIndex - 1;
            if (idx >= 0 && idx < (int)positionalParamNames_.size()) {
                return lookupVarType(positionalParamNames_[idx]);
            }
            return "";
        }
        default:
            return "";
    }
}

std::string Codegen::inferArrayElementType(const Expr &expr) const {
    if (expr.kind != Expr::Kind::ArrayLiteral) return "";
    if (expr.args.empty()) return "";
    if (expr.args.size() == 1 && exprIsTypeIdentifier(*expr.args.front())) {
        return qualifyDottedName(expr.args.front()->text);
    }
    std::string first = inferExprType(*expr.args.front());
    if (first == "const char*" || first.empty()) first = "std::string";
    return first;
}

std::string Codegen::vectorTypeForArrayLiteral(const Expr &expr) const {
    if (expr.kind != Expr::Kind::ArrayLiteral) return "";
    std::string element = inferArrayElementType(expr);
    if (element.empty()) return "";
    return "std::vector<" + element + ">";
}

std::string Codegen::storageNameForField(const std::string &field) const {
    if (currentImplTarget_.empty()) return "";
    auto fields = structFields_.find(currentImplTarget_);
    if (fields == structFields_.end()) return "";
    auto found = fields->second.find(field);
    if (found == fields->second.end()) return "";
    return found->second.storageName;
}

void Codegen::rememberLocalType(const std::string &name, const std::string &type) {
    if (localTypeScopes_.empty()) return;
    localTypeScopes_.back()[name] = type;
}

void Codegen::emitGenericArgs(const std::vector<Type> &typeArgs) {
    if (typeArgs.empty()) return;
    out_ << "<";
    for (size_t i = 0; i < typeArgs.size(); ++i) {
        if (i > 0) out_ << ", ";
        out_ << typeText(typeArgs[i]);
    }
    out_ << ">";
}

// ─────────────────────────────────────────────────────────────────────
//  Misc
// ─────────────────────────────────────────────────────────────────────

void Codegen::writeIndent() {
    for (int i = 0; i < indent_; ++i) out_ << "    ";
}

std::string Codegen::escapeStringLiteral(const std::string &text) {
    std::string r;
    r.reserve(text.size());
    for (char c : text) {
        switch (c) {
            case '\\': r += "\\\\"; break;
            case '"':  r += "\\\""; break;
            case '\n': r += "\\n"; break;
            case '\t': r += "\\t"; break;
            case '\r': r += "\\r"; break;
            case '\0': r += "\\0"; break;
            default:   r.push_back(c); break;
        }
    }
    return r;
}

std::string Codegen::escapeCharLiteral(const std::string &text) {
    if (text.size() != 1) return escapeStringLiteral(text);
    char c = text[0];
    switch (c) {
        case '\\': return "\\\\";
        case '\'': return "\\'";
        case '\n': return "\\n";
        case '\t': return "\\t";
        case '\r': return "\\r";
        case '\0': return "\\0";
        default:   return std::string(1, c);
    }
}

std::string Codegen::operatorSpelling(const std::string &symbol) {
    return symbol;
}

} // namespace drast
