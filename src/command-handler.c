#include "command-handler.h"
#include "pop3.h"
//FIXME mover a alguna libreria o algo que tenga sentido. Aca suelto es horrible
void write_to_client(Client * client, char * message){
    size_t limit;
    uint8_t *buffer;
    ssize_t count;

    printf("Sending: %s\n", message);
    buffer = buffer_write_ptr(&client->serverBuffer, &limit);
    count = snprintf((char *) buffer, limit, "%s", message);
    buffer_write_adv(&client->serverBuffer, count);
}

void handleQuitNonAuth(char * arg1, char * arg,  struct selector_key* key) {
    struct Client *client = key->data;

    write_to_client(client, "+OK POP3 server signing off\r\n");
    
    client->state = CLOSED;
}


void handleQuitAuth(char * arg1, char * arg, struct selector_key* key) {
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
        write_to_client(client, "-ERR some deleted messages not removed\r\n");
        return;
    }

    char * message = malloc(100);
    if (client->file_cant == 0) {
        sprintf(message, "+OK POP3 server signing off (maildrop empty)\r\n");
    } else {
        sprintf(message, "+OK POP3 server signing off (%d message%s left)\r\n", client->file_cant, client->file_cant == 1? "":"s");
    }

    write_to_client(client, message);
    free(message);

    client->state = CLOSED;
}

void handleStat(char * arg1, char * arg2, struct selector_key* key) {
    struct Client *client = key->data;
    char * message = malloc(100);
    sprintf(message,"+OK %d %d\r\n", client->active_file_cant, client->active_file_size);
    write_to_client(client, message);
    free(message);
}

void handleList(char * arg1, char * arg2, struct selector_key* key) {
    struct Client *client = key->data;

    if (client->files[0].file_id == 0) {
        write_to_client(client, "-ERR Error opening user folder.\r\n");
        return ;
    }

    if(!strlen(arg1)){
        char * message = malloc(100);
        sprintf(message,"+OK %d messages (%d octets)\r\n", client->active_file_cant, client->active_file_size);
        write_to_client(client, message);
        for(int i = 0; client->files[i].file_id != -1 && i < MAX_EMAILS; i++){
            free(message);
            message = malloc(100);
            if (!client->files[i].to_delete) {
                sprintf(message,"%d %d\r\n", client->files[i].file_id, client->files[i].file_size);
                write_to_client(client, message);
            }
        }
        write_to_client(client, ".\r\n");
    } else {
        char * message = malloc(100);
        for(unsigned int i = 0; i < client->file_cant; i++){
            if(client->files[i].file_id == atoi(arg1)){
                if (!client->files[i].to_delete) {
                    sprintf(message,"+OK %d %d\r\n", client->files[i].file_id, client->files[i].file_size);
                } else {
                    sprintf(message,"-ERR message %d has been marked to delete\r\n", client->files[i].file_id);
                }
                write_to_client(client, message);
                free(message);
                return;
            }
        }
        sprintf(message,"-ERR no such message \r\n");
        write_to_client(client, message);
        free(message);
    }
}

void handleRetr(char * arg1, char * arg2, struct selector_key* key) {
    struct Client *client = key->data;
    for(unsigned int i = 0; i < client->file_cant; i++){
        if(client->files[i].file_id == atoi(arg1) && !client->files[i].to_delete){
            char * message = malloc(100);
            sprintf(message, "+OK %d octets\r\n", client->files[i].file_size);
            write_to_client(client, message);
            client->activeFile = client->files[i].file_name;
            client->read = pop3ReadFile; // Seteo que ahora voy a leer archivo y no del usuario.
            client->write = pop3WriteFile;
            client->fileDoneReading = false;
            // buffer_reset(&client->serverBuffer);
            pop3ReadFile(key);
            free(message);
            return;
        }
    }

    write_to_client(client, "-ERR no such message\r\n");
}

void handleDele(char * arg1, char * arg2, struct selector_key* key) {
    struct Client *client = key->data;
    for(unsigned int i = 0; i < client->file_cant; i++){
        if(client->files[i].file_id == atoi(arg1)){
            char * message = malloc(100);

            if (!client->files[i].to_delete) {
                client->files[i].to_delete = true;
                client->active_file_cant--;
                client->active_file_size -= client->files[i].file_size;
                sprintf(message, "+OK message %d deleted\r\n", client->files[i].file_id);
            } else {
                sprintf(message, "-ERR message %d already deleted\r\n", client->files[i].file_id);
            }

            write_to_client(client, message);
            free(message);
            return;
        }
    }

    write_to_client(client, "-ERR no such message\r\n");
}

void handleNoop(char * arg1, char * arg2, struct selector_key* key) {
    struct Client *client = key->data;
    write_to_client(client, "+OK\r\n");
}

void handleRset(char * arg1, char * arg2, struct selector_key* key) {
    struct Client *client = key->data;
    for(unsigned int i = 0; i < client->file_cant; i++){
        if (client->files[i].to_delete) {
            client->files[i].to_delete = false;
            client->active_file_cant++;
            client->active_file_size += client->files[i].file_size;
        }
    }
    char * message = malloc(100);
    sprintf(message, "+OK maildrop has %d messages (%d octets)\r\n", client->active_file_cant, client->active_file_size);
    write_to_client(client, message);
    free(message);   
}

void handleUser(char * arg1, char * arg2, struct selector_key* key) {
    struct Client *client = key->data;

    // Send +OK message
    char * message = "+OK\r\n";
    write_to_client(client, message);
    //Enter the USER state (So then you expect a PASS command or a QUIT RFC page 12)

    //Save client name into client struct
    client->name = malloc(strlen(arg1) + 1);
    memcpy(client->name, arg1, strlen(arg1) + 1);

    if (arg1 == NULL) {
        // printf("Expected usage: USER [username]");
    } else {
        // printf("USER command! The user: %s\n", arg1);
        // client->name = malloc(strlen(arg1) + 1);
        // strcpy(client->name, arg1);
    }
}

void handlePass(char * arg1, char * arg2, struct selector_key* key) {
    struct Client *client = key->data;

    if (arg1 == NULL) {
        write_to_client(client, "Expected usage: PASS [password]"); //FIXME RFC ( TODO esto es asi? le tiro pass sin nada e intenta autenticarlo)
    } else if (client->name == NULL) {
        write_to_client(client, "-ERR No username given.\r\n");
    } else {
        if (check_username(client->name, arg1, args->users)) {
            write_to_client(client, "+OK Logged in.\r\n");
            client->password = malloc(strlen(arg1) + 1); //Necesario? Quizas con cambiar de estado alcanza
            memcpy(client->password, arg1, strlen(arg1) + 1);
            client->state = TRANSACTION;
            populate_array(client);
        } else {
            free(client->name);
            client->name = NULL;
            write_to_client(client, "-ERR [AUTH] Authentication failed.\r\n");
        }
    }
}


// estos son opcionales!!
void handleTop(char * arg1, char * arg2, struct selector_key* key) {
    printf("TOP command!\n");
}

void handleUidl(char * arg1, char * arg2, struct selector_key* key) {
    printf("UIDL command!\n");
}

void handleApop(char * arg1, char * arg2, struct selector_key* key) {
    printf("APOP command!\n");
}

void handleCapaNonAuth(char * arg1, char * arg2, struct selector_key* key) {
    struct Client *client = key->data;
    char * message = "+OK Capability list follows\r\n"
                     "CAPA\r\n"
                     "QUIT\r\n"
                     "USER\r\n"
                     "PASS\r\n"
                     "TOP\r\n"
                     "UIDL\r\n"
                     ".\r\n";
    write_to_client(client, message);
}

void handleCapaAuth(char * arg1, char * arg2, struct selector_key* key) {
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
                     "TOP\r\n"
                     "UIDL\r\n"
                     "APOP\r\n"
                     ".\r\n";
    write_to_client(client, message);
}

// CommandInfo commandTable[] = {
//     {QUIT, handleQuit},
//     {STAT, handleStat},
//     {LIST, handleList},
//     {RETR, handleRetr},
//     {DELE, handleDele},
//     {NOOP, handleNoop},
//     {RSET, handleRset},
//     {TOP, handleTop},
//     {UIDL, handleUidl},
//     {USER, handleUser},
//     {PASS, handlePass},
//     {APOP, handleApop}
// };

CommandInfo nonAuthTable[] = {
    {QUIT, handleQuitNonAuth},
    {USER, handleUser},
    {PASS, handlePass},
    {TOP, handleTop},
    {UIDL, handleUidl},
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
    {TOP, handleTop},
    {UIDL, handleUidl},
    {APOP, handleApop},
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

client_state executeCommand(pop3cmd_parser * p, struct selector_key* key) {    
    struct Client *client = key->data;
    CommandInfo *commandTable = getTable(client->state);

    
    int i = 0;
    bool found = false;
    while(!found && commandTable[i].handler != NULL){
        if(commandTable[i].command == p->state){
            commandTable[i].handler(p->arg1, p->arg2, key);
            
            found = true;
        }
        i++;
    }
    if (!found) write_to_client(client, "-ERR Unknown command.\r\n");
    return client->state;
   
}
