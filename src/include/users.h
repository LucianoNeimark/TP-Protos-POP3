#ifndef USERS_H
#define USERS_H

#define MAX_USER_LENGTH 32
#define MAX_PASS_LENGTH 32

#include "constants.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

struct user {
    char name[MAX_USER_LENGTH];
    char pass[MAX_PASS_LENGTH];
};

/* Agrega dinamicamente un usuario valido al servidor */
int user_add(char * string);

/* Borra dinamicamente un usuario valido del servidor */
int user_remove(char * username);


/* Retorna -1 si no existe el usuario en el servidor, un numero mayor o igual a 0 en otro caso */
int user_find(char * username);


/*
    Retorna true si existe un usuario con esa contraseña
    Retorna false si no existe un usuario con esa contraseña
*/
bool user_check_valid(char * username, char* password);

/* Devuelve la cantidad de usuarios del servidor */
int get_user_count(void);

/* Devuelve un arreglo de usuarios del servidor */
struct user * get_users(void);

#endif