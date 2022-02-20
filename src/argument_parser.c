//
//  argument_parser.c
//  drast
//
//  Created by Ashwin Paudel on 2022-02-05.
//

#include "argument_parser.h"

static void argument_parser_error(ArgumentParser *arguments);

ArgumentParser *argument_parser_init(int argc, const char **argv, const char **long_name_list) {
    ArgumentParser *argument_parser = malloc(sizeof(ArgumentParser));
    argument_parser->argc = argc;
    argument_parser->argv = argv;
    argument_parser->arg_index = 0;
    argument_parser->arg_current = argument_parser->argv[argument_parser->arg_index];
    argument_parser->long_name_list = long_name_list;

    int i = 0;
    while (long_name_list[i] != NULL)
        i++;

    argument_parser->long_name_list_length = i;

    argument_parser_advance(argument_parser);

    return argument_parser;
}

void argument_parser_parse_arguments(ArgumentParser *arguments, void (*parse_short_argument)(const char, const char *),
                                     void (*parse_long_argument)(const char *, const char *)) {
    while (!argument_parser_is_finished(arguments))
        argument_parser_parse_argument(arguments, parse_short_argument, parse_long_argument);
}

void argument_parser_parse_argument(ArgumentParser *arguments, void (*parse_short_argument)(const char, const char *),
                                    void (*parse_long_argument)(const char *, const char *)) {
    while (!argument_parser_is_finished(arguments)) {
        if (argument_parser_exists(arguments)) {
            if (arguments->arg_current[1] == '-') {
                // Long argument
                for (int i = 0; i < arguments->long_name_list_length; i++) {
                    const char *long_name;
                    if (strncmp(arguments->arg_current + 2, arguments->long_name_list[i],
                                strlen(arguments->long_name_list[i])) == 0) {
                        long_name = arguments->long_name_list[i];
                        const char *value = &arguments->arg_current[strlen(long_name) + 2];
                        if (*value == '\0') {
                            argument_parser_advance(arguments);
                            if (argument_parser_exists(arguments)) {
                                parse_long_argument(long_name, NULL);
                                return;
                            }
                            value = arguments->arg_current;
                        }
                        parse_long_argument(long_name, value);
                        argument_parser_advance(arguments);
                        return;
                    }
                }
                argument_parser_error(arguments);
            } else if (arguments->arg_current[0] == '-') {
                // Short argument
                const char *value = arguments->arg_current + 2;
                char option = arguments->arg_current[1];
                if (arguments->arg_current[2] == '\0') {
                    // Get the next argument
                    argument_parser_advance(arguments);
                    if (argument_parser_exists(arguments)) {
                        parse_short_argument(option, NULL);
                        return;
                    }
                    value = arguments->arg_current;
                }

                parse_short_argument(option, value);
                argument_parser_advance(arguments);
                return;
            }
        } else {
            // Show an error message
            argument_parser_error(arguments);
        }

        argument_parser_error(arguments);
    }
}

bool argument_parser_exists(ArgumentParser *arguments) {
    if (argument_parser_is_finished(arguments))
        return false;
    if (arguments->arg_current[0] == '-')
        return true;
    return false;
}

const char *argument_parser_advance(ArgumentParser *arguments) {
    const char *old_char = arguments->arg_current;
    arguments->arg_index++;
    arguments->arg_current = arguments->argv[arguments->arg_index];

    return old_char;
}

bool argument_parser_is_finished(ArgumentParser *arguments) {
    return arguments->arg_index >= arguments->argc;
}

static void argument_parser_error(ArgumentParser *arguments) {
    printf("Invalid argument: `%s`\n", arguments->arg_current);
    exit(EXIT_FAILURE);
}