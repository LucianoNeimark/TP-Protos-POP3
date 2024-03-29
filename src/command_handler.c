// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "include/command_handler.h"
#include "include/pop3.h"

void write_to_client(Client * client, char * message){
    size_t limit;
    uint8_t *buffer;
    ssize_t count;

    buffer = buffer_write_ptr(&client->serverBuffer, &limit);
    count = snprintf((char *) buffer, limit, "%s", message);
    buffer_write_adv(&client->serverBuffer, count);
}

stm_state_t handleQuitNonAuth(char * arg1, char * arg,  struct selector_key* key) {
    struct Client *client = key->data;

    write_to_client(client, "+OK POP3 server signing off\r\n");
    LogInfo("QUIT: Signed off from %s", sockaddr_to_human_buffered((struct sockaddr*)&client->addr));

    client->state = CLOSED;
    return WRITE;
}


stm_state_t handleQuitAuth(char * arg1, char * arg, struct selector_key* key) {
    struct Client *client = key->data;
    client->state = UPDATE;

    int error_removing = 0;
    for(unsigned int i = 0; !error_removing && i < client->file_cant; i++){
        if(client->files[i].to_delete){
            error_removing = remove_file(client->files[i].file_name, client);
            if (!error_removing) {
                client->file_cant--;
            }
        }
    }

    if (error_removing) {
        LogError("QUIT: Some deleted messages not removed");
        write_to_client(client, "-ERR some deleted messages not removed\r\n");
        return WRITE;
    }

    char * message = malloc(100);
    if (message == NULL) {
        LogError("Unable to allocate memory for message");
        return ERROR_STATE;
    }

    if (client->file_cant == 0) {
        sprintf(message, "+OK POP3 server signing off (maildrop empty)\r\n");
    } else {
        sprintf(message, "+OK POP3 server signing off (%d message%s left)\r\n", client->file_cant, client->file_cant == 1? "":"s");
    }
    LogInfo("QUIT: User %s signed off from %s", client->name, sockaddr_to_human_buffered((struct sockaddr*)&client->addr));

    write_to_client(client, message);
    free(message);

    client->state = CLOSED;

    return WRITE;
}

stm_state_t handleStat(char * arg1, char * arg2, struct selector_key* key) {
    struct Client *client = key->data;
    char * message = malloc(100);
    sprintf(message,"+OK %d %d\r\n", client->active_file_cant, client->active_file_size);
    write_to_client(client, message);
    free(message);
    return WRITE;
}

stm_state_t handleList(char * arg1, char * arg2, struct selector_key* key) {
    struct Client *client = key->data;

    if (client->files[0].file_id == 0) {
        LogError("LIST: Error opening user folder");
        write_to_client(client, "-ERR Error opening user folder.\r\n");
        return WRITE;
    }

    if(!strlen(arg1)){
        return pop3ReadList(key);
        
    } else {
        char * message = malloc(100);
        if (message == NULL) {
            LogError("Unable to allocate memory for message");
            return ERROR_STATE;
        }

        for(unsigned int i = 0; i < client->file_cant; i++){
            if(client->files[i].file_id == atoi(arg1)){
                if (!client->files[i].to_delete) {
                    LogInfo("LIST: message %d has %d octets", atoi(arg1), client->files[i].file_size);
                    sprintf(message,"+OK %d %d\r\n", client->files[i].file_id, client->files[i].file_size);
                } else {
                    LogError("LIST: message %d is deleted", atoi(arg1));
                    sprintf(message,"-ERR message is deleted\r\n");
                }
                write_to_client(client, message);
                free(message);
                return WRITE;
            }
        }
        LogError("LIST: message %d not found", atoi(arg1));
        sprintf(message,"-ERR no such message \r\n");
        write_to_client(client, message);
        free(message);
        return WRITE;
    }
}

stm_state_t handleRetr(char * arg1, char * arg2, struct selector_key* key) {
    struct Client *client = key->data;
    for(unsigned int i = 0; i < client->file_cant; i++){
        if(client->files[i].file_id == atoi(arg1) && !client->files[i].to_delete){
            char * message = malloc(100);
            LogInfo("RETR: message %d sent. Has %d octets", atoi(arg1), client->files[i].file_size);
            sprintf(message, "+OK %d octets\r\n", client->files[i].file_size);
            write_to_client(client, message);
            client->activeFile = client->files[i].file_name;
            client->fileDoneReading = false;
            
            free(message);
            return WRITE_FILE;
        }
    }
    LogError("RETR: message %d not found", atoi(arg1));
    write_to_client(client, "-ERR no such message\r\n");
    return WRITE;
}

stm_state_t handleDele(char * arg1, char * arg2, struct selector_key* key) {
    struct Client *client = key->data;
    for(unsigned int i = 0; i < client->file_cant; i++){
        if(client->files[i].file_id == atoi(arg1)){
            char * message = malloc(100);

            if (!client->files[i].to_delete) {
                client->files[i].to_delete = true;
                client->active_file_cant--;
                client->active_file_size -= client->files[i].file_size;
                LogInfo("Message %d deleted", client->files[i].file_id);
                sprintf(message, "+OK message %d deleted\r\n", client->files[i].file_id);
            } else {
                LogError("Message %d already deleted", client->files[i].file_id);
                sprintf(message, "-ERR message %d already deleted\r\n", client->files[i].file_id);
            }

            write_to_client(client, message);
            free(message);
            return WRITE;
        }
    }

    write_to_client(client, "-ERR no such message\r\n");
    return WRITE;
}

stm_state_t handleNoop(char * arg1, char * arg2, struct selector_key* key) {
    struct Client *client = key->data;
    write_to_client(client, "+OK\r\n");
    return WRITE;
}

stm_state_t handleRset(char * arg1, char * arg2, struct selector_key* key) {
    struct Client *client = key->data;
    for(unsigned int i = 0; i < client->file_cant; i++){
        if (client->files[i].to_delete) {
            client->files[i].to_delete = false;
            client->active_file_cant++;
            client->active_file_size += client->files[i].file_size;
        }
    }
    char * message = malloc(100);
    LogInfo("RSET: %d messages undeleted", client->file_cant);
    sprintf(message, "+OK maildrop has %d messages (%d octets)\r\n", client->active_file_cant, client->active_file_size);
    write_to_client(client, message);
    free(message);   
    return WRITE;
}

stm_state_t handleUser(char * arg1, char * arg2, struct selector_key* key) {
    struct Client *client = key->data;

    char * message = "+OK\r\n";
    write_to_client(client, message);

    client->name = malloc(strlen(arg1) + 1);
    memcpy(client->name, arg1, strlen(arg1) + 1);

    return WRITE;
}

stm_state_t handlePass(char * arg1, char * arg2, struct selector_key* key) {
    struct Client *client = key->data;

    if (arg1 == NULL) {
        write_to_client(client, "Expected usage: PASS [password]");
    } else if (client->name == NULL) {
        write_to_client(client, "-ERR No username given.\r\n");
    } else {
        if (user_check_valid(client->name, arg1)) {
            LogInfo("User \"%s\" logged in from %s", client->name, sockaddr_to_human_buffered((struct sockaddr*)&client->addr));
            write_to_client(client, "+OK Logged in.\r\n");
            client->password = malloc(strlen(arg1) + 1);
            memcpy(client->password, arg1, strlen(arg1) + 1);
            client->state = TRANSACTION;
            populate_array(client);
        } else {            
            LogError("Authentication failed for user \"%s\" from %s", client->name, sockaddr_to_human_buffered((struct sockaddr*)&client->addr));
            free(client->name);
            client->name = NULL;
            write_to_client(client, "-ERR [AUTH] Authentication failed.\r\n");
        }
    }
    return WRITE;
}


stm_state_t handleCapaNonAuth(char * arg1, char * arg2, struct selector_key* key) {
    struct Client *client = key->data;
    char * message = "+OK Capability list follows\r\n"
                     "CAPA\r\n"
                     "QUIT\r\n"
                     "USER\r\n"
                     "PASS\r\n"
                     ".\r\n";
    write_to_client(client, message);
    return WRITE;
}

stm_state_t handleCapaAuth(char * arg1, char * arg2, struct selector_key* key) {
    struct Client *client = key->data;
    char * message = "+OK Capability list follows\r\n"
                     "CAPA\r\n"
                     "QUIT\r\n"
                     "STAT\r\n"
                     "LIST\r\n"
                     "RETR\r\n"
                     "DELE\r\n"
                     "NOOP\r\n"
                     "RSET\r\n"
                     ".\r\n";
    write_to_client(client, message);
    return WRITE;
}


CommandInfo nonAuthTable[] = {
    {QUIT, handleQuitNonAuth},
    {USER, handleUser},
    {PASS, handlePass},
    {CAPA, handleCapaNonAuth},
    {0, NULL}
};


CommandInfo authTable[] = {
    {QUIT, handleQuitAuth},
    {STAT, handleStat},
    {LIST, handleList},
    {RETR, handleRetr},
    {DELE, handleDele},
    {NOOP, handleNoop},
    {RSET, handleRset},
    {CAPA, handleCapaAuth},
    {0, NULL}
};

CommandInfo * getTable(client_state state) {
    switch(state) {
        case AUTHORIZATION:
            return nonAuthTable;
        case TRANSACTION:
            return authTable;
        case UPDATE:
            return nonAuthTable;
        default:
            return NULL;
    }
}

stm_state_t executeCommand(pop3cmd_parser * p, struct selector_key* key) {    
    struct Client *client = key->data;
    CommandInfo *commandTable = getTable(client->state);
    if (commandTable == NULL) {
        LogError("Unable to retrieve command table");
        return ERROR_STATE;
    }

    
    int i = 0;
    bool found = false;
    stm_state_t st = ERROR_STATE;
    while(!found && commandTable[i].handler != NULL){
        if(commandTable[i].command == p->state){
            st = (commandTable[i].handler(p->arg1, p->arg2, key));
            found = true;
        }
        i++;
    }
    if (!found){
        write_to_client(client, "-ERR Unknown command.\r\n");
        st = WRITE;
    }
    return st;
   
}
