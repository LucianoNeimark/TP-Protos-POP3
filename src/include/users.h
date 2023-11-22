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

int user_add(char * string);

int user_remove(char * username);

int user_find(char * username);

bool user_check_valid(char * username, char* password);

int get_user_count(void);

struct user * get_users(void);

#endif