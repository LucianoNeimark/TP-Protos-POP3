#include "command-handler.h"

//FIXME mover a alguna libreria o algo que tenga sentido. Aca suelto es horrible
void write_to_client(Client * client, char * message){
    size_t message_len = strlen(message);
    memcpy(client->serverBuffer->data, message, message_len);
    buffer_write_adv(client->serverBuffer, message_len);
    sock_blocking_write(client->fd, client->serverBuffer);
}


void handleQuit(char * arg1, char * arg, Client * client) {
    printf("QUIT command!\n");
}

void handleStat(char * arg1, char * arg2, Client * client) {
    printf("STAT command!\n");
}

void handleList(char * arg1, char * arg2, Client * client) {

    if (client->files[0].file_id == 0) {
        write_to_client(client, "-ERR Error opening user folder.\r\n");
        return ;
    }

    if(!strlen(arg1)){
        char * message = malloc(100);
        sprintf(message,"+OK %d messages:\r\n", client->file_cant);
        write_to_client(client, message);
        for(int i = 0; client->files[i].file_id != -1 && i < MAX_EMAILS; i++){
            free(message);
            message = malloc(100);
            sprintf(message,"%d %d\r\n", client->files[i].file_id, client->files[i].file_size);
            write_to_client(client, message);
        }
        write_to_client(client, ".\r\n");
    }else{
        char * message = malloc(100);
        for(unsigned int i = 0; i<client->file_cant; i++){
            if(client->files[i].file_id == atoi(arg1)){
                sprintf(message,"+OK %d %d \r\n", client->files[i].file_id, client->files[i].file_size);
                write_to_client(client, message);
                free(message);
                return;
            }
        }
        sprintf(message,"-ERR no such message, only %d messages in maildrop \r\n", client->file_cant);
        write_to_client(client, message);
        free(message);
    }
}

void handleRetr(char * arg1, char * arg2, Client * client) {
    printf("RETR command!\n");
}

void handleDele(char * arg1, char * arg2, Client * client) {
    printf("DELE command!\n");
}

void handleNoop(char * arg1, char * arg2, Client * client) {
    write_to_client(client, "+OK\r\n");
}

void handleRset(char * arg1, char * arg2, Client * client) {
    printf("REST command!\n");
}

void handleTop(char * arg1, char * arg2, Client * client) {
    printf("TOP command!\n");
}

void handleUidl(char * arg1, char * arg2, Client * client) {
    printf("UIDL command!\n");
}

void handleUser(char * arg1, char * arg2, Client * client) {

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

void handlePass(char * arg1, char * arg2, Client * client) {

    if (arg1 == NULL) {
        write_to_client(client, "Expected usage: PASS [password]"); //FIXME RFC ( TODO esto es asi? le tiro pass sin nada e intenta autenticarlo)
    } else if (client->name == NULL) {
        write_to_client(client, "-ERR No username given.\r\n");
    } else {
        // printf("Args->users? %s\n", args->users);
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

void handleApop(char * arg1, char * arg2, Client * client) {
    printf("APOP command!\n");
}

void handleCapaNonAuth(char * arg1, char * arg2, Client * client) {
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

void handleCapaAuth(char * arg1, char * arg2, Client * client) {
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
    {QUIT, handleQuit},
    {USER, handleUser},
    {PASS, handlePass},
    {TOP, handleTop},
    {UIDL, handleUidl},
    {CAPA, handleCapaNonAuth},
    {0, NULL}
};


CommandInfo authTable[] = {
    {QUIT, handleQuit},
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

void executeCommand(pop3cmd_parser * p, Client * client) {
    
    CommandInfo *commandTable = getTable(client->state);
    
    int i = 0;
    while(commandTable[i].handler != NULL){
        if(commandTable[i].command == p->state){
            commandTable[i].handler(p->arg1, p->arg2, client);
            return;
        }
        i++;
    }
    write_to_client(client, "-ERR Unknown command.\r\n");

    // for (size_t i = 0; i < sizeof(commandTable) / sizeof(commandTable[0]); i++) {
    //     if (commandTable[i].command == p->state) {
    //         commandTable[i].handler(p->arg1, p->arg2);
    //         return;
    //     }
       
    // }
    // printf("Unknown command.\n");
}
