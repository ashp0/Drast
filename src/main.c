//
//  main.c
//  drast
//
//  Created by Ashwin Paudel on 2022-02-05.
//

#include <stdio.h>
#include "argument_parser.h"
#include "compiler/lexer.h"
#include "compiler/token.h"

// https://stackoverflow.com/a/47195924
char *read_file_contents(char *file_name) {
    FILE *f = fopen(file_name, "rt");
    if (!f) {
        printf("Error Reading File %s\n", file_name);
        exit(-1);
    }
    fseek(f, 0, SEEK_END);
    long length = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buffer = (char *) malloc(length + 1);
    buffer[length] = '\0';
    fread(buffer, 1, length, f);
    fclose(f);
    return buffer;
}

int main(int argc, char **argv) {
    char *file_contents = read_file_contents(argv[1]);

    Lexer *lexer = lexer_init(file_contents);

    do {
        Token *next_token = lexer_get_next_token(lexer);
        printf("%s(`%s`)\n", token_print(next_token->type), next_token->value);
    } while (lexer->index < lexer->source_length);
}