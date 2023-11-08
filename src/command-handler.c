#include "command-handler.h"


void handleQuit(char * arg1, char * arg2) {
    printf("QUIT command!\n");
}

void handleStat(char * arg1, char * arg2) {
    printf("STAT command!\n");
}

void handleList(char * arg1, char * arg2) {
    printf("LIST command!\n");
}

void handleRetr(char * arg1, char * arg2) {
    printf("RETR command!\n");
}

void handleDele(char * arg1, char * arg2) {
    printf("DELE command!\n");
}

void handleNoop(char * arg1, char * arg2) {
    printf("NOOP command!\n");
}

void handleRset(char * arg1, char * arg2) {
    printf("REST command!\n");
}

void handleTop(char * arg1, char * arg2) {
    printf("TOP command!\n");
}

void handleUidl(char * arg1, char * arg2) {
    printf("UIDL command!\n");
}

void handleUser(char * arg1, char * arg2) {
    if (arg1 == NULL) {
        printf("Expected usage: USER [username]");
    } else {
        printf("USER command! The user: %s\n", arg1);
    }
}

void handlePass(char * arg1, char * arg2) {
    if (arg1 == NULL) {
        printf("Expected usage: PASS [password]");
    } else {
        printf("PASS command! The password: %s\n", arg1);
    }
}

void handleApop(char * arg1, char * arg2) {
    printf("APOP command!\n");
}



CommandInfo commandTable[] = {
    {QUIT, handleQuit},
    {STAT, handleStat},
    {LIST, handleList},
    {RETR, handleRetr},
    {DELE, handleDele},
    {NOOP, handleNoop},
    {RSET, handleRset},
    {TOP, handleTop},
    {UIDL, handleUidl},
    {USER, handleUser},
    {PASS, handlePass},
    {APOP, handleApop}
};

void executeCommand(pop3cmd_parser * p) {
    for (size_t i = 0; i < sizeof(commandTable) / sizeof(commandTable[0]); i++) {
        if (commandTable[i].command == p->state) {
            commandTable[i].handler(p->arg1, p->arg2);
            return;
        }
    }
    printf("Unknown command.\n");
}
