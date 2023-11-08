#include "command-handler.h"


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
    printf("NOOP command!\n");
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
    if (arg1 == NULL) {
        printf("Expected usage: USER [username]");
    } else {
        printf("USER command! The user: %s\n", arg1);
        // client->name = malloc(strlen(arg1) + 1);
        // strcpy(client->name, arg1);
    }
}

void handlePass(char * arg1, char * arg2, Client * client) {
    if (arg1 == NULL) {
        printf("Expected usage: PASS [password]");
    } else {
        if (strcmp(arg1, client->password) == 0) {
            printf("Correct password, welcome: %s\n", client->name);
        } else {
            printf("Wrong password!\n");
        }

    }
}

void handleApop(char * arg1, char * arg2, Client * client) {
    printf("APOP command!\n");
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

CommandHandler commandTable[] = {
    handleQuit,
    handleStat,
    handleList,
    handleRetr,
    handleDele,
    handleNoop,
    handleRset,
    handleTop,
    handleUidl,
    handleUser,
    handlePass,
    handleApop
};

void executeCommand(pop3cmd_parser * p, Client * client) {

    if (p->state >= sizeof(commandTable) / sizeof(commandTable[0])) {
        printf("Unknown command.\n");
        return;
    }
    commandTable[p->state](p->arg1, p->arg2, client);


    // for (size_t i = 0; i < sizeof(commandTable) / sizeof(commandTable[0]); i++) {
    //     if (commandTable[i].command == p->state) {
    //         commandTable[i].handler(p->arg1, p->arg2);
    //         return;
    //     }
       
    // }
    // printf("Unknown command.\n");
}
