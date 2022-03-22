#include "compiler/Lexer.h"
#include "compiler/Parser.h"
#include <iostream>

std::string read_file(const char *file_name) {
    FILE *f = fopen(file_name, "rt");
    if (!f) {
        printf("Error Reading File %s\n", file_name);
        exit(-1);
    }
    fseek(f, 0, SEEK_END);
    long length = ftell(f);
    rewind(f);
    char *buffer = (char *)malloc(length + 1);
    buffer[length] = '\0';
    fread(buffer, 1, length, f);
    fclose(f);
    return buffer;
}

int main(int argc, char *argv[]) {
    std::string source = read_file(argv[1]);

    Lexer lexer(source);
    lexer.lex();

    std::string file_name = argv[1];

    Parser parser(file_name, lexer.tokens);
    parser.parse();

    return 0;
}
