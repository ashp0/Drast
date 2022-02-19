//
//  argument_parser.h
//  drast
//
//  Created by Ashwin Paudel on 2022-02-05.
//

#ifndef __DRAST_ARGUMENT_PARSER_H__
#define __DRAST_ARGUMENT_PARSER_H__

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef struct {
    int argc;
    const char **argv;

    int arg_index;
    const char *arg_current;

    const char **long_name_list;
    int long_name_list_length;
} ArgumentParser;

ArgumentParser *argument_parser_init(int argc, const char **argv, const char **long_name_list);

void argument_parser_parse_arguments(ArgumentParser *arguments, void (*parse_short_argument)(const char, const char *),
                                     void (*parse_long_argument)(const char *, const char *));

void argument_parser_parse_argument(ArgumentParser *arguments, void (*parse_short_argument)(const char, const char *),
                                    void (*parse_long_argument)(const char *, const char *));

bool argument_parser_exists(ArgumentParser *arguments);

bool argument_parser_is_finished(ArgumentParser *arguments);

const char *argument_parser_advance(ArgumentParser *arguments);

#endif // __DRAST_ARGUMENT_PARSER_H__
