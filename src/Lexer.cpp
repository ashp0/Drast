#include "Lexer.h"

#include <cctype>
#include <map>
#include <utility>

namespace drast {

Lexer::Lexer(std::string source, std::string fileName) : source_(std::move(source)) {
    location_.file = std::move(fileName);
    indentStack_.push_back(0);
}

std::vector<Token> Lexer::lex() {
    std::vector<Token> tokens;
    for (;;) {
        Token token = next();
        tokens.push_back(token);
        if (token.kind == TokenKind::End) break;
    }
    return tokens;
}

Token Lexer::next() {
    if (!pendingTokens_.empty()) {
        Token t = std::move(pendingTokens_.front());
        pendingTokens_.pop_front();
        return t;
    }

    if (atLineStart_) {
        processIndentation();
        if (!pendingTokens_.empty()) {
            Token t = std::move(pendingTokens_.front());
            pendingTokens_.pop_front();
            return t;
        }
    }

    skipWhitespaceInline();

    const SourceLocation start = location_;
    const size_t startOffset = current_;
    const char c = peek();

    if (c == '\0') {
        while (indentStack_.size() > 1) {
            indentStack_.pop_back();
            pendingTokens_.push_back(make(TokenKind::Dedent, "", location_));
        }
        if (!pendingTokens_.empty()) {
            Token t = std::move(pendingTokens_.front());
            pendingTokens_.pop_front();
            pendingTokens_.push_back(make(TokenKind::End, "", start));
            return t;
        }
        return make(TokenKind::End, "", start);
    }

    if (c == '\n') {
        advance();
        atLineStart_ = true;
        inUseLine_ = false;
        return make(TokenKind::Newline, "\n", start);
    }

    if (c == '/' && peek(1) == '/') {
        skipLineComment();
        return next();
    }
    if (c == '/' && peek(1) == '*') {
        skipBlockComment();
        return next();
    }

    if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
        return identifier(start, startOffset);
    }
    if (std::isdigit(static_cast<unsigned char>(c))) {
        return number(start, startOffset);
    }
    if (c == '\'') return quoted(start);
    if (c == '@' && peek(1) == '[') {
        advance();
        advance();
        return make(TokenKind::AtLeftBracket, "@[", start);
    }

    advance();
    switch (c) {
        case '{': return make(TokenKind::LeftBrace, "{", start);
        case '}': return make(TokenKind::RightBrace, "}", start);
        case '[': return make(TokenKind::LeftBracket, "[", start);
        case ']': return make(TokenKind::RightBracket, "]", start);
        case '(': return make(TokenKind::LeftParen, "(", start);
        case ')': return make(TokenKind::RightParen, ")", start);
        case ',': return make(TokenKind::Comma, ",", start);
        case ';': return make(TokenKind::Semicolon, ";", start);
        case '.': return make(TokenKind::Dot, ".", start);
        case ':':
            if (match(':')) return make(TokenKind::DoubleColon, "::", start);
            return make(TokenKind::Colon, ":", start);
        case '`': return make(TokenKind::Backtick, "`", start);
        case '~': return make(TokenKind::Tilde, "~", start);
        case '+':
            if (match('=')) return make(TokenKind::PlusEqual, "+=", start);
            return make(TokenKind::Plus, "+", start);
        case '-':
            if (match('=')) return make(TokenKind::MinusEqual, "-=", start);
            return make(TokenKind::Minus, "-", start);
        case '*':
            if (match('=')) return make(TokenKind::StarEqual, "*=", start);
            return make(TokenKind::Star, "*", start);
        case '/':
            if (match('=')) return make(TokenKind::SlashEqual, "/=", start);
            return make(TokenKind::Slash, "/", start);
        case '%': return make(TokenKind::Percent, "%", start);
        case '=':
            if (match('=')) return make(TokenKind::EqualEqual, "==", start);
            return make(TokenKind::Equal, "=", start);
        default:
            break;
    }
    throw CompileError("unexpected character '" + std::string(1, c) + "'", start);
}

int Lexer::measureIndent() {
    int indent = 0;
    while (peek() == ' ' || peek() == '\t') {
        indent += (peek() == '\t') ? 4 : 1;
        advance();
    }
    return indent;
}

void Lexer::processIndentation() {
    atLineStart_ = false;

    while (true) {
        int indent = measureIndent();

        if (peek() == '\n') {
            advance();
            continue;
        }
        if (peek() == '\0') {
            return;
        }
        if (peek() == '/' && peek(1) == '/') {
            skipLineComment();
            if (peek() == '\n') advance();
            continue;
        }
        if (peek() == '/' && peek(1) == '*') {
            skipBlockComment();
            continue;
        }

        int currentLevel = indentStack_.back();
        if (indent > currentLevel) {
            indentStack_.push_back(indent);
            pendingTokens_.push_back(make(TokenKind::Indent, "", location_));
        } else if (indent < currentLevel) {
            while (indentStack_.size() > 1 && indentStack_.back() > indent) {
                indentStack_.pop_back();
                pendingTokens_.push_back(make(TokenKind::Dedent, "", location_));
            }
            if (indentStack_.back() != indent) {
                throw CompileError("inconsistent indentation", location_);
            }
        }
        return;
    }
}

void Lexer::skipWhitespaceInline() {
    while (peek() == ' ' || peek() == '\t' || peek() == '\r') {
        advance();
    }
}

void Lexer::skipLineComment() {
    while (peek() != '\0' && peek() != '\n') {
        advance();
    }
}

void Lexer::skipBlockComment() {
    advance(); // /
    advance(); // *
    int nesting = 1;
    while (nesting > 0) {
        if (peek() == '\0') {
            throw CompileError("unterminated block comment", location_);
        }
        if (peek() == '/' && peek(1) == '*') {
            advance(); advance();
            nesting++;
        } else if (peek() == '*' && peek(1) == '/') {
            advance(); advance();
            nesting--;
        } else {
            advance();
        }
    }
}

Token Lexer::identifier(SourceLocation start, size_t startOffset) {
    while (std::isalnum(static_cast<unsigned char>(peek())) || peek() == '_') {
        advance();
    }
    std::string text = source_.substr(startOffset, current_ - startOffset);

    // s'...' — force a string literal even for single characters
    if (text == "s" && peek() == '\'') {
        Token tok = quoted(start);
        tok.kind = TokenKind::StringLiteral;
        return tok;
    }

    static const std::map<std::string, TokenKind> keywords = {
        {"use", TokenKind::Use},
        {"const", TokenKind::Const},
        {"return", TokenKind::Return},
        {"if", TokenKind::If},
        {"elif", TokenKind::Elif},
        {"else", TokenKind::Else},
        {"while", TokenKind::While},
        {"for", TokenKind::For},
        {"break", TokenKind::Break},
        {"continue", TokenKind::Continue},
        {"in", TokenKind::In},
        {"to", TokenKind::To},
        {"until", TokenKind::Until},
        {"step", TokenKind::Step},
        {"struct", TokenKind::Struct},
        {"enum", TokenKind::Enum},
        {"impl", TokenKind::Impl},
        {"protocol", TokenKind::Protocol},
        {"match", TokenKind::Match},
        {"with", TokenKind::With},
        {"as", TokenKind::As},
        {"try", TokenKind::Try},
        {"catch", TokenKind::Catch},
        {"default", TokenKind::Default},
        {"private", TokenKind::Private},
        {"preview", TokenKind::Preview},
        {"fileprivate", TokenKind::Fileprivate},
        {"discard", TokenKind::Discard},
        {"operator", TokenKind::Operator},
        {"maybe", TokenKind::Maybe},
        {"true", TokenKind::True},
        {"false", TokenKind::False},
        {"nil", TokenKind::Nil},
        {"self", TokenKind::Self_},
        {"variadic", TokenKind::Variadic},
        {"tuple", TokenKind::Tuple},
        {"iseq", TokenKind::Iseq},
        {"isne", TokenKind::Isne},
        {"islt", TokenKind::Islt},
        {"isgt", TokenKind::Isgt},
        {"islteq", TokenKind::Islteq},
        {"isgteq", TokenKind::Isgteq},
        {"and", TokenKind::And},
        {"or", TokenKind::Or},
        {"not", TokenKind::Not},
        {"shl", TokenKind::Shl},
        {"shr", TokenKind::Shr},
        {"bor", TokenKind::Bor},
        {"band", TokenKind::Band},
        {"bxor", TokenKind::Bxor},
    };

    auto found = keywords.find(text);
    TokenKind kind = (found == keywords.end()) ? TokenKind::Identifier : found->second;

    if (kind == TokenKind::Use) {
        inUseLine_ = true;
    }
    return make(kind, text, start);
}

Token Lexer::number(SourceLocation start, size_t startOffset) {
    if (peek() == '0') {
        char next = peek(1);
        if (next == 'x' || next == 'X') {
            advance(); advance();
            while (std::isxdigit(static_cast<unsigned char>(peek()))) advance();
            return make(TokenKind::IntLiteral, source_.substr(startOffset, current_ - startOffset), start);
        }
        if (next == 'b' || next == 'B') {
            advance(); advance();
            while (peek() == '0' || peek() == '1') advance();
            return make(TokenKind::IntLiteral, source_.substr(startOffset, current_ - startOffset), start);
        }
        if (next == 'o' || next == 'O') {
            advance(); advance();
            while (peek() >= '0' && peek() <= '7') advance();
            return make(TokenKind::IntLiteral, source_.substr(startOffset, current_ - startOffset), start);
        }
    }

    bool isFloat = false;
    while (std::isdigit(static_cast<unsigned char>(peek()))) advance();
    if (peek() == '.' && std::isdigit(static_cast<unsigned char>(peek(1)))) {
        isFloat = true;
        advance();
        while (std::isdigit(static_cast<unsigned char>(peek()))) advance();
    }
    return make(isFloat ? TokenKind::FloatLiteral : TokenKind::IntLiteral,
                source_.substr(startOffset, current_ - startOffset), start);
}

Token Lexer::quoted(SourceLocation start) {
    advance(); // opening '
    std::string text;
    for (;;) {
        char c = peek();
        if (c == '\0' || c == '\n') {
            throw CompileError("unterminated string literal", start);
        }
        if (c == '\\') {
            advance();
            switch (peek()) {
                case 'n': text.push_back('\n'); break;
                case 't': text.push_back('\t'); break;
                case '\\': text.push_back('\\'); break;
                case '\'': text.push_back('\''); break;
                case '"': text.push_back('"'); break;
                case '0': text.push_back('\0'); break;
                case 'r': text.push_back('\r'); break;
                default: text.push_back(peek()); break;
            }
            advance();
            continue;
        }
        if (c == '\'') {
            // Apostrophe heuristic — a `'` is treated as a literal apostrophe
            // (rather than a closing quote) when it is immediately followed by
            // another `'` or by an alphanumeric character. A genuine closing
            // quote is followed by whitespace, punctuation, or end-of-line.
            char after = peek(1);
            if (after == '\'' ||
                std::isalnum(static_cast<unsigned char>(after))) {
                text.push_back('\'');
                advance();
                continue;
            }
            break;
        }
        text.push_back(c);
        advance();
    }
    advance(); // closing '

    // Single character → char literal; otherwise string literal
    TokenKind kind = (text.size() == 1) ? TokenKind::CharLiteral : TokenKind::StringLiteral;
    return make(kind, text, start);
}

Token Lexer::make(TokenKind kind, std::string text, SourceLocation location) const {
    return Token{kind, std::move(text), location};
}

char Lexer::peek(size_t offset) const {
    if (current_ + offset >= source_.size()) return '\0';
    return source_[current_ + offset];
}

bool Lexer::match(char expected) {
    if (peek() != expected) return false;
    advance();
    return true;
}

void Lexer::advance() {
    if (peek() == '\n') {
        location_.line++;
        location_.column = 1;
    } else {
        location_.column++;
    }
    current_++;
}

} // namespace drast
