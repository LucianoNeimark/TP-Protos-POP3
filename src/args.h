#ifndef ARGS_H_kFlmYm1tW9p5npzDr2opQJ9jM8
#define ARGS_H_kFlmYm1tW9p5npzDr2opQJ9jM8

#include <stdbool.h>
#include "pop3.h"


#define MAX_USERS 10
#define MAX_DIR_LEN 256

// Declare args as extern to use it on other files that include args.h FIXME estara bien o es poco seguro? usar metodos para acceder quizas
extern struct POP3args *args;

struct users {
    char *name;
    char *pass;
};

// struct doh {
//     char           *host;
//     char           *ip;
//     unsigned short  port;
//     char           *path;
//     char           *query;
// };

struct POP3args {
    char           *POP3_addr;
    unsigned short  POP3_port;

    // char *          mng_addr;
    // unsigned short  mng_port;

    bool            disectors_enabled;

    // struct doh      doh;
    struct users    users[MAX_USERS];
    size_t          nusers;
    char  directory[MAX_DIR_LEN];
};

/**
 * Interpreta la linea de comandos (argc, argv) llenando
 * args con defaults o la seleccion humana. Puede cortar
 * la ejecución.
 */
void 
parse_args(const int argc, char **argv, struct POP3args *args);

bool
check_username(char * username, char* password, struct users* users);

#endif

