#ifndef POP3_H
#define POP3_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>
#include <unistd.h>

#include <sys/socket.h>

#include "parser.h"
#include "netutils.h"
#include "selector.h"
#include "pop3cmd.h"
#include "metrics.h"
#include "users.h"
#include "stm.h"


#define MAX_EMAILS 10

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0x4000
#endif

/*
* Estructura que guarda informacion sobre los archivos.
*/
typedef struct file{
    char file_name[FILENAME_MAX];
    int file_id;
    int file_size;
    bool to_delete;
} file;

typedef struct {
    FILE *file;
} FileState;


/*
* Definicion de los posibles estados de un cliente.
*/
typedef enum client_state {
    AUTHORIZATION,
    TRANSACTION,
    UPDATE,
    CLOSED
} client_state;


/*
* Estructura que almacena informacion y utilidades sobre el cliente.
*/
typedef struct Client {
    struct sockaddr_storage addr;

    uint32_t fd;

    //Buffers.
    struct buffer serverBuffer;
    uint8_t serverBuffer_data[BUFFER_SIZE];
    size_t serverBuffer_size;   

    struct buffer clientBuffer;
    uint8_t clientBuffer_data[BUFFER_SIZE];

    struct pop3cmd_parser * parser;

    //USER and PASS
    char * name;
    char * password;

    //File management.
    client_state state;
    file files[MAX_EMAILS];
    unsigned int file_cant;
    unsigned int active_file_cant;
    unsigned int active_file_size;
    int lastFileList;
    char* activeFile;
    bool fileDoneReading;
    bool newLine;
    FileState fileState;

    // state machine
    struct state_machine stm;

} Client;

/*
* Maneja el evento "Read" de un cliente. (Selector.c)
*/
void pop3Read(struct selector_key *key);


/*
* Maneja el evento "Write" de un cliente. (Selector.c)
*/
void pop3Write(struct selector_key *key);


/*
* Maneja el evento "Error" de un cliente. (Selector.c) 
*/
void pop3Error(unsigned int n, struct selector_key *key);




/*
* Maneja el evento "Block" de un cliente. (Selector.c)
*/
void pop3Block(struct selector_key *key);

/*
* Lectura para comandos cliente.
*/
unsigned int pop3ReadCommand(struct selector_key* key);

/*
* Lectura para mails del cliente.
*/
unsigned int pop3ReadFile(struct selector_key* key);

/*
* Escritura para mails del cliente.
*/
unsigned int pop3WriteFile(struct selector_key* key);

/*
* Escritura para comandos cliente.
*/
unsigned int pop3WriteCommand(struct selector_key* key);

/*
* Escritura para el comando LIST.
*/
unsigned int pop3WriteList(struct selector_key* key);

/*
* Lectura para el comando LIST.
*/
unsigned int pop3ReadList(struct selector_key* key);

/*
* Maneja el cierre de conexion de un cliente.
*/
void closeConnection(unsigned int state, struct  selector_key *key);

/*
* Calcula el comando a ejecutar con el parser.
*/
stm_state_t parseCommandInBuffer(struct selector_key* key);


/*
* Libera recursos del cliente
*/
void user_free(Client * client);

/*
* Definicion de los estados para la maquina de estados
*/
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
        .state = CLOSE_STATE,
        .on_arrival = closeConnection,
    },
    {
        .state = ERROR_STATE,
        .on_arrival = pop3Error,
    }
};


#endif // POP3_H


