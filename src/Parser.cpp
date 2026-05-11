#include "Parser.h"

#include <cctype>
#include <cstdlib>
#include <utility>

namespace drast {

Parser::Parser(std::vector<Token> tokens) : tokens_(std::move(tokens)) {}

// ─────────────────────────────────────────────────────────────────────
//  Top-level program
// ─────────────────────────────────────────────────────────────────────

Program Parser::parse() {
    Program program;
    skipNewlines();
    while (!check(TokenKind::End)) {
        if (match(TokenKind::Use)) {
            program.uses.push_back(parseUse());
        } else if (match(TokenKind::Struct)) {
            parseStruct(program, /*fileprivate=*/false);
        } else if (match(TokenKind::Enum)) {
            parseEnum(program, /*fileprivate=*/false);
        } else if (match(TokenKind::Protocol)) {
            parseProtocol(program);
        } else if (match(TokenKind::Impl)) {
            parseImpl(program);
        } else if (match(TokenKind::Fileprivate)) {
            // `fileprivate enum X` / `fileprivate struct X`
            if (match(TokenKind::Enum)) {
                parseEnum(program, /*fileprivate=*/true);
            } else if (match(TokenKind::Struct)) {
                parseStruct(program, /*fileprivate=*/true);
            } else {
                throw CompileError("expected 'enum' or 'struct' after 'fileprivate'",
                                   current().location);
            }
        } else if (check(TokenKind::Identifier)) {
            TopLevelKind kind = classifyTopLevel();
            Token name = advance();
            switch (kind) {
                case TopLevelKind::Function:
                    program.functions.push_back(parseFunction(name, /*insideImpl=*/false, ""));
                    break;
                case TopLevelKind::GlobalAssign:
                    program.globals.push_back(parseGlobalAssign(name));
                    break;
                case TopLevelKind::GlobalUninit:
                    program.globals.push_back(parseGlobalUninit(name));
                    break;
            }
        } else {
            throw CompileError("unexpected token '" + current().text +
                               "' at top level",
                               current().location);
        }
        skipNewlines();
    }
    return program;
}

// ─────────────────────────────────────────────────────────────────────
//  use declaration
// ─────────────────────────────────────────────────────────────────────

UseDecl Parser::parseUse() {
    UseDecl decl;
    decl.location = previous().location;

    if (check(TokenKind::StringLiteral) || check(TokenKind::CharLiteral)) {
        decl.kind = UseDecl::Kind::Quoted;
        decl.path = advance().text;
    } else {
        decl.kind = UseDecl::Kind::Angle;
        std::string path;
        while (!check(TokenKind::Newline) && !check(TokenKind::End)) {
            const Token &t = current();
            if (t.kind == TokenKind::Identifier || t.kind == TokenKind::Dot) {
                path += t.text;
                advance();
            } else {
                throw CompileError("unexpected token '" + t.text + "' in use path",
                                   t.location);
            }
        }
        decl.path = path;
    }
    consumeStatementEnd();
    return decl;
}

// ─────────────────────────────────────────────────────────────────────
//  Top-level classification (Function vs. Global assign vs. Global uninit)
// ─────────────────────────────────────────────────────────────────────

Parser::TopLevelKind Parser::classifyTopLevel() const {
    // The leading Identifier has not yet been consumed.
    size_t i = current_ + 1;
    bool sawEqual = false;
    bool sawConst = false;
    bool sawSemicolonEol = false;
    int bracketDepth = 0;
    bool sawParamSemicolon = false;

    while (i < tokens_.size()) {
        TokenKind k = tokens_[i].kind;
        if (k == TokenKind::Newline || k == TokenKind::End ||
            k == TokenKind::Indent || k == TokenKind::Dedent) {
            if (i > 0 && tokens_[i - 1].kind == TokenKind::Semicolon) {
                sawSemicolonEol = true;
            }
            break;
        }
        if (bracketDepth == 0) {
            if (k == TokenKind::Equal) sawEqual = true;
            else if (k == TokenKind::Const) sawConst = true;
            else if (k == TokenKind::Semicolon) {
                if (i + 1 < tokens_.size() &&
                    tokens_[i + 1].kind == TokenKind::Identifier) {
                    sawParamSemicolon = true;
                }
            }
        }
        if (k == TokenKind::LeftBracket || k == TokenKind::LeftBrace ||
            k == TokenKind::LeftParen) bracketDepth++;
        if (k == TokenKind::RightBracket || k == TokenKind::RightBrace ||
            k == TokenKind::RightParen) bracketDepth--;
        ++i;
    }

    if (sawEqual || sawConst) return TopLevelKind::GlobalAssign;
    if (sawSemicolonEol && !sawParamSemicolon) return TopLevelKind::GlobalUninit;
    return TopLevelKind::Function;
}

// ─────────────────────────────────────────────────────────────────────
//  Function definition
// ─────────────────────────────────────────────────────────────────────

Function Parser::parseFunction(Token nameToken, bool insideImpl,
                               const std::string &implTarget) {
    Function function;
    function.name = nameToken.text;
    function.location = nameToken.location;
    if (insideImpl) function.implTarget = implTarget;

    // Optional generics: name`[T U]
    if (match(TokenKind::Backtick)) {
        consume(TokenKind::LeftBracket, "expected '[' after '`' in generic params");
        while (!check(TokenKind::RightBracket) && !check(TokenKind::End)) {
            if (check(TokenKind::Comma)) { advance(); continue; }
            function.typeParams.push_back(
                consume(TokenKind::Identifier, "expected generic type parameter").text);
        }
        consume(TokenKind::RightBracket, "expected ']' after generic params");
    }

    // Parameters: argName;Type [;default] ...
    while (!check(TokenKind::Comma) && !check(TokenKind::Newline) &&
           !check(TokenKind::End)) {
        Parameter param;
        Token nameTok = consumeIdentLike("expected parameter name");
        param.name = nameTok.text;
        param.location = nameTok.location;
        consume(TokenKind::Semicolon, "expected ';' after parameter name");
        param.type = parseType();
        if (match(TokenKind::Semicolon)) {
            param.defaultValue = parseExpression();
        }
        function.parameters.push_back(std::move(param));
    }

    // Optional return-type clause: , [maybe] Type [discard]
    if (match(TokenKind::Comma)) {
        if (match(TokenKind::Maybe)) {
            function.returnIsMaybe = true;
        }
        function.returnType = parseType();
        if (match(TokenKind::Discard)) {
            function.nodiscardSuppressed = true;
            function.returnType->isDiscard = true;
        }
    }

    consumeStatementEnd();
    skipNewlines();

    // Optional "with T as Trait, ..." constraints — appear on the line(s)
    // immediately *after* the function body in the spec, but we accept them
    // before the indented block for the common pattern. For v1 they're parsed
    // and silently dropped.
    // (A real impl would attach them as concept clauses.)

    // The body is only present when the next token is Indent.
    if (check(TokenKind::Indent)) {
        function.body = parseBlock();
    }

    // Trailing "with ..." constraints
    skipNewlines();
    if (check(TokenKind::With)) {
        // Consume the entire `with ...` line; v1 ignores semantics.
        while (!check(TokenKind::Newline) && !check(TokenKind::End)) advance();
        consumeStatementEnd();
    }

    return function;
}

Function Parser::parseOperator(Token opTok, const std::string &implTarget) {
    // We've already consumed `operator`. Next: `[` symbol `]`.
    consume(TokenKind::LeftBracket, "expected '[' after 'operator'");
    std::string sym;
    while (!check(TokenKind::RightBracket) && !check(TokenKind::End)) {
        sym += current().text;
        advance();
    }
    consume(TokenKind::RightBracket, "expected ']' after operator symbol");

    Function fn;
    fn.name = "operator" + sym;
    fn.operatorSymbol = sym;
    fn.isOperator = true;
    fn.implTarget = implTarget;
    fn.location = opTok.location;

    // Parameters identical to a normal function signature.
    while (!check(TokenKind::Comma) && !check(TokenKind::Newline) &&
           !check(TokenKind::End)) {
        Parameter param;
        Token nameTok = consumeIdentLike("expected parameter name");
        param.name = nameTok.text;
        param.location = nameTok.location;
        consume(TokenKind::Semicolon, "expected ';' after parameter name");
        param.type = parseType();
        if (match(TokenKind::Semicolon)) {
            param.defaultValue = parseExpression();
        }
        fn.parameters.push_back(std::move(param));
    }
    if (match(TokenKind::Comma)) {
        if (match(TokenKind::Maybe)) fn.returnIsMaybe = true;
        fn.returnType = parseType();
        if (match(TokenKind::Discard)) fn.returnType->isDiscard = true;
    }
    consumeStatementEnd();
    skipNewlines();
    if (check(TokenKind::Indent)) {
        fn.body = parseBlock();
    }
    return fn;
}

// ─────────────────────────────────────────────────────────────────────
//  Global vars
// ─────────────────────────────────────────────────────────────────────

Stmt Parser::parseGlobalAssign(Token nameToken) {
    Stmt stmt;
    stmt.kind = Stmt::Kind::VarDecl;
    stmt.location = nameToken.location;
    stmt.varName = nameToken.text;

    if (match(TokenKind::Const)) stmt.isConst = true;

    if (!check(TokenKind::Equal)) {
        // Optional explicit type before '='
        stmt.declaredType = parseType();
    }
    consume(TokenKind::Equal, "expected '=' in variable declaration");
    stmt.initializer = parseExpression();
    consumeStatementEnd();
    return stmt;
}

Stmt Parser::parseGlobalUninit(Token nameToken) {
    Stmt stmt;
    stmt.kind = Stmt::Kind::VarDecl;
    stmt.location = nameToken.location;
    stmt.varName = nameToken.text;
    stmt.declaredType = parseType();
    consume(TokenKind::Semicolon, "expected ';' for uninitialized declaration");
    consumeStatementEnd();
    return stmt;
}

// ─────────────────────────────────────────────────────────────────────
//  struct, enum, protocol, impl
// ─────────────────────────────────────────────────────────────────────

void Parser::parseStruct(Program &program, bool isFileprivate) {
    Token nameTok = consume(TokenKind::Identifier, "expected struct name");
    StructDecl decl;
    decl.name = nameTok.text;
    decl.location = nameTok.location;
    decl.isFileprivate = isFileprivate;
    // Optional generics: struct Name`[T U]
    if (match(TokenKind::Backtick)) {
        consume(TokenKind::LeftBracket, "expected '[' after '`' in struct generics");
        while (!check(TokenKind::RightBracket) && !check(TokenKind::End)) {
            if (check(TokenKind::Comma)) { advance(); continue; }
            decl.typeParams.push_back(
                consume(TokenKind::Identifier, "expected generic param").text);
        }
        consume(TokenKind::RightBracket, "expected ']'");
    }
    consumeStatementEnd();
    skipNewlines();
    consume(TokenKind::Indent, "expected indented block after struct header");
    while (!check(TokenKind::Dedent) && !check(TokenKind::End)) {
        Visibility vis = Visibility::Public;
        if (match(TokenKind::Preview)) vis = Visibility::Preview;
        else if (match(TokenKind::Private)) vis = Visibility::Private;
		else if (match(TokenKind::Fileprivate)) vis = Visibility::Fileprivate;

        // Allow `Field1; Field2;` shorthand on a single line for enum-like
        // tag flags (rare in structs but harmless).
        StructField field;
        Token nameField = consume(TokenKind::Identifier, "expected field name");
        field.name = nameField.text;
        field.location = nameField.location;
        field.visibility = vis;
        field.type = parseType();
        // Trailing semicolon optional (per syntax: `preview x float;`)
        match(TokenKind::Semicolon);
        decl.fields.push_back(std::move(field));
        consumeStatementEnd();
        skipNewlines();
    }
    consume(TokenKind::Dedent, "expected end of struct block");
    program.structs.push_back(std::move(decl));
}

void Parser::parseEnum(Program &program, bool isFileprivate) {
    Token nameTok = consume(TokenKind::Identifier, "expected enum name");
    std::string fullName = nameTok.text;
    // Nested enums: `enum Outer.Inner`
    while (match(TokenKind::Dot)) {
        fullName += "." +
            consume(TokenKind::Identifier, "expected nested enum name").text;
    }
    EnumDecl decl;
    decl.name = fullName;
    decl.location = nameTok.location;
    decl.isFileprivate = isFileprivate;
    consumeStatementEnd();
    skipNewlines();
    consume(TokenKind::Indent, "expected indented block after enum header");
    while (!check(TokenKind::Dedent) && !check(TokenKind::End)) {
        // Allow multiple variants per line, semicolon-separated.
        while (check(TokenKind::Identifier)) {
            EnumVariant variant;
            variant.name = current().text;
            variant.location = current().location;
            advance();
            if (check(TokenKind::Identifier) && peek(1).kind == TokenKind::Semicolon) {
                while (check(TokenKind::Identifier) && peek(1).kind == TokenKind::Semicolon) {
                    StructField field;
                    Token fieldTok = advance();
                    field.name = fieldTok.text;
                    field.location = fieldTok.location;
                    consume(TokenKind::Semicolon, "expected ';' after enum payload field name");
                    field.type = parseType();
                    variant.fields.push_back(std::move(field));
                }
            } else {
                // Multi-word variant names like `Golden Retriver` — accept
                // additional identifiers on the same line until we hit ';' or newline.
                while (check(TokenKind::Identifier)) {
                    variant.name += "_" + current().text;
                    advance();
                }
            }
            decl.variants.push_back(std::move(variant));
            if (!match(TokenKind::Semicolon)) break;
        }
        consumeStatementEnd();
        skipNewlines();
    }
    consume(TokenKind::Dedent, "expected end of enum block");
    program.enums.push_back(std::move(decl));
}

void Parser::parseProtocol(Program &program) {
    Token nameTok = consume(TokenKind::Identifier, "expected protocol name");
    ProtocolDecl decl;
    decl.name = nameTok.text;
    decl.location = nameTok.location;
    consumeStatementEnd();
    skipNewlines();
    consume(TokenKind::Indent, "expected indented block after protocol header");
    while (!check(TokenKind::Dedent) && !check(TokenKind::End)) {
        ProtocolMethod method;
        Token nameMethod = consume(TokenKind::Identifier, "expected method name");
        method.name = nameMethod.text;
        method.location = nameMethod.location;
        // Params (optional)
        while (!check(TokenKind::Comma) && !check(TokenKind::Newline) &&
               !check(TokenKind::End)) {
            Parameter param;
            Token pn = consumeIdentLike("expected parameter name");
            param.name = pn.text;
            param.location = pn.location;
            consume(TokenKind::Semicolon, "expected ';' after parameter name");
            param.type = parseType();
            method.parameters.push_back(std::move(param));
        }
        if (match(TokenKind::Comma)) {
            if (match(TokenKind::Maybe)) method.returnIsMaybe = true;
            method.returnType = parseType();
        }
        consumeStatementEnd();
        skipNewlines();
        decl.methods.push_back(std::move(method));
    }
    consume(TokenKind::Dedent, "expected end of protocol block");
    program.protocols.push_back(std::move(decl));
}

void Parser::parseImpl(Program &program) {
    Token typeName = consume(TokenKind::Identifier, "expected type name after 'impl'");
    ImplBlock impl;
    impl.targetType = typeName.text;
    impl.location = typeName.location;

    if (match(TokenKind::As)) {
        impl.protocolName =
            consume(TokenKind::Identifier, "expected protocol name after 'as'").text;
    }
    consumeStatementEnd();
    skipNewlines();
    consume(TokenKind::Indent, "expected indented block after impl header");
    while (!check(TokenKind::Dedent) && !check(TokenKind::End)) {
        // Method visibility prefix is allowed (e.g. `private updateQuadrants`).
        Visibility vis = Visibility::Public;
        if (match(TokenKind::Private)) vis = Visibility::Private;
        else if (match(TokenKind::Preview)) vis = Visibility::Preview;

        if (match(TokenKind::Operator)) {
            Token opTok = previous();
            Function fn = parseOperator(opTok, impl.targetType);
            fn.visibility = vis;
            impl.methods.push_back(std::move(fn));
        } else {
            Token nameTok = consume(TokenKind::Identifier, "expected method name in impl");
            Function fn = parseFunction(nameTok, /*insideImpl=*/true, impl.targetType);
            fn.visibility = vis;
            impl.methods.push_back(std::move(fn));
        }
        skipNewlines();
    }
    consume(TokenKind::Dedent, "expected end of impl block");
    program.impls.push_back(std::move(impl));
}

// ─────────────────────────────────────────────────────────────────────
//  Blocks & statements
// ─────────────────────────────────────────────────────────────────────

std::vector<StmtPtr> Parser::parseBlock() {
    consume(TokenKind::Indent, "expected indented block");
    std::vector<StmtPtr> statements;
    while (!check(TokenKind::Dedent) && !check(TokenKind::End)) {
        statements.push_back(parseStatement());
        // Block-structured statements self-terminate (consume their own dedent).
        Stmt::Kind k = statements.back()->kind;
        if (k != Stmt::Kind::If && k != Stmt::Kind::While &&
            k != Stmt::Kind::ForIn && k != Stmt::Kind::Match &&
            k != Stmt::Kind::TryCatch) {
            consumeStatementEnd();
        }
        skipNewlines();
    }
    consume(TokenKind::Dedent, "expected end of indented block");
    return statements;
}

StmtPtr Parser::parseStatement() {
    if (match(TokenKind::Return)) return parseReturn();
    if (match(TokenKind::Break)) {
        auto s = std::make_unique<Stmt>();
        s->kind = Stmt::Kind::Break;
        s->location = previous().location;
        return s;
    }
    if (match(TokenKind::Continue)) {
        auto s = std::make_unique<Stmt>();
        s->kind = Stmt::Kind::Continue;
        s->location = previous().location;
        return s;
    }
    if (check(TokenKind::If)) return parseIf();
    if (check(TokenKind::While)) return parseWhile();
    if (check(TokenKind::For)) return parseFor();
    if (check(TokenKind::Match)) return parseMatch();
    if (check(TokenKind::Try)) return parseTryCatch();
    // Identifier-led statements, including soft-keywords that double as names
    // (to/until/step/in may be used as parameter / variable names).
    if (check(TokenKind::Identifier) || check(TokenKind::Self_) ||
        check(TokenKind::To) || check(TokenKind::Until) ||
        check(TokenKind::Step) || check(TokenKind::In)) {
        return parseIdentifierLeadStatement();
    }
    // Fallthrough: parse as expression statement.
    auto stmt = std::make_unique<Stmt>();
    stmt->kind = Stmt::Kind::Expression;
    stmt->location = current().location;
    stmt->expression = parseExpression();
    return stmt;
}

// Handles:
//   name = expr             (var-decl-with-inference / reassignment — emitted as auto-decl)
//   name const = expr       (immutable inferred)
//   name Type;              (uninit typed local)
//   name Type = expr        (typed init)
//   name `[T]` = expr       (var-decl with generic type instantiation)
//   name.x.y = expr / .x += expr  (assignment to lvalue chain)
//   plain expression-statement
StmtPtr Parser::parseIdentifierLeadStatement() {
    size_t saved = current_;

    bool startsWithSelf = check(TokenKind::Self_);
    Token nameTok = advance();
    std::string lvalueText = nameTok.text;

    // Build dot-chain target text for codegen-string assignment (kept simple).
    bool hasChain = false;
    while (check(TokenKind::Dot) && peek(1).kind == TokenKind::Identifier) {
        advance();
        hasChain = true;
        lvalueText += "." + advance().text;
    }

    // ── Assignment to dot-chain or simple name ──
    if (check(TokenKind::Equal) || check(TokenKind::PlusEqual) ||
        check(TokenKind::MinusEqual) || check(TokenKind::StarEqual) ||
        check(TokenKind::SlashEqual)) {

        // For local var-decl-with-inference: bare name (no chain, not self), '='
        if (!hasChain && !startsWithSelf && check(TokenKind::Equal)) {
            advance(); // consume '='
            auto stmt = std::make_unique<Stmt>();
            stmt->kind = Stmt::Kind::VarDecl;
            stmt->location = nameTok.location;
            stmt->varName = nameTok.text;
            stmt->initializer = parseExpression();
            return stmt;
        }

        // Otherwise it's an assignment to an existing lvalue.
        Token op = advance();
        auto stmt = std::make_unique<Stmt>();
        stmt->kind = Stmt::Kind::Assign;
        stmt->location = nameTok.location;

        // Rebuild target as an expression for indexed/field cases.
        auto target = std::make_unique<Expr>();
        if (startsWithSelf) {
            target->kind = Expr::Kind::Identifier;
            target->text = "self";
            target->location = nameTok.location;
        } else {
            target->kind = Expr::Kind::Identifier;
            target->text = nameTok.text;
            target->location = nameTok.location;
        }
        // Re-apply dot chain
        std::string chainAccum = startsWithSelf ? std::string("self") : nameTok.text;
        size_t pos = chainAccum.size();
        for (size_t i = pos + 1; i < lvalueText.size();) {
            size_t end = lvalueText.find('.', i);
            std::string field = lvalueText.substr(
                i, (end == std::string::npos ? lvalueText.size() : end) - i);
            auto fa = std::make_unique<Expr>();
            fa->kind = Expr::Kind::FieldAccess;
            fa->left = std::move(target);
            fa->text = field;
            fa->location = nameTok.location;
            target = std::move(fa);
            if (end == std::string::npos) break;
            i = end + 1;
        }
        stmt->assignTarget = std::move(target);
        if (op.kind != TokenKind::Equal) {
            stmt->compoundOp = op.text.substr(0, op.text.size() - 1);
        }
        stmt->value = parseExpression();
        return stmt;
    }

    // ── name const [= ...] : local immutable var-decl ──
    if (!hasChain && !startsWithSelf && check(TokenKind::Const)) {
        advance(); // consume const
        auto stmt = std::make_unique<Stmt>();
        stmt->kind = Stmt::Kind::VarDecl;
        stmt->location = nameTok.location;
        stmt->varName = nameTok.text;
        stmt->isConst = true;
        if (!check(TokenKind::Equal)) {
            stmt->declaredType = parseType();
        }
        consume(TokenKind::Equal, "expected '=' after 'const'");
        stmt->initializer = parseExpression();
        return stmt;
    }

    // ── name Type [`[U]`] = expr  /  name Type;  (typed local) ──
    // Type can begin with: Identifier, `{` (array type), `~` (reference).
    bool typeStarter = check(TokenKind::Identifier) ||
                       check(TokenKind::LeftBrace) ||
                       check(TokenKind::AtLeftBracket) ||
                       check(TokenKind::Tilde);
    if (!hasChain && !startsWithSelf && typeStarter) {
        // Peek: name followed by another identifier — is this `name Type;` or
        //       `name Type = expr` or expression-statement starting with name?
        // Heuristic: scan to newline; if we see `;` before any other operator,
        //            uninit-typed; if we see `=`, typed-init; otherwise it's
        //            an expression call.
        size_t scan = current_;
        bool typed = false;
        bool typedInit = false;
        int depth = 0;
        while (scan < tokens_.size()) {
            TokenKind k = tokens_[scan].kind;
            if (k == TokenKind::Newline || k == TokenKind::End ||
                k == TokenKind::Indent || k == TokenKind::Dedent) break;
            if (k == TokenKind::LeftBracket || k == TokenKind::LeftBrace ||
                k == TokenKind::LeftParen) depth++;
            else if (k == TokenKind::RightBracket || k == TokenKind::RightBrace ||
                     k == TokenKind::RightParen) depth--;
            if (depth == 0) {
                if (k == TokenKind::Equal) { typed = true; typedInit = true; break; }
                if (k == TokenKind::Semicolon &&
                    scan + 1 < tokens_.size() &&
                    (tokens_[scan + 1].kind == TokenKind::Newline ||
                     tokens_[scan + 1].kind == TokenKind::End ||
                     tokens_[scan + 1].kind == TokenKind::Dedent)) {
                    typed = true; break;
                }
            }
            ++scan;
        }
        if (typed) {
            auto stmt = std::make_unique<Stmt>();
            stmt->kind = Stmt::Kind::VarDecl;
            stmt->location = nameTok.location;
            stmt->varName = nameTok.text;
            stmt->declaredType = parseType();
            if (typedInit) {
                consume(TokenKind::Equal, "expected '=' after type");
                stmt->initializer = parseExpression();
            } else {
                consume(TokenKind::Semicolon, "expected ';' for uninit declaration");
            }
            return stmt;
        }
    }

    // ── Otherwise: expression statement (possibly a call). Rewind and reparse. ──
    current_ = saved;
    auto stmt = std::make_unique<Stmt>();
    stmt->kind = Stmt::Kind::Expression;
    stmt->location = current().location;
    stmt->expression = parseExpression();
    // A bare Identifier or FieldAccess sitting alone as a statement-level
    // expression is, per spec, a zero-argument call (e.g. `mySensor.setup`).
    if (stmt->expression &&
        (stmt->expression->kind == Expr::Kind::Identifier ||
         stmt->expression->kind == Expr::Kind::FieldAccess)) {
        auto call = std::make_unique<Expr>();
        call->kind = Expr::Kind::Call;
        call->location = stmt->expression->location;
        call->callee = std::move(stmt->expression);
        stmt->expression = std::move(call);
    }
    return stmt;
}

StmtPtr Parser::parseReturn() {
    auto stmt = std::make_unique<Stmt>();
    stmt->kind = Stmt::Kind::Return;
    stmt->location = previous().location;
    if (!check(TokenKind::Newline) && !check(TokenKind::Dedent) &&
        !check(TokenKind::End)) {
        stmt->expression = parseExpression();
    }
    return stmt;
}

StmtPtr Parser::parseIf() {
    consume(TokenKind::If, "expected 'if'");
    auto stmt = std::make_unique<Stmt>();
    stmt->kind = Stmt::Kind::If;
    stmt->location = previous().location;
    stmt->condition = parseExpression();
    consumeStatementEnd();
    skipNewlines();
    stmt->thenBody = parseBlock();
    skipNewlines();
    if (match(TokenKind::Elif)) {
        // Treat elif as `else { if ... }` by recursively parsing one more if-chain.
        auto chain = std::make_unique<Stmt>();
        chain->kind = Stmt::Kind::If;
        chain->location = previous().location;
        chain->condition = parseExpression();
        consumeStatementEnd();
        skipNewlines();
        chain->thenBody = parseBlock();
        // Continue the elif/else chain recursively.
        // (Simple trick: stash position and let parseIf logic for else continue here.)
        skipNewlines();
        while (match(TokenKind::Elif)) {
            auto next = std::make_unique<Stmt>();
            next->kind = Stmt::Kind::If;
            next->location = previous().location;
            next->condition = parseExpression();
            consumeStatementEnd();
            skipNewlines();
            next->thenBody = parseBlock();
            // Attach this as the elseBody of the previous chain — find the deepest
            // chain element via linear walk.
            Stmt *cursor = chain.get();
            while (!cursor->elseBody.empty() &&
                   cursor->elseBody.front()->kind == Stmt::Kind::If) {
                cursor = cursor->elseBody.front().get();
            }
            cursor->elseBody.push_back(std::move(next));
            skipNewlines();
        }
        if (match(TokenKind::Else)) {
            consumeStatementEnd();
            skipNewlines();
            Stmt *cursor = chain.get();
            while (!cursor->elseBody.empty() &&
                   cursor->elseBody.front()->kind == Stmt::Kind::If) {
                cursor = cursor->elseBody.front().get();
            }
            cursor->elseBody = parseBlock();
        }
        stmt->elseBody.push_back(std::move(chain));
    } else if (match(TokenKind::Else)) {
        consumeStatementEnd();
        skipNewlines();
        stmt->elseBody = parseBlock();
    }
    return stmt;
}

StmtPtr Parser::parseWhile() {
    consume(TokenKind::While, "expected 'while'");
    auto stmt = std::make_unique<Stmt>();
    stmt->kind = Stmt::Kind::While;
    stmt->location = previous().location;
    stmt->condition = parseExpression();
    consumeStatementEnd();
    skipNewlines();
    stmt->thenBody = parseBlock();
    return stmt;
}

StmtPtr Parser::parseFor() {
    consume(TokenKind::For, "expected 'for'");
    auto stmt = std::make_unique<Stmt>();
    stmt->kind = Stmt::Kind::ForIn;
    stmt->location = previous().location;
    stmt->loopVar = consume(TokenKind::Identifier, "expected loop variable").text;
    consume(TokenKind::In, "expected 'in' after for-variable");
    ExprPtr first = parseExpression();
    if (check(TokenKind::To) || check(TokenKind::Until)) {
        bool inclusive = check(TokenKind::To);
        advance();
        stmt->rangeInclusive = inclusive;
        stmt->rangeStart = std::move(first);
        stmt->rangeEnd = parseExpression();
        if (match(TokenKind::Step)) {
            stmt->rangeStep = parseExpression();
        }
    } else {
        stmt->iterable = std::move(first);
    }
    consumeStatementEnd();
    skipNewlines();
    stmt->thenBody = parseBlock();
    return stmt;
}

StmtPtr Parser::parseMatch() {
    consume(TokenKind::Match, "expected 'match'");
    auto stmt = std::make_unique<Stmt>();
    stmt->kind = Stmt::Kind::Match;
    stmt->location = previous().location;
    stmt->expression = parseExpression();
    consumeStatementEnd();
    skipNewlines();
    consume(TokenKind::Indent, "expected indented block after 'match' expression");
    while (!check(TokenKind::Dedent) && !check(TokenKind::End)) {
        MatchArm arm;
        arm.location = current().location;
        if (match(TokenKind::Default)) {
            arm.isDefault = true;
        } else {
            arm.pattern = parseExpression();
        }
        consumeStatementEnd();
        skipNewlines();
        arm.body = parseBlock();
        stmt->matchArms.push_back(std::move(arm));
        skipNewlines();
    }
    consume(TokenKind::Dedent, "expected end of match block");
    return stmt;
}

StmtPtr Parser::parseTryCatch() {
    consume(TokenKind::Try, "expected 'try'");
    auto stmt = std::make_unique<Stmt>();
    stmt->kind = Stmt::Kind::TryCatch;
    stmt->location = previous().location;
    consumeStatementEnd();
    skipNewlines();
    stmt->tryBody = parseBlock();
    skipNewlines();
    consume(TokenKind::Catch, "expected 'catch' after 'try' block");
    // Optional binding: catch ;name
    if (match(TokenKind::Semicolon)) {
        stmt->catchBinding =
            consume(TokenKind::Identifier, "expected catch binding name").text;
    }
    consumeStatementEnd();
    skipNewlines();
    stmt->catchBody = parseBlock();
    return stmt;
}

// ─────────────────────────────────────────────────────────────────────
//  Types
// ─────────────────────────────────────────────────────────────────────

Type Parser::parseType() {
    Type type;
    type.location = current().location;

    if (match(TokenKind::Tilde)) {
        // ~T  → pass-by-reference parameter
        type = parseType();
        type.isReference = true;
        return type;
    }

    if (match(TokenKind::LeftBrace)) {
        // {T} → array
        type.isArray = true;
        Type inner = parseType();
        type.name = inner.name;
        type.typeArgs = std::move(inner.typeArgs);
        type.pointerDepth = inner.pointerDepth;
        consume(TokenKind::RightBrace, "expected '}' in array type");
        return type;
    }

    if (match(TokenKind::AtLeftBracket)) {
        Type inner = parseType();
        consume(TokenKind::RightBracket, "expected ']' after heap-owned type");
        inner.heapKind = HeapKind::Shared;
        return inner;
    }

    if (match(TokenKind::Variadic)) {
        consume(TokenKind::LeftBracket, "expected '[' after 'variadic'");
        Type inner = parseType();
        consume(TokenKind::RightBracket, "expected ']' after variadic element type");
        inner.isVariadic = true;
        return inner;
    }

    if (match(TokenKind::Tuple)) {
        Type t;
        t.isTuple = true;
        // Space-separated element types until end-of-signature.
        while (check(TokenKind::Identifier)) {
            t.typeArgs.push_back(parseType());
        }
        return t;
    }

    Token nameTok = consume(TokenKind::Identifier, "expected type name");
    type.name = nameTok.text;
    // Dotted / namespaced names like Dog.Breed, std::list, std.list.
    while (true) {
        if (match(TokenKind::Dot)) {
            type.name += "." +
                consume(TokenKind::Identifier, "expected identifier after '.'").text;
        } else if (match(TokenKind::DoubleColon)) {
            type.name += "::" +
                consume(TokenKind::Identifier, "expected identifier after '::'").text;
        } else {
            break;
        }
    }

    // Generic args appear only as ``[…]`` — bare backtick is a pointer suffix.
    while (check(TokenKind::Backtick)) {
        if (peek(1).kind == TokenKind::LeftBracket) {
            advance(); // `
            advance(); // [
            while (!check(TokenKind::RightBracket) && !check(TokenKind::End)) {
                if (check(TokenKind::Comma)) { advance(); continue; }
                type.typeArgs.push_back(parseType());
            }
            consume(TokenKind::RightBracket, "expected ']' after generic args");
        } else {
            advance();
            type.pointerDepth++;
        }
    }
    return type;
}

// ─────────────────────────────────────────────────────────────────────
//  Expression precedence ladder
// ─────────────────────────────────────────────────────────────────────

ExprPtr Parser::parseExpression() { return parseOr(); }

static std::string binaryOpFor(TokenKind kind) {
    switch (kind) {
        case TokenKind::Plus: return "+";
        case TokenKind::Minus: return "-";
        case TokenKind::Star: return "*";
        case TokenKind::Slash: return "/";
        case TokenKind::Percent: return "%";
        case TokenKind::EqualEqual: return "==";
        case TokenKind::Iseq: return "==";
        case TokenKind::Isne: return "!=";
        case TokenKind::Islt: return "<";
        case TokenKind::Isgt: return ">";
        case TokenKind::Islteq: return "<=";
        case TokenKind::Isgteq: return ">=";
        case TokenKind::And: return "&&";
        case TokenKind::Or: return "||";
        default: return "";
    }
}

static ExprPtr makeBinary(ExprPtr left, const Token &op, ExprPtr right) {
    auto node = std::make_unique<Expr>();
    node->kind = Expr::Kind::Binary;
    node->location = op.location;
    node->opText = binaryOpFor(op.kind);
    node->left = std::move(left);
    node->right = std::move(right);
    return node;
}

ExprPtr Parser::parseOr() {
    auto left = parseAnd();
    while (check(TokenKind::Or)) {
        Token op = advance();
        auto right = parseAnd();
        left = makeBinary(std::move(left), op, std::move(right));
    }
    return left;
}

ExprPtr Parser::parseAnd() {
    auto left = parseEquality();
    while (check(TokenKind::And)) {
        Token op = advance();
        auto right = parseEquality();
        left = makeBinary(std::move(left), op, std::move(right));
    }
    return left;
}

ExprPtr Parser::parseEquality() {
    auto left = parseComparison();
    while (check(TokenKind::EqualEqual) || check(TokenKind::Iseq) ||
           check(TokenKind::Isne)) {
        Token op = advance();
        auto right = parseComparison();
        left = makeBinary(std::move(left), op, std::move(right));
    }
    return left;
}

ExprPtr Parser::parseComparison() {
    auto left = parseTerm();
    while (check(TokenKind::Islt) || check(TokenKind::Isgt) ||
           check(TokenKind::Islteq) || check(TokenKind::Isgteq)) {
        Token op = advance();
        auto right = parseTerm();
        left = makeBinary(std::move(left), op, std::move(right));
    }
    return left;
}

ExprPtr Parser::parseTerm() {
    auto left = parseFactor();
    while (check(TokenKind::Plus) || check(TokenKind::Minus)) {
        Token op = advance();
        auto right = parseFactor();
        left = makeBinary(std::move(left), op, std::move(right));
    }
    return left;
}

ExprPtr Parser::parseFactor() {
    auto left = parseUnary();
    while (check(TokenKind::Star) || check(TokenKind::Slash) ||
           check(TokenKind::Percent)) {
        Token op = advance();
        auto right = parseUnary();
        left = makeBinary(std::move(left), op, std::move(right));
    }
    return left;
}

ExprPtr Parser::parseUnary() {
    if (check(TokenKind::Not) || check(TokenKind::Minus)) {
        Token op = advance();
        auto operand = parseUnary();
        auto node = std::make_unique<Expr>();
        node->kind = Expr::Kind::Unary;
        node->location = op.location;
        node->opText = (op.kind == TokenKind::Not) ? "!" : "-";
        node->left = std::move(operand);
        return node;
    }
    if (match(TokenKind::Tilde)) {
        auto operand = parseUnary();
        auto node = std::make_unique<Expr>();
        node->kind = Expr::Kind::Ref;
        node->location = previous().location;
        node->left = std::move(operand);
        return node;
    }
    if (match(TokenKind::Backtick)) {
        // Prefix backtick = deref. (When backtick appears as a *type* suffix
        // it's already consumed by parseType.)
        auto operand = parseUnary();
        auto node = std::make_unique<Expr>();
        node->kind = Expr::Kind::Deref;
        node->location = previous().location;
        node->left = std::move(operand);
        return node;
    }
    return parsePostfix();
}

ExprPtr Parser::parsePostfix() {
    auto expr = parsePrimary();
    while (true) {
        if (match(TokenKind::Dot)) {
            // Could be a normal field access or an enum shorthand inside chains
            // (we don't expect `.X` inside chains, but we handle dot+identifier).
            if (check(TokenKind::Identifier) && isTypeLikeIdentifier(current().text) &&
                expr->kind == Expr::Kind::FieldAccess) {
                current_--;
                break;
            }
            Token member = consume(TokenKind::Identifier,
                                    "expected identifier after '.'");
            auto node = std::make_unique<Expr>();
            node->kind = Expr::Kind::FieldAccess;
            node->location = member.location;
            node->left = std::move(expr);
            node->text = member.text;
            expr = std::move(node);
            continue;
        }
        if (match(TokenKind::LeftBrace)) {
            // arr{idx} indexing
            auto idx = parseExpression();
            consume(TokenKind::RightBrace, "expected '}' after index");
            auto node = std::make_unique<Expr>();
            node->kind = Expr::Kind::Index;
            node->location = expr->location;
            node->left = std::move(expr);
            node->right = std::move(idx);
            expr = std::move(node);
            continue;
        }
        if (check(TokenKind::LeftBracket) && isCalleeKind(*expr)) {
            // Type[args] constructor OR func[a, b] batched call.
            expr = parseConstructorOrBracketCall(std::move(expr));
            continue;
        }
        // Generic instantiation in expression position: name`[T] args
        if (check(TokenKind::Backtick) &&
            peek(1).kind == TokenKind::LeftBracket && isCalleeKind(*expr)) {
            advance(); // `
            advance(); // [
            while (!check(TokenKind::RightBracket) && !check(TokenKind::End)) {
                if (check(TokenKind::Comma)) { advance(); continue; }
                expr->genericArgs.push_back(parseType());
            }
            consume(TokenKind::RightBracket, "expected ']' after generic args");
            continue;
        }
        break;
    }
    // Implicit space-separated call: print 'x', mySensor.setup
    return maybeImplicitCall(std::move(expr));
}

bool Parser::isCalleeKind(const Expr &expr) const {
    return expr.kind == Expr::Kind::Identifier ||
           expr.kind == Expr::Kind::FieldAccess;
}

bool Parser::isPrimaryStart(TokenKind kind) const {
    switch (kind) {
        case TokenKind::IntLiteral:
        case TokenKind::FloatLiteral:
        case TokenKind::StringLiteral:
        case TokenKind::CharLiteral:
        case TokenKind::Identifier:
        case TokenKind::True:
        case TokenKind::False:
        case TokenKind::Nil:
        case TokenKind::Self_:
        case TokenKind::LeftParen:
        case TokenKind::LeftBrace:      // array literal {a b c}
        case TokenKind::AtLeftBracket:
        case TokenKind::Tilde:          // ~x reference
        case TokenKind::Backtick:       // `x deref / .X enum-shorthand follower
        case TokenKind::Dot:            // .Variant (enum shorthand)
        case TokenKind::Semicolon:      // ;label  labeled call arg
            return true;
        default:
            return false;
    }
}

ExprPtr Parser::maybeImplicitCall(ExprPtr callee) {
    if (inCallArg_ > 0) return callee;  // outer call is gathering args
    if (!isCalleeKind(*callee)) return callee;
    if (!isPrimaryStart(current().kind)) return callee;

    auto call = std::make_unique<Expr>();
    call->kind = Expr::Kind::Call;
    call->location = callee->location;
    call->callee = std::move(callee);
    inCallArg_++;
    while (isPrimaryStart(current().kind)) {
        // Skip labeled-arg prefix: `;name expr` — drop the label.
        if (check(TokenKind::Semicolon) &&
            (peek(1).kind == TokenKind::Identifier ||
             peek(1).kind == TokenKind::To || peek(1).kind == TokenKind::Until ||
             peek(1).kind == TokenKind::Step || peek(1).kind == TokenKind::In)) {
            advance(); advance();
        }
        call->args.push_back(parseUnary());
    }
    inCallArg_--;
    return call;
}

ExprPtr Parser::parseConstructorOrBracketCall(ExprPtr callee) {
    consume(TokenKind::LeftBracket, "expected '['");
    // Decide constructor vs call-batch by callee identifier's leading case.
    bool typeLike = false;
    if (callee->kind == Expr::Kind::Identifier) {
        typeLike = isTypeLikeIdentifier(callee->text);
    } else if (callee->kind == Expr::Kind::FieldAccess) {
        typeLike = isTypeLikeIdentifier(callee->text);
    }

    if (typeLike) {
        auto node = std::make_unique<Expr>();
        node->kind = Expr::Kind::ConstructorCall;
        node->location = callee->location;
        node->callee = std::move(callee);
        // Space-separated args.  Bump inCallArg_ so each item is parsed as
        // a single unary expression rather than greedily slurping
        // subsequent primaries into an implicit call.
        inCallArg_++;
        while (!check(TokenKind::RightBracket) && !check(TokenKind::End)) {
            ConstructorArg arg;
            if (match(TokenKind::Semicolon)) {
                arg.label =
                    consume(TokenKind::Identifier, "expected label after ';'").text;
            }
            arg.value = parseUnary();
            node->ctorArgs.push_back(std::move(arg));
            match(TokenKind::Comma); // optional comma separator
        }
        inCallArg_--;
        consume(TokenKind::RightBracket, "expected ']' after constructor args");
        return node;
    }

    // Function call with brackets: comma-separated arg batches.
    auto node = std::make_unique<Expr>();
    node->kind = Expr::Kind::BracketCallBatch;
    node->location = callee->location;
    node->callee = std::move(callee);

    // Parse batches. Indent/Dedent inside the bracket region are ignored as
    // pure layout noise — `[ ... ]` is a single logical expression.
    auto skipLayout = [&]() {
        while (check(TokenKind::Newline) || check(TokenKind::Indent) ||
               check(TokenKind::Dedent)) {
            advance();
        }
    };
    skipLayout();
    std::vector<ExprPtr> batch;
    // Each space-separated item inside the brackets is a single argument;
    // suppress implicit-call gathering so `foo[bar baz]` parses as a call
    // with two args rather than one arg `bar(baz)`.
    inCallArg_++;
    while (!check(TokenKind::RightBracket) && !check(TokenKind::End)) {
        if (match(TokenKind::Comma)) {
            if (!batch.empty()) {
                node->batches.push_back(std::move(batch));
                batch.clear();
            }
            skipLayout();
            continue;
        }
        if (check(TokenKind::Newline) || check(TokenKind::Indent) ||
            check(TokenKind::Dedent)) { advance(); continue; }
        // Labeled positional: ;label value — drop the label, keep the value.
        if (check(TokenKind::Semicolon) &&
            (peek(1).kind == TokenKind::Identifier ||
             peek(1).kind == TokenKind::To || peek(1).kind == TokenKind::Until ||
             peek(1).kind == TokenKind::Step || peek(1).kind == TokenKind::In)) {
            advance(); advance();
        }
        batch.push_back(parseUnary());
    }
    inCallArg_--;
    if (!batch.empty()) node->batches.push_back(std::move(batch));
    consume(TokenKind::RightBracket, "expected ']' at end of call");
    return node;
}

ExprPtr Parser::parseArrayLiteralOrType() {
    // Already consumed '{'.
    auto node = std::make_unique<Expr>();
    node->kind = Expr::Kind::ArrayLiteral;
    node->location = previous().location;
    skipNewlines();
    while (!check(TokenKind::RightBrace) && !check(TokenKind::End)) {
        if (check(TokenKind::Newline)) { advance(); continue; }
        if (match(TokenKind::Comma)) continue;
        node->args.push_back(parseUnary());
    }
    consume(TokenKind::RightBrace, "expected '}' after array literal");
    return node;
}

ExprPtr Parser::parseHeapAlloc(SourceLocation location) {
    auto node = std::make_unique<Expr>();
    node->kind = Expr::Kind::HeapAlloc;
    node->location = location;
    node->text = "shared";
    node->heapType = parseType();
    while (!check(TokenKind::RightBracket) && !check(TokenKind::End)) {
        ConstructorArg arg;
        if (match(TokenKind::Semicolon)) {
            arg.label = consume(TokenKind::Identifier, "expected label after ';'").text;
        }
        arg.value = parseUnary();
        node->ctorArgs.push_back(std::move(arg));
        match(TokenKind::Comma);
    }
    consume(TokenKind::RightBracket, "expected ']' after heap allocation");
    return node;
}

ExprPtr Parser::parsePrimary() {
    Token tok = current();
    auto expr = std::make_unique<Expr>();
    expr->location = tok.location;

    switch (tok.kind) {
        case TokenKind::IntLiteral: {
            advance();
            expr->kind = Expr::Kind::IntLiteral;
            expr->text = tok.text;
            return expr;
        }
        case TokenKind::FloatLiteral: {
            advance();
            expr->kind = Expr::Kind::FloatLiteral;
            expr->text = tok.text;
            return expr;
        }
        case TokenKind::StringLiteral: {
            advance();
            expr->kind = Expr::Kind::StringLiteral;
            expr->text = tok.text;
            return expr;
        }
        case TokenKind::CharLiteral: {
            advance();
            expr->kind = Expr::Kind::CharLiteral;
            expr->text = tok.text;
            return expr;
        }
        case TokenKind::True:
            advance();
            expr->kind = Expr::Kind::BoolLiteral;
            expr->boolValue = true;
            return expr;
        case TokenKind::False:
            advance();
            expr->kind = Expr::Kind::BoolLiteral;
            expr->boolValue = false;
            return expr;
        case TokenKind::Nil:
            advance();
            expr->kind = Expr::Kind::NilLiteral;
            return expr;
        case TokenKind::Self_:
            advance();
            expr->kind = Expr::Kind::Identifier;
            expr->text = "self";
            return expr;
        case TokenKind::Identifier:
        case TokenKind::To:
        case TokenKind::Until:
        case TokenKind::Step:
        case TokenKind::In:
            advance();
            expr->kind = Expr::Kind::Identifier;
            expr->text = tok.text;
            return expr;
        case TokenKind::Tuple:
            // `tuple a b c`  → std::make_tuple(a, b, c)
            advance();
            expr->kind = Expr::Kind::Identifier;
            expr->text = "std::make_tuple";
            return expr;
        case TokenKind::LeftParen: {
            // A parenthesised expression starts a fresh context — reset
            // inCallArg_ so that implicit calls work inside the group
            // even when we are nested inside an outer call's arg list.
            advance();
            int savedCallArg = inCallArg_;
            inCallArg_ = 0;
            auto inner = parseExpression();
            inCallArg_ = savedCallArg;
            consume(TokenKind::RightParen, "expected ')' after grouped expression");
            auto group = std::make_unique<Expr>();
            group->kind = Expr::Kind::Grouping;
            group->location = tok.location;
            group->left = std::move(inner);
            return group;
        }
        case TokenKind::LeftBracket: {
            // [expr] grouping — interchangeable with (expr).
            advance();
            int savedCallArg = inCallArg_;
            inCallArg_ = 0;
            auto inner = parseExpression();
            inCallArg_ = savedCallArg;
            consume(TokenKind::RightBracket,
                    "expected ']' after bracketed expression");
            auto group = std::make_unique<Expr>();
            group->kind = Expr::Kind::Grouping;
            group->location = tok.location;
            group->left = std::move(inner);
            return group;
        }
        case TokenKind::LeftBrace: {
            advance();
            return parseArrayLiteralOrType();
        }
        case TokenKind::AtLeftBracket: {
            advance();
            return parseHeapAlloc(tok.location);
        }
        case TokenKind::Dot: {
            // Enum shorthand: .Variant
            advance();
            Token name = consume(TokenKind::Identifier,
                                  "expected identifier after '.'");
            expr->kind = Expr::Kind::EnumShorthand;
            expr->text = name.text;
            expr->location = name.location;
            return expr;
        }
        case TokenKind::Semicolon: {
            // Positional placeholder: ;1, ;2 (used inside operator bodies / catch).
            advance();
            if (check(TokenKind::IntLiteral)) {
                Token n = advance();
                expr->kind = Expr::Kind::PositionalArg;
                expr->positionalIndex = std::atoi(n.text.c_str());
                return expr;
            }
            // Labeled identifier: ;label  → treat as plain identifier reference.
            if (check(TokenKind::Identifier)) {
                Token n = advance();
                expr->kind = Expr::Kind::Identifier;
                expr->text = n.text;
                return expr;
            }
            throw CompileError("expected positional index or label after ';'",
                               tok.location);
        }
        default:
            throw CompileError(
                std::string("unexpected token '") + tok.text + "' (" +
                    tokenKindName(tok.kind) + ") in expression",
                tok.location);
    }
}

// ─────────────────────────────────────────────────────────────────────
//  Helpers
// ─────────────────────────────────────────────────────────────────────

bool Parser::isTypeLikeIdentifier(const std::string &name) {
    if (name.empty()) return false;
    return std::isupper(static_cast<unsigned char>(name[0])) != 0;
}

void Parser::skipNewlines() { while (match(TokenKind::Newline)) {} }

void Parser::consumeStatementEnd() {
    if (match(TokenKind::Newline) || check(TokenKind::Dedent) ||
        check(TokenKind::End)) {
        return;
    }
    throw CompileError("expected end of statement, found '" + current().text + "'",
                       current().location);
}

bool Parser::match(TokenKind kind) {
    if (!check(kind)) return false;
    advance();
    return true;
}

bool Parser::check(TokenKind kind) const { return current().kind == kind; }

Token Parser::advance() {
    if (!check(TokenKind::End)) current_++;
    return previous();
}

Token Parser::consume(TokenKind kind, const std::string &message) {
    if (check(kind)) return advance();
    throw CompileError(message + " (got '" + current().text + "', " +
                           tokenKindName(current().kind) + ")",
                       current().location);
}

Token Parser::consumeIdentLike(const std::string &message) {
    if (check(TokenKind::Identifier) || check(TokenKind::To) ||
        check(TokenKind::Until) || check(TokenKind::Step) ||
        check(TokenKind::In)) {
        return advance();
    }
    throw CompileError(message + " (got '" + current().text + "', " +
                           tokenKindName(current().kind) + ")",
                       current().location);
}

const Token &Parser::current() const { return tokens_[current_]; }
const Token &Parser::previous() const { return tokens_[current_ - 1]; }
const Token &Parser::peek(size_t offset) const {
    size_t i = current_ + offset;
    if (i >= tokens_.size()) return tokens_.back();
    return tokens_[i];
}

} // namespace drast
