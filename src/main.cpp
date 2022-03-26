#include "compiler/Lexer.h"
#include "compiler/Parser.h"
#include <fstream>
#include <iostream>
#include <sstream>

std::string read_file(const char *file_name) {
    FILE *f = fopen(file_name, "rt");
    if (!f) {
        printf("Error: File `%s` cannot be found\n", file_name);
        exit(-1);
    }
    fseek(f, 0, SEEK_END);
    long length = ftell(f);
    rewind(f);
    std::string buffer(length + 1, '\0');
    buffer[length + 1] = '\0'; // Must add that, otherwise lexer will crash
    fread(buffer.data(), 1, length, f);
    fclose(f);
    return buffer;
}

int main(int argc, char *argv[]) {
    std::string source = read_file(argv[1]);

    std::string file_name = argv[1];

    Lexer lexer(source, file_name);
    lexer.lex();

    Parser parser(file_name, lexer.tokens, source);
    parser.parse();

    return 0;
}
