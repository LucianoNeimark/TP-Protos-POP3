#ifndef NUESTRO_PARSER_H
#define NUESTRO_PARSER_H

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define BUFFER_SIZE 4096

typedef enum pop3cmd_state {
    QUIT,
    STAT,
    LIST,
    RETR,
    DELE,
    NOOP,
    RSET,
    TOP,
    UIDL,
    USER,
    PASS,
    APOP,
    CAPA,
    UNDEF,
    ERROR,
    
} pop3cmd_state; 

typedef struct pop3cmd_parser {
    pop3cmd_state state;             // Comando POP3 o error en caso de no reconocerlo
    bool finished;              // Indica cuando salir del while de consumir

    char line[BUFFER_SIZE];     // Vamos almacenando la linea y cuando vemos espacios completamos state y args
    int line_size;

    char * arg1;
    char * arg2;
} pop3cmd_parser;

pop3cmd_parser * parser_init(void);

pop3cmd_state parser_feed(pop3cmd_parser * p, const uint8_t c);

void parser_destroy(pop3cmd_parser * p);

void parser_reset(pop3cmd_parser * p);

#endif