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
#include "include/metrics.h"
#include "stm.h"

#define MAX_EMAILS 10

typedef struct file{
    char file_name[FILENAME_MAX];
    int file_id;
    int file_size;
    bool to_delete;
} file;

typedef struct {
    FILE *file;
} FileState;

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
    size_t serverBuffer_size;       // TODO: se actualiza pero no se usa!!

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
    int lastFileList;
    char* activeFile;
    bool fileDoneReading;
    bool newLine;
    bool seenEndOctet;
    bool seenEndOctetR;
    FileState fileState;

    // state machine

    struct state_machine stm;



} Client;

void pop3Read(struct selector_key *key);
void pop3Write(struct selector_key *key);
void pop3Block(struct selector_key *key);
void pop3Close(struct selector_key *key);
unsigned int pop3ReadCommand(struct selector_key* key);
unsigned int pop3ReadFile(struct selector_key* key);
unsigned int pop3WriteFile(struct selector_key* key);
unsigned int pop3WriteCommand(struct selector_key* key);
void pop3Error(unsigned int n, struct selector_key *key);
unsigned int pop3WriteList(struct selector_key* key);
unsigned int pop3ReadList(struct selector_key* key);

stm_state_t parseCommandInBuffer(struct selector_key* key);

 char * byte_stuffing(char* line);

static const struct state_definition states [] = {
    {
        .state = WRITE,
        .on_write_ready = pop3WriteCommand,
    },
    {
        .state = READ,
        .on_read_ready = pop3ReadCommand,
    },
    {
        .state = WRITE_FILE,
        .on_write_ready = pop3WriteFile,
    },
    {
        .state = READ_FILE,
        .on_read_ready = pop3ReadFile,
    },
    {
        .state = READ_LIST,
        .on_read_ready = pop3ReadList,
    },
    {
        .state = WRITE_LIST,
        .on_write_ready = pop3WriteList,
    },
    {
        .state = ERROR_STATE,
        .on_arrival = pop3Error,
    }
};


#endif // POP3_H


