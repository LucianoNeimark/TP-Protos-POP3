#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include "nuestro-parser.h"
#include <stdio.h>
#include "netutils.h"
#include <string.h>
#include "args.h"
#include "pop3file.h"

typedef void (*CommandHandler)(char * arg1, char * arg2, Client * client);

typedef struct {
    pop3cmd_state command;
    CommandHandler handler;
} CommandInfo;

client_state executeCommand(pop3cmd_parser * p, Client * client);


#endif