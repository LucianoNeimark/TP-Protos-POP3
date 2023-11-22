/**
 * main.c - servidor proxy socks concurrente
 *
 * Interpreta los argumentos de línea de comandos, y monta un socket
 * pasivo. Por cada nueva conexión lanza un hilo que procesará de
 * forma bloqueante utilizando el protocolo SOCKS5.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <errno.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <unistd.h>
#include "buffer.h"
#include "netutils.h"
#include "tests.h"
#include "pop3cmd.h"
#include "command-handler.h"
#include "include/manager_handler.h"
#include "include/metrics.h" // temporal (hasta que corramos pop3_handle_connection)
#include "args.h"
#include "selector.h"
#include "logger/logger.h"

#define SELECTOR_SIZE 1024

static bool done = false;

static int setupManagerSocket(char *addr, int port);

// llamamos a nuestros metodos de leer y escribir para que los use el selector cuando le toca a cada cliente.
//  Donde escribiamos ahora copiamos al buffer y seteamos la intencion
//  Cuando me toca se llama a estos metodos de abajo dependiendo la intencion que haya seteado antes.
static const fd_handler pop3Handlers = {
    .handle_read = pop3Read,
    .handle_write = pop3Write,
    .handle_block = pop3Block,
    .handle_close = pop3Close,
};

struct POP3args *args;

static void sigterm_handler(const int signal)
{
    LogInfo("signal %d, cleaning up and exiting\n", signal);
    done = true;
    exit(0);
}

/**
 * estructura utilizada para transportar datos entre el hilo
 * que acepta sockets y los hilos que procesa cada conexión
 */

struct connection
{
    int fd;
    socklen_t addrlen;
    struct sockaddr_in6 addr;
};

// #include "socks5.h

/**
 * maneja cada conexión entrante
 *
 * @param fd   descriptor de la conexión entrante.
 * @param caddr información de la conexión entrante.
 */

static void pop3_handle_connection(/*int fd, const struct sockaddr *caddr*/ struct selector_key *key)
{
    const char *err_msg = 0;

    struct sockaddr_storage client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    int client_fd = accept(key->fd, (struct sockaddr *)&client_addr, &client_addr_len);
    Client *client = calloc(1, (sizeof(struct Client)));

    if (client_fd == -1)
    {
        err_msg = "Unable to accept connection";
        goto handle_error;
    }

    if (client == NULL)
    {
        err_msg = "Unable to allocate memory for client";
        goto handle_error;
    }

    client->name = NULL;
    client->fd = client_fd;
    client->state = AUTHORIZATION;
    client->parser = pop3cmd_parser_init();
    client->fileState.file = NULL;
    client->stm.initial = WRITE;
    client->stm.max_state = ERROR_STATE;
    client->stm.states = states; // para cada estado un afuncion;
    stm_init(&client->stm);
    client->newLine = true;
    client->lastFileList = -1;

    buffer_init(&client->serverBuffer, BUFFER_SIZE, client->serverBuffer_data);

    buffer_init(&client->clientBuffer, BUFFER_SIZE, client->clientBuffer_data);

    char *s = "+OK POP3 server ready\r\n";

    // Since the output buffer is empty, we can write directly to it
    for (int i = 0; s[i]; i++)
    {
        buffer_write(&client->serverBuffer, s[i]);
    }

    client->serverBuffer_size = strlen(s);

    if (selector_fd_set_nio(client_fd) == -1)
    {
        err_msg = "Unable to set client socket as non-blocking";
        goto handle_error;
    }

    selector_status status = selector_register(key->s, client_fd, &pop3Handlers, OP_WRITE, client);
    if (status != SELECTOR_SUCCESS)
    {
        err_msg = "Unable to register client socket";
        goto handle_error;
    }

    metrics_new_connection();

    return;

handle_error:
    if(err_msg){
        LogError(err_msg);
    }
    if (client_fd != -1)
    {
        close(client_fd);
    }
    if (client != NULL) {
        free(client);
    }
}

int main(int argc, char **argv)
{

    int ret = -1;

    args = (struct POP3args *)malloc(sizeof(struct POP3args));

    parse_args(argc, argv, args);

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(args->POP3_port);

    const char *err_msg = 0;

    LogInfo("Starting POP3 server on %s:%d", args->POP3_addr, args->POP3_port);

    const int server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    const struct selector_init init = {
        .signal = SIGALRM,
        .select_timeout = {
            .tv_sec = 10,
            .tv_nsec = 0,
        },
    };

    fd_selector selector = NULL;
    selector_status selectStatus = selector_init(&init);

    if (server < 0)
    {
        err_msg = "Unable to create socket\n";
        goto finally;
    }

    if (selectStatus != SELECTOR_SUCCESS)
        goto finally;

    // man 7 ip. no importa reportar nada si falla.
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

    if (bind(server, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        err_msg = "Unable to bind socket";
        goto finally;
    }

    if (listen(server, 20) < 0)
    {
        err_msg = "Unable to listen";
        goto finally;
    }

    LogRaw("Listening on TCP %s:%d\n", args->POP3_addr, args->POP3_port);

    int managerSocket = setupManagerSocket(args->mng_addr, args->mng_port);
    if (managerSocket == -1) {
        // err_msg = "Unable to setup manager socket";        
        goto finally;
    }

    // registrar sigterm es útil para terminar el programa normalmente.
    // esto ayuda mucho en herramientas como valgrind.
    signal(SIGTERM, sigterm_handler);
    signal(SIGINT, sigterm_handler);

    // Trabajamos aca
    if (server != -1 && selector_fd_set_nio(server) == -1)
    {
        err_msg = "Unable to set server socket as non-blocking";
        goto finally;
    }

    selector = selector_new(SELECTOR_SIZE);
    if (selector == NULL) {
        err_msg = "Unable to create selector";
        goto finally;
    }

    const fd_handler passiveHandler = {
        .handle_read = pop3_handle_connection, // TODO Es el saludo, registra el cliente y sus cosas en el selector.
        .handle_write = NULL,
        .handle_close = NULL,
        .handle_block = NULL,
    };

    selectStatus = selector_register(selector, server, &passiveHandler, OP_READ, NULL);
    if (selectStatus != SELECTOR_SUCCESS) {
        err_msg = "Unable to register server socket";
        goto finally;
    }

    const fd_handler managerHandler = {
        .handle_read = manager_handle_connection,
        .handle_write = NULL,
        .handle_close = NULL,
        .handle_block = NULL};

    selectStatus = selector_register(selector, managerSocket, &managerHandler, OP_READ, NULL);
    if (selectStatus != SELECTOR_SUCCESS) {
        goto finally;
    }

    while (!done)
    {
        selectStatus = selector_select(selector);
        if (selectStatus != SELECTOR_SUCCESS) {
            goto finally;
        }
    }

    // err_msg = 0;
    ret = 0;

finally:
    if(selectStatus != SELECTOR_SUCCESS){
        LogError("Error in selector: %s", selector_error(selectStatus));
        if (selectStatus == SELECTOR_IO) {
            LogRaw("More info: %s", strerror(errno));
        }
    }
    if (err_msg)
    {
        LogError(err_msg);
        ret = 1;
    }
    if (selector != NULL)
    {
        selector_destroy(selector);
    }
    selector_close();
    if (server >= 0)
    {
        close(server);
    }
    // if(managerSocket >= 0){
    //     close(managerSocket);
    // }
    return ret;
}

static int setupManagerSocket(char *addr, int port)
{
    const char * err_msg = 0;
    const int managerSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (managerSocket < 0)
    {
        err_msg = "Unable to create manager socket";
        goto manager_error;
    }

    // Para reusar el socket
    setsockopt(managerSocket, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

    struct sockaddr_in manager_addr;
    memset(&manager_addr, 0, sizeof(manager_addr));
    manager_addr.sin_family = AF_INET;
    manager_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, addr, &manager_addr.sin_addr) < 0)
    {
        err_msg = "Unable to parse manager address";
        goto manager_error;
    }

    if (bind(managerSocket, (struct sockaddr *)&manager_addr, sizeof(manager_addr)) < 0)
    {
        err_msg = "Unable to bind manager socket";
        goto manager_error;
    }

    LogRaw("Manager listening on UDP %s:%d\n", addr, port);

    return managerSocket;

manager_error:
    if (err_msg)
    {
        LogError(err_msg);
    }
    if (managerSocket != -1)
        close(managerSocket);
    return -1;
}