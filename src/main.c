//
//  main.c
//  drast
//
//  Created by Ashwin Paudel on 2022-02-05.
//

#include <stdio.h>
#include "compiler/lexer.h"
#include "compiler/ast.h"
#include "compiler/parser.h"
#include <time.h>
#include "utils/mxHashmap.h"

#define measure_time(x) \
    clock_t t; \
    t = clock(); \
    x; \
    t = clock() - t; \
    double time_taken = ((double)t)/CLOCKS_PER_SEC; \
    printf("%s took %f seconds to execute \n", #x, time_taken);

long file_size = 0;

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
    file_size = length;
    char *buffer = (char *) malloc(length + 1);
    buffer[length] = '\0';
    fread(buffer, 1, length, f);
    fclose(f);
    return buffer;
}

int main(__attribute__((unused)) int argc, char **argv) {
    printf("Drast Compiler\n");

    char *file_contents = read_file_contents(argv[1]);

    lexer_init(file_contents, file_size);
    parser_init(lexer_get());


//    for (;;) {
//        Token token = lexer_get_token();
//        if (token.type == T_EOF) {
//            break;
//        }
////        printf("%s :: %.*s :: %zu :: %zu\n",
////               token_print(token.type),
////               (int) token.length, token.value,
////               token.line,
////               token.column);
//
//    }

    measure_time(parser_parse());

    free(file_contents);

//    char *key = "hello world";
//    mxHashmap *map = mxHashmap_create();
//    mxHashmap_set(map, key, 11, T_IDENTIFIER);
//
//    uintptr_t out = 0;
//    if (mxHashmap_get(map, "hello world", 11, &out)) {
//        printf("%s\n", token_print((int)out));
//    }


    return 0;
}