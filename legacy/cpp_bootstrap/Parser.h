#pragma once

#include <vector>
#include "Ast.h"
#include "Token.h"

namespace drast {

class Parser final {
public:
    explicit Parser(std::vector<Token> tokens);
    Program parse();

private:
    std::vector<Token> tokens_;
    size_t current_ = 0;

    // ── Top-level dispatchers ─────────────────────────────────────
    enum class TopLevelKind { Function, GlobalAssign, GlobalUninit };
    UseDecl parseUse();
    void parseStruct(Program &program, bool isFileprivate);
    void parseEnum(Program &program, bool isFileprivate);
    void parseProtocol(Program &program);
    void parseImpl(Program &program);
    Function parseFunction(Token nameToken, bool insideImpl, const std::string &implTarget);
    Function parseOperator(Token operatorToken, const std::string &implTarget);
    Stmt parseGlobalAssign(Token nameToken);
    Stmt parseGlobalUninit(Token nameToken);
    TopLevelKind classifyTopLevel() const;

    // ── Statements ────────────────────────────────────────────────
    std::vector<StmtPtr> parseBlock();
    StmtPtr parseStatement();
    StmtPtr parseReturn();
    StmtPtr parseIf();
    StmtPtr parseWhile();
    StmtPtr parseFor();
    StmtPtr parseMatch();
    StmtPtr parseTryCatch();

    // Parse `name [const] [Type] = expr` / `name Type;` / assignment / expr-stmt.
    // Caller has either: nothing consumed yet, OR an identifier has already been read.
    StmtPtr parseIdentifierLeadStatement();

    // ── Types ─────────────────────────────────────────────────────
    Type parseType();

    // ── Expression precedence ladder ──────────────────────────────
    ExprPtr parseExpression();
    ExprPtr parseOr();
    ExprPtr parseAnd();
    ExprPtr parseEquality();
    ExprPtr parseComparison();
    ExprPtr parseTerm();
    ExprPtr parseFactor();
    ExprPtr parseUnary();
    ExprPtr parsePostfix();
    ExprPtr parsePrimary();
    ExprPtr maybeImplicitCall(ExprPtr callee);
    ExprPtr parseConstructorOrBracketCall(ExprPtr callee);
    ExprPtr parseArrayLiteralOrType();
    ExprPtr parseHeapAlloc(SourceLocation location);

    // ── Token helpers ─────────────────────────────────────────────
    bool match(TokenKind kind);
    bool check(TokenKind kind) const;
    Token advance();
    Token consume(TokenKind kind, const std::string &message);
    const Token &current() const;
    const Token &previous() const;
    const Token &peek(size_t offset = 1) const;

    void skipNewlines();
    void consumeStatementEnd();

    bool isPrimaryStart(TokenKind kind) const;
    bool isCalleeKind(const Expr &expr) const;

    // When > 0, postfix parsing must NOT wrap a bare identifier/field access in
    // an implicit call — the surrounding context is already gathering args.
    int inCallArg_ = 0;

    // Accept Identifier or a select set of soft-keywords (to/until/step/in)
    // in identifier-name positions like parameter names.
    Token consumeIdentLike(const std::string &message);

    // Heuristic: starts an upper-case identifier (probable Type) →
    // bracket-after-it is a constructor; lower-case → call-batch.
    static bool isTypeLikeIdentifier(const std::string &name);
};

} // namespace drast
