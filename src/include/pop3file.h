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

/* Popula el arreglo de mails del cliente */
int populate_array(Client * client);

/*Lee una parte del mail*/
char* read_first_line_file(char *file_name, Client * client);

/*Funcion interna que usa read_first_line_file para leer parte del mail */
ssize_t private_getline(char **lineptr, FILE *stream);

/* Borra un archivo del directorio */
int remove_file(char * file_name, Client * client);

#endif // POP3FILE_H
