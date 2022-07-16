#include <iostream>
#include <fstream>
#include "ArgsParser/ArgsParser.h"
#include "Config/Config.h"
#include "Lexer/Lexer.h"
#include "Parser/Parser.h"
#include "TypeCheck/TypeChecker.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cout << "Invalid argument count" << std::endl;
        exit(EXIT_FAILURE);
    }

    ArgsParser argsParser(argc, argv);
    argsParser.parseArguments();

    const auto source = ([&]() -> std::string {
        try {
            std::ifstream in(Config::shared()->filename, std::ios::in);
            in.exceptions(std::ifstream::failbit | std::ifstream::badbit);
            return std::string{std::istreambuf_iterator<char>{in},
                               std::istreambuf_iterator<char>{}};
        } catch (std::exception &e) {
            throw std::runtime_error("Couldn't open input source file.");
        }
    })();

    Lexer lexer(source);
    Parser parser(lexer);
    parser.parse();
    if (Config::shared()->showsLexerOutput) {
        for (const auto &token: parser.tokens) {
            std::cout << Config::shared()->filename << ":" << token.location.line << ":"
                      << token.location.column << ": " << token.toString() << std::endl;
        }
    }

    TypeChecker typeChecker(parser.root);

    typeChecker.check();

    std::cout << parser.root->generate();
    std::cout << "\n-----------------------------------------------------\n\n\n";
    std::cout << parser.root->toString();

    return 0;
}

