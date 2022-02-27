//
//  main.c
//  drast
//
//  Created by Ashwin Paudel on 2022-02-05.
//

#include <stdio.h>
#include <time.h>
#include "compiler/lexer.h"
#include "compiler/parser.h"
#include "compiler/ast.h"
#include "compiler/ast_print.h"
#include "compiler/semantic_analyzer.h"

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
    Parser *parser = parser_init(lexer);

    double time = 0;

    clock_t t;
    t = clock();
    SemanticAnalyzerASTItems *ast_items = calloc(1, sizeof(SemanticAnalyzerASTItems));
    while (parser->lexer->index < parser->lexer->source_length) {
        Token *next_token = lexer_get_next_token_without_advance(parser->lexer);
//        if (next_token->type == T_EOF)
//            break;
//        printf("%s(`%s`)\n", token_print(next_token->type), next_token->value);
        if (next_token->type == T_EOF)
            break;
        AST *ast = parser_parse(parser);
//         ast_print(ast);

        // Add the item into the array
        ast_items->item_size += 1;
        ast_items->items = realloc(ast_items->items, ast_items->item_size * sizeof(AST *));

        ast_items->items[ast_items->item_size - 1] = ast;
    }
    /*
     t = clock() - t;
    double time_taken = ((double) t) / CLOCKS_PER_SEC;

    printf("\n%f seconds to lex and parse the file \n", time_taken);
     */

    UNMap *map = semantic_analyzer_create_declaration_table(ast_items);
    semantic_analyzer_run_analysis(map);


    return 0;
}