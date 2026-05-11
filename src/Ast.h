#pragma once

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>
#include "Token.h"

namespace drast {

struct Expr;
struct Stmt;

using ExprPtr = std::unique_ptr<Expr>;
using StmtPtr = std::unique_ptr<Stmt>;

// ── Types ──────────────────────────────────────────────────────────────

enum class HeapKind { None, Shared };

struct Type {
    std::string name;                       // e.g. "int", "string", "Point", "Dog.Breed"
    std::vector<Type> typeArgs;             // generic args: `[T U]
    int pointerDepth = 0;                   // T`  → 1, T``  → 2
    bool isReference = false;               // ~T parameter passes by ref
    bool isMaybe = false;                   // maybe T return → std::optional<T>
    bool isArray = false;                   // {T} → std::vector<T>
    bool isVariadic = false;                // variadic[T] → T...
    bool isTuple = false;                   // tuple T1 T2 → std::tuple<T1, T2>
    bool isDiscard = false;                 // return type ... discard → suppress nodiscard
    HeapKind heapKind = HeapKind::None;     // @[T] explicit shared ownership
    SourceLocation location;
};

// ── Imports ────────────────────────────────────────────────────────────

struct UseDecl {
    enum class Kind { Angle, Quoted };
    Kind kind = Kind::Angle;
    std::string path;
    bool isDrastModule = false;             // Resolved by the driver; merged before codegen.
    SourceLocation location;
};

// ── Match arm ─────────────────────────────────────────────────────────

struct MatchArm {
    ExprPtr pattern;                        // null for `default`
    bool isDefault = false;
    std::vector<StmtPtr> body;
    SourceLocation location;
};

// ── Expressions ────────────────────────────────────────────────────────

struct ConstructorArg {
    std::optional<std::string> label;       // ;x 25  → label = "x"
    ExprPtr value;
};

struct Expr {
    enum class Kind {
        IntLiteral,
        FloatLiteral,
        StringLiteral,
        CharLiteral,
        BoolLiteral,
        NilLiteral,
        Identifier,             // bare name
        EnumShorthand,          // .Variant — leading dot
        FieldAccess,            // expr.field
        Index,                  // expr{idx}
        Call,                   // implicit space-separated args: print 'x'
        ConstructorCall,        // Type[args]  (or labeled named init)
        BracketCallBatch,       // callee[ a, b ] → emits multiple calls
        Binary,                 // a op b
        Unary,                  // op a
        Grouping,               // (expr) or [expr]
        ArrayLiteral,           // {a b c}
        HeapAlloc,              // @[Type args] explicit shared allocation
        Ref,                    // ~x
        Deref,                  // `x
        PositionalArg,          // ;1 / ;2  (used in operator bodies and catch)
    } kind = Kind::IntLiteral;
    SourceLocation location;

    std::string text;                       // Identifier name / literal text / field name
    long long intValue = 0;
    double floatValue = 0.0;
    bool boolValue = false;
    int positionalIndex = 0;

    ExprPtr left;
    ExprPtr right;
    std::string opText;                     // for Binary / Unary
    Type heapType;                          // HeapAlloc target type

    ExprPtr callee;                         // Call / Ctor / BracketCallBatch
    std::vector<ExprPtr> args;              // Call / ArrayLiteral elements
    std::vector<Type> genericArgs;          // expression generic args: f`[T U]
    std::vector<ConstructorArg> ctorArgs;   // ConstructorCall
    std::vector<std::vector<ExprPtr>> batches; // BracketCallBatch — each inner vector is one call's args
};

// ── Statements ─────────────────────────────────────────────────────────

struct Stmt {
    enum class Kind {
        Expression,
        Return,
        VarDecl,
        Assign,
        If,
        While,
        ForIn,
        Match,
        TryCatch,
        Break,
        Continue,
    } kind = Kind::Expression;
    SourceLocation location;

    // ── Expression / Return value ──
    ExprPtr expression;

    // ── VarDecl ──
    std::string varName;
    bool isConst = false;
    std::optional<Type> declaredType;
    ExprPtr initializer;                    // null for uninitialized vars

    // ── Assign ──
    ExprPtr assignTarget;                   // any lvalue expression
    std::string compoundOp;                 // empty / "+" / "-" / "*" / "/"
    ExprPtr value;

    // ── If / While ──
    ExprPtr condition;
    std::vector<StmtPtr> thenBody;
    std::vector<StmtPtr> elseBody;

    // ── ForIn ──
    std::string loopVar;
    ExprPtr iterable;                       // for-in collection (non-range)
    ExprPtr rangeStart;
    ExprPtr rangeEnd;
    ExprPtr rangeStep;
    bool rangeInclusive = false;

    // ── Match ──
    std::vector<MatchArm> matchArms;

    // ── TryCatch ──
    std::vector<StmtPtr> tryBody;
    std::vector<StmtPtr> catchBody;
    std::optional<std::string> catchBinding;
};

// ── Function & method declarations ────────────────────────────────────

struct Parameter {
    std::string name;
    Type type;
    ExprPtr defaultValue;
    SourceLocation location;
};

enum class Visibility { Public, Private, Preview, Fileprivate };

struct Function {
    std::string name;
    std::vector<std::string> typeParams;    // generic param names
    std::vector<Parameter> parameters;
    std::optional<Type> returnType;         // unset for void
    bool returnIsMaybe = false;
    bool nodiscardSuppressed = false;       // return type followed by `discard`
    std::vector<StmtPtr> body;
    Visibility visibility = Visibility::Public;
    SourceLocation location;
    bool isOperator = false;                // operator[==], etc.
    std::string operatorSymbol;             // "==", "+", ...
    // Within impl, identifies the host type for self-rewriting.
    std::string implTarget;
};

// ── Structs ──────────────────────────────────────────────────────────

struct StructField {
    std::string name;
    Type type;
    Visibility visibility = Visibility::Public;
    SourceLocation location;
};

struct StructDecl {
    std::string name;
    std::vector<std::string> typeParams;
    std::vector<StructField> fields;
    bool isFileprivate = false;
    SourceLocation location;
};

// ── Enums ────────────────────────────────────────────────────────────

struct EnumVariant {
    std::string name;
    std::vector<StructField> fields;
    SourceLocation location;
};

struct EnumDecl {
    std::string name;                       // may be "Outer.Inner" for nested
    std::vector<EnumVariant> variants;
    bool isFileprivate = false;
    SourceLocation location;
};

// ── Protocols ────────────────────────────────────────────────────────

struct ProtocolMethod {
    std::string name;
    std::vector<Parameter> parameters;
    std::optional<Type> returnType;
    bool returnIsMaybe = false;
    SourceLocation location;
};

struct ProtocolDecl {
    std::string name;
    std::vector<ProtocolMethod> methods;
    SourceLocation location;
};

// ── Impl blocks ──────────────────────────────────────────────────────

struct ImplBlock {
    std::string targetType;                 // host type name
    std::optional<std::string> protocolName;
    std::vector<Function> methods;
    SourceLocation location;
};

// ── Top-level program ───────────────────────────────────────────────

struct Program {
    std::vector<UseDecl> uses;
    std::vector<Function> functions;
    std::vector<Stmt> globals;              // uses Stmt VarDecl entries
    std::vector<StructDecl> structs;
    std::vector<EnumDecl> enums;
    std::vector<ProtocolDecl> protocols;
    std::vector<ImplBlock> impls;
};

} // namespace drast
