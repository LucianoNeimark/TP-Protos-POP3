#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include "nuestro-parser.h"
#include <stdio.h>
#include "pop3.h"

typedef void (*CommandHandler)(char * arg1, char * arg2, Client * client);

typedef struct {
    pop3cmd_state command;
    CommandHandler handler;
} CommandInfo;

void executeCommand(pop3cmd_parser * p, Client * client);


#endif