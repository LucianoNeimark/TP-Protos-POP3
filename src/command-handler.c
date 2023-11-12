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
    printf("LIST command!\n");
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
    memccpy(client->name, arg1, 0, strlen(arg1) + 1);

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
        printf("Expected usage: PASS [password]"); //FIXME RFC
    } else {

        if (check_username(client->name, arg1, args->users)) {
            write_to_client(client, "+OK\r\n");
            client->password = malloc(strlen(arg1) + 1); //Necesario? Quizas con cambiar de estado alcanza
            memccpy(client->password, arg1, 0, strlen(arg1) + 1);
            client->isLogged = true;
        } else {
            write_to_client(client, "-ERR\r\n");
        }
    }
}

void handleApop(char * arg1, char * arg2, Client * client) {
    printf("APOP command!\n");
}

void handleCapa(char * arg1, char * arg2, Client * client) {
    char * message = "+OK Capability list follows\r\n"
                     "QUIT\n"
                     "STAT\n"
                     "LIST\n"
                     "RETR\n"
                     "DELE\n"
                     "NOOP\n"
                     "RSET\n"
                     "TOP\n"
                     "UIDL\n"
                     "USER\n"
                     "PASS\n"
                     "APOP\n"
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
   { QUIT,handleQuit},
    {USER,handleUser},
    {PASS,handlePass},
    {CAPA,handleCapa},
    {0,NULL}
};


CommandInfo authTable[] = {
    {QUIT,handleQuit},
    {STAT,handleStat},
    {LIST,handleList},
    {RETR,handleRetr},
    {DELE,handleDele},
    {NOOP,handleNoop},
    {RSET,handleRset},
    {TOP,handleTop},
    {UIDL,handleUidl},
    {APOP, handleApop},
    {CAPA,handleCapa},
    {0,NULL}
};

void executeCommand(pop3cmd_parser * p, Client * client) {


    
    CommandInfo  *commandTable = client->isLogged ? authTable : nonAuthTable;
    int i = 0;
    while(commandTable[i].handler != NULL){
        if(commandTable[i].command == p->state){
            commandTable[i].handler(p->arg1, p->arg2, client);
            return;
        }
        i++;
    }

    // for (size_t i = 0; i < sizeof(commandTable) / sizeof(commandTable[0]); i++) {
    //     if (commandTable[i].command == p->state) {
    //         commandTable[i].handler(p->arg1, p->arg2);
    //         return;
    //     }
       
    // }
    // printf("Unknown command.\n");
}
