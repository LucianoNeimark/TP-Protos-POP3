#ifndef ARGS_H_kFlmYm1tW9p5npzDr2opQJ9jM8
#define ARGS_H_kFlmYm1tW9p5npzDr2opQJ9jM8

#include <stdbool.h>
#include "pop3.h"
#include "constants.h"
#include "users.h"
#include "../logger/logger.h"

#define MAX_DIR_LEN 256

extern struct POP3args *args;

struct POP3args {
    char           *POP3_addr;
    unsigned short  POP3_port;

    char *          mng_addr;
    unsigned short  mng_port;

    bool            disectors_enabled;
    char  directory[MAX_DIR_LEN];
};

/**
 * Interpreta la linea de comandos (argc, argv) llenando
 * args con defaults o la seleccion humana. Puede cortar
 * la ejecución.
 */
void 
parse_args(const int argc, char **argv, struct POP3args *args);

#endif

