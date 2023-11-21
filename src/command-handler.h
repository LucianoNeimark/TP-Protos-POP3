#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include <stdio.h>
#include <string.h>
#include "netutils.h"
#include "args.h"
#include "pop3file.h"
#include "nuestro-parser.h"

typedef void (*CommandHandler)(char * arg1, char * arg2, Client * client);

typedef struct {
    pop3cmd_state command;
    CommandHandler handler;
} CommandInfo;

client_state executeCommand(pop3cmd_parser * p, Client * client);


#endif