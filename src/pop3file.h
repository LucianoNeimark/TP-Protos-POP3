#ifndef POP3FILE_H
#define POP3FILE_H
#include "pop3.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "args.h"
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include<sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int populate_array(Client * client);

char* read_file(char *file_name, Client * client);

int remove_file(char * file_name, Client * client);

#endif // POP3FILE_H
