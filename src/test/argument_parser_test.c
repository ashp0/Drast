//
//  argument_parser_test.h
//  drast
//
//  Created by Ashwin Paudel on 2022-02-05.
//

#include "../utils/argument_parser.h"

const char *long_name_list[] = {"version", "help"};

static void parse_short_argument(const char option, const char *value);

static void parse_long_argument(const char *option, const char *value);

int main(int argc, const char *argv[]) {
    ArgumentParser *argument_parser = argument_parser_init(argc, argv, long_name_list);

    argument_parser_parse_arguments(argument_parser, &parse_short_argument, &parse_long_argument);

    return 0;
}

static void parse_short_argument(const char option, const char *value) {
    switch (option) {
        case 'v':
            parse_long_argument("version", value);
            break;

        case 'h':
            parse_long_argument("help", value);
            break;

        default:
            break;
    }
}

static void parse_long_argument(const char *option, const char *value) {
    if (strcmp(option, "version") == 0) {
        printf("VERSION v1.0\n");
    } else if (strcmp(option, "help") == 0) {
        printf("The Drast Compiler\n");
        printf("Options:\n");
        printf("\tv - Version\n");
        printf("\th - Help\n");
    } else {
        printf("Unsupported Argument");
    }
}