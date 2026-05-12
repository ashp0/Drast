#include "Token.h"

namespace drast {

const char *tokenKindName(TokenKind kind) {
    switch (kind) {
        case TokenKind::End: return "End";
        case TokenKind::Newline: return "Newline";
        case TokenKind::Indent: return "Indent";
        case TokenKind::Dedent: return "Dedent";
        case TokenKind::Identifier: return "Identifier";
        case TokenKind::IntLiteral: return "IntLiteral";
        case TokenKind::FloatLiteral: return "FloatLiteral";
        case TokenKind::StringLiteral: return "StringLiteral";
        case TokenKind::CharLiteral: return "CharLiteral";
        case TokenKind::AngleInclude: return "AngleInclude";
        case TokenKind::QuotedInclude: return "QuotedInclude";
        case TokenKind::LeftBrace: return "LeftBrace";
        case TokenKind::RightBrace: return "RightBrace";
        case TokenKind::LeftBracket: return "LeftBracket";
        case TokenKind::RightBracket: return "RightBracket";
        case TokenKind::AtLeftBracket: return "AtLeftBracket";
        case TokenKind::LeftParen: return "LeftParen";
        case TokenKind::RightParen: return "RightParen";
        case TokenKind::Comma: return "Comma";
        case TokenKind::Semicolon: return "Semicolon";
        case TokenKind::Dot: return "Dot";
        case TokenKind::Colon: return "Colon";
        case TokenKind::DoubleColon: return "DoubleColon";
        case TokenKind::Backtick: return "Backtick";
        case TokenKind::Tilde: return "Tilde";
        case TokenKind::Equal: return "Equal";
        case TokenKind::EqualEqual: return "EqualEqual";
        case TokenKind::Plus: return "Plus";
        case TokenKind::Minus: return "Minus";
        case TokenKind::Star: return "Star";
        case TokenKind::Slash: return "Slash";
        case TokenKind::Percent: return "Percent";
        case TokenKind::PlusEqual: return "PlusEqual";
        case TokenKind::MinusEqual: return "MinusEqual";
        case TokenKind::StarEqual: return "StarEqual";
        case TokenKind::SlashEqual: return "SlashEqual";
        case TokenKind::Iseq: return "Iseq";
        case TokenKind::Isne: return "Isne";
        case TokenKind::Islt: return "Islt";
        case TokenKind::Isgt: return "Isgt";
        case TokenKind::Islteq: return "Islteq";
        case TokenKind::Isgteq: return "Isgteq";
        case TokenKind::And: return "And";
        case TokenKind::Or: return "Or";
        case TokenKind::Not: return "Not";
        case TokenKind::Shl: return "Shl";
        case TokenKind::Shr: return "Shr";
        case TokenKind::Bor: return "Bor";
        case TokenKind::Band: return "Band";
        case TokenKind::Bxor: return "Bxor";
        case TokenKind::Use: return "Use";
        case TokenKind::Const: return "Const";
        case TokenKind::Return: return "Return";
        case TokenKind::If: return "If";
        case TokenKind::Elif: return "Elif";
        case TokenKind::Else: return "Else";
        case TokenKind::While: return "While";
        case TokenKind::For: return "For";
        case TokenKind::Break: return "Break";
        case TokenKind::Continue: return "Continue";
        case TokenKind::In: return "In";
        case TokenKind::To: return "To";
        case TokenKind::Until: return "Until";
        case TokenKind::Step: return "Step";
        case TokenKind::Struct: return "Struct";
        case TokenKind::Enum: return "Enum";
        case TokenKind::Impl: return "Impl";
        case TokenKind::Protocol: return "Protocol";
        case TokenKind::Match: return "Match";
        case TokenKind::With: return "With";
        case TokenKind::As: return "As";
        case TokenKind::Try: return "Try";
        case TokenKind::Catch: return "Catch";
        case TokenKind::Default: return "Default";
        case TokenKind::Private: return "Private";
        case TokenKind::Preview: return "Preview";
        case TokenKind::Fileprivate: return "Fileprivate";
        case TokenKind::Discard: return "Discard";
        case TokenKind::Operator: return "Operator";
        case TokenKind::Maybe: return "Maybe";
        case TokenKind::True: return "True";
        case TokenKind::False: return "False";
        case TokenKind::Nil: return "Nil";
        case TokenKind::Self_: return "Self";
        case TokenKind::Variadic: return "Variadic";
        case TokenKind::Tuple: return "Tuple";
    }
    return "?";
}

} // namespace drast
