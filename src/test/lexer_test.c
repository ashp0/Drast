//
//  lexer_test.h
//  drast
//
//  Created by Ashwin Paudel on 2022-02-05.
//

#include "../compiler/lexer.h"

int main() {
    char *file_contents = "import io;\n"
                          "\n"
                          "func main() -> Int {\n"
                          "    print(\"Hello World\")\n"
                          "}\n";

    Lexer *lexer = lexer_init(file_contents);
    do {
        Token *next_token = lexer_get_next_token(lexer);
        printf("%s(`%s`)\n", token_print(next_token->type), next_token->value);
    } while (lexer->index < lexer->source_length);
}