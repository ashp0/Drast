//
//  main.c
//  drast
//
//  Created by Ashwin Paudel on 2022-02-05.
//

#include <stdio.h>
#include "argument_parser.h"

const char *long_name_list[] = {"version", "help"};

static void parse_short_argument(const char option, const char *value);
static void parse_long_argument(const char *option, const char *value);

int main(int argc, const char *argv[])
{
    ArgumentParser *argument_parser = argument_parser_init(argc, argv, long_name_list);

    argument_parser_parse_arguments(argument_parser, &parse_short_argument, &parse_long_argument);

    return 0;
}

static void parse_short_argument(const char option, const char *value)
{
    switch (option)
    {
    case 'v':
        printf("VERSION v1.0\n");
        break;

    case 'h':
        printf("The Drast Compiler\n");
        printf("Options:\n");
        printf("\tv - Version\n");
        printf("\th - Help\n");
        break;

    default:
        break;
    }
}

static void parse_long_argument(const char *option, const char *value)
{
    printf("LONG ARGUMENT: %s :: %s\n", option, value);
}