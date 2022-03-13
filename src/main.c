//
//  main.c
//  drast
//
//  Created by Ashwin Paudel on 2022-02-05.
//

#include <stdio.h>
#include "compiler/lexer.h"
#include "compiler/ast.h"

// https://stackoverflow.com/a/47195924
char *read_file_contents(char *file_name) {
    FILE *f = fopen(file_name, "rt");
    if (!f) {
        printf("Error Reading File %s\n", file_name);
        exit(-1);
    }
    fseek(f, 0, SEEK_END);
    long length = ftell(f);
    rewind(f);
    char *buffer = (char *) malloc(length + 1);
    buffer[length] = '\0';
    fread(buffer, 1, length, f);
    fclose(f);
    return buffer;
}

int main(__attribute__((unused)) int argc, char **argv) {
    char *file_contents = read_file_contents(argv[1]);

    lexer_init(file_contents);

    for (;;) {
        Token token = lexer_get_token();
        if (token.type == T_EOF)
            break;

        // If it's debug
        printf("%s :: %.*s :: %zu :: %zu\n", token_print(token.type), (int) token.length - 1, token.value, token.line,
               token.column);
    }

    free(file_contents);

    return 0;
}
