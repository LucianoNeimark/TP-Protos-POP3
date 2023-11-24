#ifndef MANAGER_HANDLER_H
#define MANAGER_HANDLER_H

#include <stddef.h>
#include <stdint.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "selector.h"
#include "buffer.h"
#include "constants.h"
#include "manager_parser.h"
#include "metrics.h"
#include "users.h"
#include "../logger/logger.h"

#include <stdio.h> // temp


typedef struct manager_state {
    struct sockaddr_storage manager_addr;
    socklen_t manager_addr_len;

    uint8_t server_buffer[UDP_BUFFER_SIZE];
    uint8_t manager_buffer[UDP_BUFFER_SIZE];

    struct manager_cmd_parser * parser;
} manager_state;

void manager_handle_connection(struct selector_key *key);

typedef void (*manager_cmd_handler)(char * request, char * response);

#endif