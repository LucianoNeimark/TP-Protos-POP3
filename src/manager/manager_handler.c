#include "../include/manager_handler.h"


manager_cmd_state execute_manager_command(manager_state * manager);

void handleCapa(char * request, char * response);
void handleUsers(char * request, char * response);
void handleAddUser(char * request, char * response);
void handleDeleteUser(char * request, char * response);
void handleHistoric(char * request, char * response);
void handleConcurrent(char * request, char * response);
void handleTransferred(char * request, char * response);
void handleStop(char * request, char * response);

typedef struct command_mapping {
    manager_cmd_state state;
    manager_cmd_handler handler;
} command_mapping;

command_mapping command_mappings[] = {
    {M_CAPA, handleCapa},
    {M_USERS, handleUsers},
    {M_ADD, handleAddUser},
    {M_DELE, handleDeleteUser},
    {M_HISTORIC, handleHistoric},
    {M_CONCURRENT, handleConcurrent},
    {M_TRANSFERRED, handleTransferred},
    {M_STOP, handleStop},
    {0, NULL}
};

void manager_handle_connection(struct selector_key *key) {
    struct manager_state the_manager;

    the_manager.manager_addr_len = sizeof(the_manager.manager_addr);
    memset(the_manager.server_buffer, 0, UDP_BUFFER_SIZE);
    memset(the_manager.manager_buffer, 0, UDP_BUFFER_SIZE);

    ssize_t bytes_read = recvfrom(key->fd, the_manager.manager_buffer, UDP_BUFFER_SIZE, 0,
                            (struct sockaddr *) &the_manager.manager_addr, &the_manager.manager_addr_len);

    if (bytes_read <= 0) {
        LogError("Unable to recieve message from manager: %s", strerror(errno));
        manager_parser_destroy(the_manager.parser);
        return ;
    }

    the_manager.parser = manager_parser_init();

    if(the_manager.parser->state == M_ERROR) {
        LogError("Unable to initialize manager parser");
        return ;
    }

    /*manager_cmd_state parser_state = */manager_parser_analyze(the_manager.parser, the_manager.manager_buffer, UDP_BUFFER_SIZE);

    manager_cmd_state execute_state = execute_manager_command(&the_manager);

    if (sendto(key->fd, the_manager.server_buffer, sizeof(the_manager.server_buffer), 0,
               (struct sockaddr *) &the_manager.manager_addr, the_manager.manager_addr_len) < 0) {
        LogError("Unable to send manager message to client: %s", strerror(errno));
        manager_parser_destroy(the_manager.parser);
        return;
    }

    // if(parser_state == M_ERROR) {
    //     LogError("Unable to parse manager command");
    //     manager_parser_destroy(the_manager.parser);
    //     return ;
    // }

    if(execute_state == M_ERROR) {
        LogError("Unable to execute manager command");
        manager_parser_destroy(the_manager.parser);
        return ;
    }
    
    char command[UDP_BUFFER_SIZE];
    memcpy(command, the_manager.manager_buffer, strlen((char *)the_manager.manager_buffer) - 1);
    command[strlen((char *)the_manager.manager_buffer) - 1] = '\0';

    LogInfo("Recieved manager message: %s", command);
    manager_parser_destroy(the_manager.parser);
}

manager_cmd_state execute_manager_command(manager_state * manager) {    
    char response[UDP_BUFFER_SIZE - 1];
    int i = 0;
    while(command_mappings[i].handler != NULL){
        if(command_mappings[i].state == manager->parser->state){
            command_mappings[i].handler(manager->parser->arg1, response);
            memcpy(manager->server_buffer, response, strlen(response));
            return manager->parser->state;
        }
        i++;
    }
    char error_buffer[UDP_BUFFER_SIZE - 1];
    memcpy(error_buffer, manager->manager_buffer, strlen((char *)manager->manager_buffer) - 1);
    error_buffer[strlen((char *)manager->manager_buffer) - 1] = '\0';
    LogError("Unknown command: %s", error_buffer);
    char * unknown = "-ERR Unknown command. Use CAPA for available commands.\r\n";
    memcpy(manager->server_buffer, unknown, strlen(unknown));
    return manager->parser->state;
}

void handleCapa(char * request, char * response) {
    sprintf(response, "+OK Capability list follows:\r\n"
                        "CAPA\r\n"
                        "USERS\r\n"
                        "ADD\r\n"
                        "DEL\r\n"
                        "HIST\r\n"
                        "CONC\r\n"
                        "TRANS\r\n"
                        "STOP\r\n"
                        ".\r\n");
}

void handleUsers(char * request, char * response) {
    struct user * users = get_users();

    char aux[UDP_BUFFER_SIZE - 1];
    int aux_size = 0;

    int user_count = get_user_count();

    int offset = atoi(request);     // atoi(null) = 0;

    aux_size += sprintf(aux + aux_size, "+OK User list follows (%d - %d):\n", offset, (offset+9 > user_count)? user_count:offset+9);

    if (request == NULL) {
        sprintf(response, "-ERR Please specify offset\r\n");
        return ;       
    }

    if (offset < 1 || offset > user_count) {
        sprintf(response, "-ERR Offset must be between 1 and %d\r\n", user_count);
        return ;
    }

    for (int i = offset-1; i < user_count && i < offset + 9; i++) {
        aux_size += sprintf(aux + aux_size, "%d %s:%s\r\n", i+1, users[i].name, users[i].pass);
    }

    aux_size += sprintf(aux + aux_size, ".\r\n");

    if (aux_size >= UDP_BUFFER_SIZE) {
        sprintf(response, "-ERR UDP buffer size limit exceeded");
        return ;
    }

    strcpy(response, aux);
}

void handleAddUser(char * request, char * response) {
    switch (user_add(request)) {
        case 0:
            sprintf(response, "+OK\r\n");
            break;
        case -1:
            sprintf(response, "-ERR User already exists\r\n");
            break;
        case -2:
            sprintf(response, "-ERR Invalid user format\r\n");
            break;
        case -3:
            sprintf(response, "-ERR User limit reached: %d\r\n", MAX_USERS);
            break;
        case -4:
            sprintf(response, "-ERR Username length exceeded: %d\r\n", MAX_USER_LENGTH);
            break;
        case -5:
            sprintf(response, "-ERR Password length exceeded: %d\r\n", MAX_PASS_LENGTH);
            break;
        default:
            sprintf(response, "-ERR\r\n");
            break;
    }
}
void handleDeleteUser(char * request, char * response) {
    if (user_remove(request) == 0) {
        sprintf(response, "+OK\r\n");
    } else {
        sprintf(response, "-ERR User not found\r\n");
    }
}

void handleHistoric(char * request, char * response) {
    struct metrics * metrics = get_metrics();

    sprintf(response, "+OK Historic connections: %d\r\n", metrics->hist_connections);
}

void handleConcurrent(char * request, char * response) {
    struct metrics * metrics = get_metrics();

    sprintf(response, "+OK Concurrent connections: %d\r\n", metrics->conc_connections);
}

void handleTransferred(char * request, char * response) {
    struct metrics * metrics = get_metrics();

    sprintf(response, "+OK Bytes transferred: %d\r\n", metrics->bytes_transferred);
}

extern bool done;

void handleStop(char * request, char * response) {
    done = true;

    sprintf(response, "+OK Shutting down\r\n");
}
