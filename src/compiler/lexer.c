//
//  lexer.c
//  drast
//
//  Created by Ashwin Paudel on 2022-02-05.
//

#include "lexer.h"


Lexer *lexer_init(char *source) {
    Lexer *lexer = malloc(sizeof(Lexer));
    lexer->source = source;
    lexer->source_length = strlen(lexer->source);

    lexer->index = 0;
    lexer->current = lexer->source[lexer->index];

    lexer->line = 1;
    lexer->position = 0;

    return lexer;
}

Lexer *lexer_duplicate(Lexer *lexer) {
    Lexer *new_lexer = lexer_init(lexer->source);
    new_lexer->current = lexer->current;
    new_lexer->source = lexer->source;
    new_lexer->index = lexer->index;
    new_lexer->position = lexer->position;
    new_lexer->line = lexer->line;
    new_lexer->source_length = lexer->source_length;

    return new_lexer;
}

Token *lexer_get_next_token_without_advance(Lexer *lexer) {
    Lexer *new_lexer = lexer_duplicate(lexer);
    Token *next_token = lexer_get_next_token(new_lexer);

    free(new_lexer);

    return next_token;
}

Token *lexer_get_next_token_without_advance_offset(Lexer *lexer, uintptr_t offset) {
    Lexer *new_lexer = lexer_duplicate(lexer);

    for (int i = 0; i < offset - 1; ++i) {
        lexer_get_next_token(new_lexer);
    }

    return lexer_get_next_token(new_lexer);
}

Token *lexer_get_next_token(Lexer *lexer) {
    if (lexer_is_eof(lexer)) {
        return lexer_advance_token(T_EOF, (char[2]) {lexer->current, 0}, lexer, true);
    }

    char c = lexer->current;

    switch (c) {
        case ' ':
        case '\r':
        case '\t':
            break;
        case '\n':
            lexer->line += 1;
            lexer->position = 0;
            break;
        case '?':
            return lexer_advance_token(T_QUESTION, "?", lexer, false);
        case '<':
            if (lexer_peek_next(lexer) == '<') {
                lexer_advance(lexer);
                if (lexer_peek_next(lexer) == '=') {
                    lexer_advance(lexer);
                    return lexer_advance_token(T_BITWISE_SHIFT_LEFT_EQUAL, "<<=", lexer, false);
                } else {
                    return lexer_advance_token(T_BITWISE_SHIFT_LEFT, "<<", lexer, false);
                }
            } else if (lexer_peek_next(lexer) == '=') {
                lexer_advance(lexer);
                return lexer_advance_token(T_LESS_THAN_EQUAL, "<=", lexer, false);
            } else {
                return lexer_advance_token(T_LESS_THAN, "<", lexer, false);
            }
        case '>':
            if (lexer_peek_next(lexer) == '>') {
                lexer_advance(lexer);
                if (lexer_peek_next(lexer) == '=') {
                    lexer_advance(lexer);
                    return lexer_advance_token(T_BITWISE_SHIFT_RIGHT_EQUAL, ">>=", lexer, false);
                } else {
                    return lexer_advance_token(T_BITWISE_SHIFT_RIGHT, ">>", lexer, false);
                }
            } else if (lexer_peek_next(lexer) == '=') {
                lexer_advance(lexer);
                return lexer_advance_token(T_GREATER_THAN_EQUAL, ">=", lexer, false);
            } else {
                return lexer_advance_token(T_GREATER_THAN, ">", lexer, false);
            }
        case '=':
            if (lexer_peek_next(lexer) == '=') {
                lexer_advance(lexer);
                return lexer_advance_token(T_EQUAL_EQUAL, "==", lexer, false);
            } else {
                return lexer_advance_token(T_EQUAL, "=", lexer, false);
            }
        case '!':
            if (lexer_peek_next(lexer) == '=') {
                lexer_advance(lexer);
                return lexer_advance_token(T_NOT_EQUAL, "!=", lexer, false);
            } else {
                return lexer_advance_token(T_NOT, "!", lexer, false);
            }
        case '+':
            if (lexer_peek_next(lexer) == '=') {
                lexer_advance(lexer);
                return lexer_advance_token(T_OPERATOR_ADD_EQUAL, "+=", lexer, false);
            } else {
                return lexer_advance_token(T_OPERATOR_ADD, "+", lexer, false);
            }
        case '-':
            if (lexer_peek_next(lexer) == '=') {
                lexer_advance(lexer);
                return lexer_advance_token(T_OPERATOR_SUB_EQUAL, "-=", lexer, false);
            } else if (lexer_peek_next(lexer) == '>') {
                lexer_advance(lexer);
                return lexer_advance_token(T_ARROW, "->", lexer, false);
            } else if (isdigit(lexer_peek_next(lexer))) {
                return lexer_parse_number(lexer);
            } else {
                return lexer_advance_token(T_OPERATOR_SUB, "-", lexer, false);
            }
        case '*':
            if (lexer_peek_next(lexer) == '=') {
                lexer_advance(lexer);
                return lexer_advance_token(T_OPERATOR_MUL_EQUAL, "*=", lexer, false);
            } else {
                return lexer_advance_token(T_OPERATOR_MUL, "*", lexer, false);
            }
        case '/':
            if (lexer_peek_next(lexer) == '=') {
                lexer_advance(lexer);
                return lexer_advance_token(T_OPERATOR_DIV_EQUAL, "/=", lexer, false);
            } else if (lexer_peek_next(lexer) == '/') {
                lexer_skip_line(lexer);
                return lexer_get_next_token(lexer);
            } else if (lexer_peek_next(lexer) == '*') {
                lexer_skip_block_comment(lexer);
                return lexer_get_next_token(lexer);
            } else {
                return lexer_advance_token(T_OPERATOR_DIV, "/", lexer, false);
            }
        case '%':
            if (lexer_peek_next(lexer) == '=') {
                lexer_advance(lexer);
                return lexer_advance_token(T_OPERATOR_MOD_EQUAL, "%=", lexer, false);
            } else {
                return lexer_advance_token(T_OPERATOR_MOD, "%", lexer, false);
            }
        case '&':
            if (lexer_peek_next(lexer) == '&') {
                lexer_advance(lexer);
                if (lexer_peek_next(lexer) == '=') {
                    lexer_advance(lexer);
                    return lexer_advance_token(T_BITWISE_AND_AND_EQUAL, "&&=", lexer, false);
                } else {
                    return lexer_advance_token(T_BITWISE_AND_AND, "&&", lexer, false);
                }
            }
            if (lexer_peek_next(lexer) == '=') {
                lexer_advance(lexer);
                return lexer_advance_token(T_BITWISE_AND_EQUAL, "&=", lexer, false);
            } else {
                return lexer_advance_token(T_BITWISE_AND, "&", lexer, false);
            }
        case '|':
            if (lexer_peek_next(lexer) == '|') {
                lexer_advance(lexer);
                if (lexer_peek_next(lexer) == '=') {
                    lexer_advance(lexer);
                    return lexer_advance_token(T_BITWISE_PIPE_PIPE_EQUAL, "||=", lexer, false);
                } else {
                    return lexer_advance_token(T_BITWISE_PIPE_PIPE, "||", lexer, false);
                }
            }
            if (lexer_peek_next(lexer) == '=') {
                lexer_advance(lexer);
                return lexer_advance_token(T_BITWISE_PIPE_EQUAL, "|=", lexer, false);
            } else {
                return lexer_advance_token(T_BITWISE_PIPE, "|", lexer, false);
            }
        case '^':
            if (lexer_peek_next(lexer) == '=') {
                lexer_advance(lexer);
                return lexer_advance_token(T_BITWISE_POWER_EQUAL, "^=", lexer, false);
            } else {
                return lexer_advance_token(T_BITWISE_POWER, "^", lexer, false);
            }
        case '~':
            return lexer_advance_token(T_BITWISE_NOT, "~", lexer, false);
        case ':':
            if (lexer_peek_next(lexer) == ':') {
                lexer_advance(lexer);
                return lexer_advance_token(T_DOUBLE_COLON, "::", lexer, false);
            } else
                return lexer_advance_token(T_COLON, ":", lexer, false);
        case ';':
            return lexer_advance_token(T_SEMICOLON, ";", lexer, false);
        case '(':
            return lexer_advance_token(T_PARENS_OPEN, "(", lexer, false);
        case ')':
            return lexer_advance_token(T_PARENS_CLOSE, ")", lexer, false);
        case '{':
            return lexer_advance_token(T_BRACE_OPEN, "{", lexer, false);
        case '}':
            return lexer_advance_token(T_BRACE_CLOSE, "}", lexer, false);
        case '[':
            return lexer_advance_token(T_SQUARE_OPEN, "[", lexer, false);
        case ']':
            return lexer_advance_token(T_SQUARE_CLOSE, "]", lexer, false);
        case ',':
            return lexer_advance_token(T_COMMA, ",", lexer, false);
        case '.':
            return lexer_advance_token(T_PERIOD, ".", lexer, false);
        case '$':
            return lexer_advance_token(T_DOLLAR, "$", lexer, false);
        case '#':
            return lexer_advance_token(T_HASHTAG, "#", lexer, false);
        case '@':
            return lexer_advance_token(T_AT, "@", lexer, false);
        case '\\':
            return lexer_advance_token(T_BACKSLASH, "\\", lexer, false);
        case '"':
            return lexer_parse_string(lexer);
        case '\'':
            return lexer_parse_character(lexer);
        case '\0':
            break;
        default:
            if (isalpha(c) || c == '_')
                return lexer_parse_identifier(lexer);
            else if (isdigit(c) || isxdigit(c))
                return lexer_parse_number(lexer);
            fprintf(stderr, "Lexer: Unrecognized Character: `%c`\n", c);
            exit(-1);
    }

    return lexer_advance_token(T_EOF, (char[2]) {lexer->current, 0}, lexer, false);
}

Token *lexer_parse_character(Lexer *lexer) {
    lexer_advance(lexer);
    char *value = calloc(2, sizeof(char));
    value[0] = lexer_advance(lexer);
    value[1] = '\0';
    if (lexer_advance(lexer) != '\'') {
        fprintf(stderr,
                "Lexer: Characters can't have more than 1 character, perhaps you meant to use the `\"` operator?\n");
        exit(-1);
    }

    return lexer_advance_token(T_CHAR, value, lexer, true);
}

Token *lexer_parse_string(Lexer *lexer) {
    uintptr_t line_size = lexer_get_line_count(lexer);

    char *value = calloc(line_size, sizeof(char));
    uintptr_t value_count = 0;
    lexer_advance(lexer);

    while (lexer->current != '"') {
        if (lexer->current == '\\' && (lexer_peek_next(lexer) == '\"')) {
            lexer_advance(lexer);
            lexer_advance(lexer);
            continue;
        } else if (lexer->current == '\n') {
            fprintf(stderr, "Lexer: Can't have multi-line string, use the \\n operator instead\n");
            exit(-1);
        }

        lexer_check_eof(lexer, "Unterminated String Literal\n");

        value_count++;
        strcat(value, (char[2]) {lexer->current, 0});

        lexer_advance(lexer);
    }

    value = realloc(value, (value_count + 2) * sizeof(char));

    lexer_advance(lexer);

    return lexer_advance_token(T_STRING, value, lexer, true);
}

Token *lexer_parse_number(Lexer *lexer) {
    uintptr_t line_size = lexer_get_line_count(lexer);

    char *value = calloc(line_size + 2, sizeof(char));
    uintptr_t value_count = 0;

    while (isdigit(lexer->current) || lexer->current == '.' || lexer->current == '-' || lexer->current == '+' ||
           isxdigit(lexer->current) || lexer->current == 'x' || lexer->current == 'X') {
        value_count++;
        strcat(value, (char[2]) {lexer->current, 0});

        lexer_advance(lexer);
    }

    value = realloc(value, (value_count + 2) * sizeof(char));

    if (string_is_float(value))
        return lexer_advance_token(T_FLOAT, value, lexer, true);
    else if (string_is_hex(value))
        return lexer_advance_token(T_HEX, value, lexer, true);
    else if (string_is_octal(value))
        return lexer_advance_token(T_OCTAL, value, lexer, true);
    else
        return lexer_advance_token(T_INT, value, lexer, true);
}

Token *lexer_parse_identifier(Lexer *lexer) {
    uintptr_t line_size = lexer_get_line_count(lexer);

    char *value = calloc(line_size + 1, sizeof(char));
    uintptr_t value_count = 0;

    while (isalnum(lexer->current) || lexer->current == '_') {
        value_count++;
        strcat(value, (char[2]) {lexer->current, 0});

        lexer_advance(lexer);
    }

    value = realloc(value, (value_count + 1) * sizeof(char));

    return lexer_is_keyword(value, lexer);
}

Token *lexer_is_keyword(char *identifier, Lexer *lexer) {
    // TODO: Faster way to do this?
    if (strcmp(identifier, "struct") == 0)
        return lexer_advance_token(T_K_STRUCT, identifier, lexer, true);
    else if (strcmp(identifier, "self") == 0)
        return lexer_advance_token(T_K_SELF, identifier, lexer, true);
    else if (strcmp(identifier, "enum") == 0)
        return lexer_advance_token(T_K_ENUM, identifier, lexer, true);
    else if (strcmp(identifier, "alias") == 0)
        return lexer_advance_token(T_K_ALIAS, identifier, lexer, true);
    else if (strcmp(identifier, "return") == 0)
        return lexer_advance_token(T_K_RETURN, identifier, lexer, true);
    else if (strcmp(identifier, "if") == 0)
        return lexer_advance_token(T_K_IF, identifier, lexer, true);
    else if (strcmp(identifier, "else") == 0)
        return lexer_advance_token(T_K_ELSE, identifier, lexer, true);
    else if (strcmp(identifier, "import") == 0)
        return lexer_advance_token(T_K_IMPORT, identifier, lexer, true);
    else if (strcmp(identifier, "asm") == 0)
        return lexer_advance_token(T_K_ASM, identifier, lexer, true);
    else if (strcmp(identifier, "volatile") == 0)
        return lexer_advance_token(T_K_VOLATILE, identifier, lexer, true);
    else if (strcmp(identifier, "cast") == 0)
        return lexer_advance_token(T_K_CAST, identifier, lexer, true);
    else if (strcmp(identifier, "switch") == 0)
        return lexer_advance_token(T_K_SWITCH, identifier, lexer, true);
    else if (strcmp(identifier, "matches") == 0)
        return lexer_advance_token(T_K_MATCHES, identifier, lexer, true);
    else if (strcmp(identifier, "case") == 0)
        return lexer_advance_token(T_K_CASE, identifier, lexer, true);
    else if (strcmp(identifier, "break") == 0)
        return lexer_advance_token(T_K_BREAK, identifier, lexer, true);
    else if (strcmp(identifier, "default") == 0)
        return lexer_advance_token(T_K_DEFAULT, identifier, lexer, true);
    else if (strcmp(identifier, "while") == 0)
        return lexer_advance_token(T_K_WHILE, identifier, lexer, true);
    else if (strcmp(identifier, "for") == 0)
        return lexer_advance_token(T_K_FOR, identifier, lexer, true);
    else if (strcmp(identifier, "continue") == 0)
        return lexer_advance_token(T_K_CONTINUE, identifier, lexer, true);
    else if (strcmp(identifier, "union") == 0)
        return lexer_advance_token(T_K_UNION, identifier, lexer, true);
    else if (strcmp(identifier, "false") == 0)
        return lexer_advance_token(T_K_FALSE, identifier, lexer, true);
    else if (strcmp(identifier, "true") == 0)
        return lexer_advance_token(T_K_TRUE, identifier, lexer, true);
    else if (strcmp(identifier, "bool") == 0)
        return lexer_advance_token(T_K_BOOL, identifier, lexer, true);
    else if (strcmp(identifier, "int") == 0)
        return lexer_advance_token(T_K_INT, identifier, lexer, true);
    else if (strcmp(identifier, "float") == 0)
        return lexer_advance_token(T_K_FLOAT, identifier, lexer, true);
    else if (strcmp(identifier, "void") == 0)
        return lexer_advance_token(T_K_VOID, identifier, lexer, true);
    else if (strcmp(identifier, "string") == 0)
        return lexer_advance_token(T_K_STRING, identifier, lexer, true);
    else if (strcmp(identifier, "char") == 0)
        return lexer_advance_token(T_K_CHAR, identifier, lexer, true);
    else if (strcmp(identifier, "goto") == 0)
        return lexer_advance_token(T_K_GOTO, identifier, lexer, true);
    else if (strcmp(identifier, "private") == 0)
        return lexer_advance_token(T_K_PRIVATE, identifier, lexer, true);
    else if (strcmp(identifier, "do") == 0)
        return lexer_advance_token(T_K_DO, identifier, lexer, true);
    else if (strcmp(identifier, "try") == 0)
        return lexer_advance_token(T_K_TRY, identifier, lexer, true);
    else if (strcmp(identifier, "catch") == 0)
        return lexer_advance_token(T_K_CATCH, identifier, lexer, true);
    else
        return lexer_advance_token(T_IDENTIFIER, identifier, lexer, true);
}

uintptr_t lexer_get_line_count(Lexer *lexer) {
    Lexer *fake_lexer = lexer_duplicate(lexer);

    uintptr_t line_size = 0;
    while (fake_lexer->current != '\n' && !lexer_is_eof(fake_lexer)) {
        line_size++;
        lexer_advance(fake_lexer);
    }

    free(fake_lexer);

    return line_size;
}

void lexer_skip_block_comment(Lexer *lexer) {
    lexer_advance(lexer);
    lexer_advance(lexer);
    while (!lexer_is_eof(lexer)) {
        if (lexer->current == '*' && lexer_peek_next(lexer) == '/') {
            break;
        }
        if (lexer->current == '\n') {
            lexer->line++;
        }
        lexer_advance(lexer);
    }

    if (!lexer_is_eof(lexer)) {
        lexer_advance(lexer);
        lexer_advance(lexer);
    } else {
        fprintf(stderr, "Lexer: Unterminated Block Comment\n");
        exit(-1);
    }

    lexer_skip_whitespace(lexer);
}

bool lexer_is_whitespace(Lexer *lexer) {
    return lexer->current == '\t' || lexer->current == ' ' || lexer->current == '\n' || lexer->current == '\r';
}

void lexer_skip_whitespace(Lexer *lexer) {
    do {
        if (lexer->current == '\n') {
            lexer->position = 0;
            lexer->line++;
        }
        lexer_advance(lexer);
    } while (!lexer_is_eof(lexer) &&
             (lexer->current == '\t' || lexer->current == ' ' || lexer->current == '\n' || lexer->current == '\r'));
}

void lexer_skip_line(Lexer *lexer) {
    while (!lexer_is_eof(lexer) && lexer->current != '\n') {
        lexer_advance(lexer);
    }

    if (lexer_is_whitespace(lexer))
        lexer_skip_whitespace(lexer);
    lexer->position = 0;
}

char lexer_peek_next(Lexer *lexer) {
    return lexer->source[lexer->index + 1];
}

char lexer_advance(Lexer *lexer) {
    if (lexer_is_eof(lexer)) {
        return lexer->current;
    } else {
        lexer->position += 1;
        char previous = lexer->current;
        lexer->index += 1;
        lexer->current = lexer->source[lexer->index];

        return previous;
    }
}

Token *lexer_advance_token(int token_type, char *value, Lexer *lexer, bool does_not_advance) {
    Token *new_token = calloc(1, sizeof(Token));
    new_token->value = value;
    new_token->type = token_type;

    if (!does_not_advance)
        lexer_advance(lexer);
    if (lexer_is_whitespace(lexer))
        lexer_skip_whitespace(lexer);

    return new_token;
}

bool lexer_is_eof(Lexer *lexer) {
    return (lexer->index >= lexer->source_length);
}

void lexer_check_eof(Lexer *lexer, char *error_message) {
    if (lexer_is_eof(lexer)) {
        fprintf(stderr, "Lexer: %s\n", error_message);
        exit(-1);
    }
}