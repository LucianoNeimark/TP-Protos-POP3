#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include <stdio.h>
#include <string.h>
#include "netutils.h"
#include "args.h"
#include "pop3file.h"
#include "nuestro-parser.h"

typedef stm_state_t (*CommandHandler)(char * arg1, char * arg2, struct selector_key* key);

typedef struct {
    pop3cmd_state command;
    CommandHandler handler;
} CommandInfo;

stm_state_t executeCommand(pop3cmd_parser * p, struct selector_key* key);


#endif