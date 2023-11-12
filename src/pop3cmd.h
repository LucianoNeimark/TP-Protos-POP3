#ifndef POP3CMD_H
#define POP3CMD_H

#include <stdint.h>
#include <stdbool.h>

#include "buffer.h"
#include "nuestro-parser.h"

/** inicializa el parser */
pop3cmd_parser * pop3cmd_parser_init (void);

/** entrega un byte al parser. retorna true si se llego al final  */
enum pop3cmd_state pop3cmd_parser_feed (struct pop3cmd_parser *p, uint8_t b);

/**
 * por cada elemento del buffer llama a `pop3cmd_parser_feed' hasta que
 * el parseo se encuentra completo o se requieren mas bytes.
 *
 * @param errored parametro de salida. si es diferente de NULL se deja dicho
 *   si el parsing se debió a una condición de error
 */
enum pop3cmd_state
pop3cmd_consume(buffer *b, struct pop3cmd_parser *p, bool *errored);

/** libera recursos internos del parser */
void pop3cmd_parser_close(struct pop3cmd_parser *p);

#endif
