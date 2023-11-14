#ifndef POP3FILE_H
#define POP3FILE_H
#include "pop3.h"

int populate_array(Client * client);

char* read_file(char *file_name, Client * client);

#endif // POP3FILE_H
