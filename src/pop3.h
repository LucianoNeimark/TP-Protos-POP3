#ifndef POP3_H
#define POP3_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include "buffer.h"
#include "nuestro-parser.h"
#include "netutils.h"
#include "selector.h"
#include "pop3cmd.h"

#define MAX_EMAILS 10

typedef struct file{
    char file_name[FILENAME_MAX];
    int file_id;
    int file_size;
    bool to_delete;
} file;

typedef enum client_state {
    AUTHORIZATION,
    TRANSACTION,
    UPDATE,
    CLOSED
} client_state;

typedef struct Client {
    uint32_t fd;

    struct buffer serverBuffer;
    uint8_t serverBuffer_data[BUFFER_SIZE];
    size_t serverBuffer_size;

    struct buffer clientBuffer;
    uint8_t clientBuffer_data[BUFFER_SIZE];


    struct pop3cmd_parser * parser;

    char * name;
    char * password;

    client_state state;
    file files[MAX_EMAILS];
    unsigned int file_cant;
    unsigned int active_file_cant;
    unsigned int active_file_size;
} Client;

void pop3Read(struct selector_key *key);
void pop3Write(struct selector_key *key);
void pop3Block(struct selector_key *key);
void pop3Close(struct selector_key *key);

#endif // POP3_H


