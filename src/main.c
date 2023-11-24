// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
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
#include "include/buffer.h"
#include "include/netutils.h"
#include "include/tests.h"
#include "include/pop3cmd.h"
#include "include/command_handler.h"
#include "include/manager_handler.h"
#include "include/metrics.h" 
#include "include/socket_setup.h"
#include "include/args.h"
#include "include/selector.h"
#include "include/logger.h"

#define SELECTOR_SIZE 1024

bool done = false;

static const fd_handler pop3Handlers = {
    .handle_read = pop3Read,
    .handle_write = pop3Write,
    .handle_block = pop3Block,
    .handle_close = NULL,
};

struct POP3args *args;

static void sigterm_handler(const int signal)
{
    LogInfo("Signal %d, cleaning up and exiting", signal);
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

    client->addr = client_addr;
    client->name = NULL;
    client->fd = client_fd;
    client->state = AUTHORIZATION;
    client->parser = pop3cmd_parser_init();
    client->fileState.file = NULL;
    client->stm.initial = WRITE;
    client->stm.max_state = ERROR_STATE;
    client->stm.states = states; 
    stm_init(&client->stm);
    client->newLine = true;
    client->lastFileList = -1;

    if(client->parser == NULL) {
        err_msg = "Unable to initialize POP3 parser";
        goto handle_error;
    }

    buffer_init(&client->serverBuffer, BUFFER_SIZE, client->serverBuffer_data);

    buffer_init(&client->clientBuffer, BUFFER_SIZE, client->clientBuffer_data);

    char *s = "+OK POP3 server ready\r\n";

    // Como el buffer esta vacío, podemos escribir directamente en el
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

    LogInfo("New client connected from %s", sockaddr_to_human_buffered((struct sockaddr*)&client_addr));

    return;

handle_error:
    if (*err_msg)
    {
        LogError(err_msg);
    }
    if (client != NULL) {
        parser_destroy(client->parser);
    }
    if (client_fd != -1)
    {
        close(client_fd);
    }
    if (client != NULL)
    {
        free(client);
    }
}

int main(int argc, char **argv)
{

    int ret = -1;

    args = (struct POP3args *)malloc(sizeof(struct POP3args));

    parse_args(argc, argv, args);

    const char *err_msg = 0;

    int server = -1;
    int managerSocket = -1;

    LogInfo("Starting POP3 server on %s:%d", args->POP3_addr, args->POP3_port);

    const struct selector_init init = {
        .signal = SIGALRM,
        .select_timeout = {
            .tv_sec = 10,
            .tv_nsec = 0,
        },
    };

    fd_selector selector = NULL;
    selector_status selectStatus = selector_init(&init);

    if (selectStatus != SELECTOR_SUCCESS)
        goto finally;

    server = setupServerSocket(args->POP3_addr, args->POP3_port);
    if (server == -1)
    {
        LogError("Unable to setup server socket");
        goto finally;
    }

    managerSocket = setupManagerSocket(args->mng_addr, args->mng_port);
    if (managerSocket == -1)
    {
        LogError("Unable to setup manager socket");
        goto finally;
    }

    signal(SIGTERM, sigterm_handler);
    signal(SIGINT, sigterm_handler);

    if (selector_fd_set_nio(server) == -1)
    {
        err_msg = "Unable to set server socket as non-blocking";
        goto finally;
    }

    selector = selector_new(SELECTOR_SIZE);
    if (selector == NULL)
    {
        err_msg = "Unable to create selector";
        goto finally;
    }

    const fd_handler passiveHandler = {
        .handle_read = pop3_handle_connection,
        .handle_write = NULL,
        .handle_close = NULL,
        .handle_block = NULL,
    };

    selectStatus = selector_register(selector, server, &passiveHandler, OP_READ, NULL);
    if (selectStatus != SELECTOR_SUCCESS)
    {
        err_msg = "Unable to register server socket";
        goto finally;
    }

    const fd_handler managerHandler = {
        .handle_read = manager_handle_connection,
        .handle_write = NULL,
        .handle_close = NULL,
        .handle_block = NULL};

    selectStatus = selector_register(selector, managerSocket, &managerHandler, OP_READ, NULL);
    if (selectStatus != SELECTOR_SUCCESS)
    {
        goto finally;
    }

    while (!done)
    {
        selectStatus = selector_select(selector);
        if (selectStatus != SELECTOR_SUCCESS)
        {
            goto finally;
        }
    }

    ret = 0;

finally:
    if (selectStatus != SELECTOR_SUCCESS)
    {
        LogError("Error in selector: %s", selector_error(selectStatus));
        if (selectStatus == SELECTOR_IO)
        {
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
    if (managerSocket >= 0)
    {
        close(managerSocket);
    }
    return ret;
}