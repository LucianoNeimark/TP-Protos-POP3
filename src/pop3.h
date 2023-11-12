#ifndef POP3_H
#define POP3_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include "buffer.h"

#define MAX_EMAILS 10

typedef struct file{
    char file_name[FILENAME_MAX];
    int file_id;
    int file_size;
    bool deleted;
} file;

typedef enum client_state {
    AUTHORIZATION,
    TRANSACTION,
    UPDATE
} client_state;

typedef struct Client {
    uint32_t fd;
    struct buffer* serverBuffer;
    struct buffer* clientBuffer;
    char * name;
    char * password;
    client_state state;
    file files[MAX_EMAILS];
    unsigned int file_cant;
} Client;






#endif // POP3_H


