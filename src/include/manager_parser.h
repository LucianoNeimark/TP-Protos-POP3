#ifndef MANAGER_PARSER_H
#define MANAGER_PARSER_H

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h> 

#include "constants.h"

typedef enum manager_cmd_state {
    M_USERS,
    M_ADD,
    M_DELE,
    M_HISTORIC,
    M_CONCURRENT,
    M_TRANSFERRED,
    M_STOP,
    M_CAPA,
    M_UNDEF,
    M_ERROR
} manager_cmd_state; 

typedef struct manager_cmd_parser {
    manager_cmd_state state;
    bool finished;

    char line[UDP_BUFFER_SIZE];
    int line_size;

    char * arg1;
} manager_cmd_parser;

manager_cmd_parser * manager_parser_init(void);

manager_cmd_state manager_parser_analyze(manager_cmd_parser * p, uint8_t * input, size_t input_length);

void manager_parser_destroy(manager_cmd_parser * p);

void manager_parser_reset(manager_cmd_parser * p);

#endif