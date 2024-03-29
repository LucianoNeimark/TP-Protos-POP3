// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "../include/manager_parser.h"

manager_cmd_state manager_parser_feed(manager_cmd_parser * p, const uint8_t c);

//the parser is initialized every time a new command is received
manager_cmd_parser * manager_parser_init(void) {
    // LogDebug("Initializing manager parser...");
    manager_cmd_parser *ret = malloc(sizeof(*ret));
    if (ret == NULL) {
        LogError("Unable to allocate memory for manager parser");
        return NULL;
    }
    
    memset(ret, 0, sizeof(*ret));
    ret->state = M_UNDEF;
    ret->finished = false;
    ret->line_size = 0;
    ret->arg1 = calloc(BUFFER_SIZE, sizeof(char));

    // LogDebug("Manager parser initialized");
    return ret;
}

#include <stdio.h>

struct state_mapping {
    const char *command;
    manager_cmd_state state;
};

struct state_mapping state_mappings[] = {
    {"CAPA", M_CAPA},
    {"USERS", M_USERS},
    {"ADD", M_ADD},
    {"DEL", M_DELE},
    {"HIST", M_HISTORIC},
    {"CONC", M_CONCURRENT},
    {"TRANS", M_TRANSFERRED},
    {"STOP", M_STOP}
};

enum manager_cmd_state getParserState(const char *line) {
    for (size_t i = 0; i < sizeof(state_mappings) / sizeof(state_mappings[0]); ++i) {
        if (strcmp(line, state_mappings[i].command) == 0) {
            return state_mappings[i].state;
        }
    }
    return M_ERROR; // Default state for unrecognized commands
}

void m_process_buffer(manager_cmd_parser * p) {
    if (p->state == M_UNDEF) {
        p->state = getParserState(p->line);
    } else if (p->state != M_ERROR && p->arg1[0] == 0) {
        memcpy(p->arg1, p->line, p->line_size);
    }
}

manager_cmd_state manager_parser_analyze(manager_cmd_parser * p, uint8_t * input, size_t input_length) {
    uint8_t * aux = malloc(input_length);
    if (aux == NULL) {
        return M_ERROR;
    }

    memcpy(aux, input, input_length);

    size_t i = 0;
    while (i < input_length && aux[i] != 0) {
        manager_parser_feed(p, aux[i++]);
    }
    
    free(aux);
    return p->state;
}

manager_cmd_state manager_parser_feed(manager_cmd_parser * p, uint8_t c) {
    if(p->state == M_UNDEF) c = toupper(c);
    if (BUFFER_SIZE <= p->line_size) {
        return M_ERROR;
    }
    
    switch (c) { 
        case ' ':
            m_process_buffer(p);
            memset(p->line, 0, p->line_size);
            p->line_size = 0;
            break;
        case '\n':
            if (p->line_size > 0) {
                p->line[p->line_size] = 0;
                m_process_buffer(p);
            } else {
                p->state = M_ERROR;
            }
            p->finished = true;
            break;
        default:
            p->line[p->line_size++] = c;
    }

    return p->state;
}

void manager_parser_destroy(manager_cmd_parser * p) {
    // LogDebug("Destroying manager parser...");
    if(p != NULL) {
        if(p->arg1 != NULL) free(p->arg1);
        free(p);
    }
    // LogDebug("Manager parser destroyed");
}

void manager_parser_reset(manager_cmd_parser * p) {
    if(p->arg1 != NULL) memset(p->arg1, 0, UDP_BUFFER_SIZE);
    p->line_size = 0;
    p->finished = false;
    p->state = M_UNDEF;
    memset(p->line, 0, UDP_BUFFER_SIZE);
}