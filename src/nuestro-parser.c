#include "nuestro-parser.h"

pop3cmd_parser * parser_init(void) {
    pop3cmd_parser *ret = malloc(sizeof(*ret));
    
    if(ret != NULL) {
        memset(ret, 0, sizeof(*ret));
        ret->state = UNDEF;
        ret->finished = false;
        ret->line_size = 0;
        ret->arg1 = calloc(BUFFER_SIZE, sizeof(char));
        ret->arg2 = calloc(BUFFER_SIZE, sizeof(char));
    }
    return ret;
}

#include <stdio.h>


void process_buffer(pop3cmd_parser * p) {
    if (p->state == UNDEF) {
        if (strcmp(p->line, "QUIT") == 0) {
            p->state = QUIT;
        } else if (strcmp(p->line, "STAT") == 0) {
            p->state = STAT;
        } else if (strcmp(p->line, "LIST") == 0) {
            p->state = LIST;
        } else if (strcmp(p->line, "RETR") == 0) {
            p->state = RETR;
        } else if (strcmp(p->line, "DELE") == 0) {
            p->state = DELE;
        } else if (strcmp(p->line, "NOOP") == 0) {
            p->state = NOOP;
        } else if (strcmp(p->line, "RSET") == 0) {
            p->state = RSET;
        } else if (strcmp(p->line, "TOP") == 0) {
            p->state = TOP;
        } else if (strcmp(p->line, "UIDL") == 0) {
            p->state = UIDL;
        } else if (strcmp(p->line, "USER") == 0) {
            p->state = USER;
        } else if (strcmp(p->line, "PASS") == 0) {
            p->state = PASS;
        } else if (strcmp(p->line, "APOP") == 0) {
            p->state = APOP;
        } else {
            p->state = ERROR;
        }
    } else if (p->state != ERROR && p->arg1[0] == 0) {
        memcpy(p->arg1,p->line,p->line_size);
    } else if (p->state != ERROR && p->arg2[0] == 0) {
        memcpy(p->arg2,p->line,p->line_size);
    } else {
        p->state = ERROR;
    }
}

pop3cmd_state parser_feed(pop3cmd_parser * p, const uint8_t c) {
    
    if (BUFFER_SIZE == p->line_size) {
        
        return ERROR;
    }
    
    switch (c) { 
        case ' ':
            process_buffer(p);
            memset(p->line, 0, p->line_size);
            p->line_size = 0;
            break;
        case '\n':
            if (p->line_size > 0 && p->line[p->line_size-1] == '\r') {
                p->line[--p->line_size] = 0;
                process_buffer(p);
            } else {
                p->state = ERROR;
            }
            p->finished = true;
            break;
        default:
            p->line[p->line_size++] = c;
    }

    return p->state;
}

void parser_destroy(pop3cmd_parser * p) {
    if(p != NULL) {
        if(p->arg1 != NULL) free(p->arg1);
        if(p->arg2 != NULL) free(p->arg2);
        free(p);
    }
}

void parser_reset(pop3cmd_parser * p) {
    if(p->arg1 != NULL) memset(p->arg1, 0, BUFFER_SIZE);
    if(p->arg2 != NULL) memset(p->arg2, 0, BUFFER_SIZE);
    p->line_size = 0;
    p->finished = false;
    p->state = UNDEF;
    memset(p->line,0,BUFFER_SIZE);
}