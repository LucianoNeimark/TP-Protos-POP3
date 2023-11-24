#ifndef NUESTRO_PARSER_H
#define NUESTRO_PARSER_H

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "constants.h"
#include "logger.h"
#include "netutils.h"

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
    pop3cmd_state state;        // Comando POP3 o error en caso de no reconocerlo
    bool finished;              // Indica cuando salir del while de consumir

    char line[BUFFER_SIZE];     // Vamos almacenando la linea y cuando vemos espacios completamos state y args
    int line_size;

    char * arg1;
    char * arg2;
} pop3cmd_parser;



/* Inicializa el parser y devuelve un puntero del mismo */
pop3cmd_parser * parser_init(void);

/**
* @param p el parser
* @param c el caracter a parsear
*
* Consume un nuevo elemento, y actualiza su maquina de estados interna
*/
pop3cmd_state parser_feed(pop3cmd_parser * p, const uint8_t c);

/**
* @param p el parser
*
* Destruye el parser, liberando la memoria reservada
*/
void parser_destroy(pop3cmd_parser * p);


/**
* @param p el parser
*
* Reinicia el parser, dejandolo de igual manera que el init
*/
void parser_reset(pop3cmd_parser * p);

#endif