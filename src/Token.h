#pragma once

#include <ostream>
#include <string>
#include <stdexcept>
#include <utility>
#include <vector>

namespace drast {

struct SourceLocation {
    std::string file;
    int line = 1;
    int column = 1;
};

struct Diagnostic {
    std::string message;
    SourceLocation location;
};

class DiagnosticBag {
public:
    void add(std::string message, SourceLocation location) {
        diagnostics_.push_back(Diagnostic{std::move(message), std::move(location)});
    }
    bool empty() const { return diagnostics_.empty(); }
    size_t size() const { return diagnostics_.size(); }
    const std::vector<Diagnostic> &all() const { return diagnostics_; }
    void emitTo(std::ostream &out) const {
        for (const Diagnostic &diagnostic : diagnostics_) {
            out << format(diagnostic.message, diagnostic.location) << '\n';
        }
    }
    static std::string format(const std::string &message, SourceLocation location) {
        std::string prefix;
        if (!location.file.empty()) prefix += location.file + ":";
        prefix += std::to_string(location.line) + ":" +
                  std::to_string(location.column);
        return "[" + prefix + "] " + message;
    }
private:
    std::vector<Diagnostic> diagnostics_;
};

class CompileError : public std::runtime_error {
public:
    CompileError(const std::string &message, SourceLocation location)
        : std::runtime_error(format(message, location)), location_(location) {}
    SourceLocation location() const { return location_; }
private:
    static std::string format(const std::string &message, SourceLocation location) {
        return DiagnosticBag::format(message, std::move(location));
    }
    SourceLocation location_;
};

enum class TokenKind {
    End,
    Newline,
    Indent,
    Dedent,

    Identifier,
    IntLiteral,
    FloatLiteral,
    StringLiteral,
    CharLiteral,
    AngleInclude,    // <Arduino.h> bare module identifier — handled by parser
    QuotedInclude,   // "myHeader.h" path — stored without quotes

    // Punctuation
    LeftBrace,       // { array literal / array type / block braces (unused)
    RightBrace,
    LeftBracket,     // [ constructor / function-call brackets
    RightBracket,
    AtLeftBracket,   // @[ shared heap allocation / type
    LeftParen,       // ( grouping (interchangeable with brackets in expressions)
    RightParen,
    Comma,           // ,
    Semicolon,       // ; labeled-parameter prefix, unset-var terminator
    Dot,             // .
    Colon,           // :  used in C++ interop names like std::list
    DoubleColon,     // ::
    Backtick,        // `  generic marker / deref / pointer type suffix
    Tilde,           // ~  reference / address-of

    // Symbolic operators
    Equal,           // =
    EqualEqual,      // ==
    Plus, Minus, Star, Slash, Percent,
    PlusEqual, MinusEqual, StarEqual, SlashEqual,

    // Textual comparisons
    Iseq, Isne, Islt, Isgt, Islteq, Isgteq,

    // Textual logical
    And, Or, Not,

    // Textual bitwise
    Shl, Shr, Bor, Band, Bxor,

    // Keywords
    Use, Const, Return,
    If, Elif, Else, While, For, Break, Continue,
    In, To, Until, Step,
    Struct, Enum, Impl, Protocol, Match, With, As,
    Try, Catch, Default,
    Private, Preview, Fileprivate, Discard, Operator, Maybe,
    True, False, Nil, Self_,
    Variadic, Tuple,
};

struct Token {
    TokenKind kind = TokenKind::End;
    std::string text;
    SourceLocation location;
};

const char *tokenKindName(TokenKind kind);

} // namespace drast
