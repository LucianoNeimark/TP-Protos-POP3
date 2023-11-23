#include "include/users.h"

int user_count = 0; 

struct user users[MAX_USERS];

int user_add(char * string) {
    if (user_count == MAX_USERS) {
        fprintf(stdout, "User limit reached: %d\n", MAX_USERS);
        return -3;
    }

    char *username = strtok(string, ":");
    char *password = strtok(NULL, ":");

    if (username == NULL || password == NULL) {
        fprintf(stdout, "Invalid user format\n");
        return -2;
    }

    if (strlen(username) > MAX_USER_LENGTH) {
        fprintf(stdout, "Username length exceeded: %d\n", MAX_USER_LENGTH);
        return -4;
    }

    if (strlen(password) > MAX_PASS_LENGTH) {
        fprintf(stdout, "Password length exceeded: %d\n", MAX_PASS_LENGTH);
        return -5;        
    }

    if (user_find(username) != -1) {
        fprintf(stdout, "User %s already exists\n", username);
        return -1;
    }

    strncpy(users[user_count].name, username, MAX_USER_LENGTH);
    strncpy(users[user_count].pass, password, MAX_PASS_LENGTH);

    user_count++;
    return 0;
}

int user_remove(char * username) {
    int pos = user_find(username);
    if ( pos == -1 ) {
        fprintf(stdout, "User %s not found\n", username);
        return -1;
    }

    user_count--;

    if (pos != user_count) {
        strncpy(users[pos].name, users[user_count].name, MAX_USER_LENGTH);
        strncpy(users[pos].pass, users[user_count].pass, MAX_PASS_LENGTH);
    }

    return 0;
}

int user_find(char * username) {
    for(int i = 0; i < user_count; i++){        
        if(strcmp(username, users[i].name) == 0){
            return i;
        }
    }

    return -1;
}

bool user_check_valid(char * username, char* password){
    //check if in the users structure the username matches with the password
    for(int i = 0; i < user_count; i++){
        // printf("Username - password: %s - %s\n", username, password);
        
        if(strcmp(username, users[i].name) == 0){
            if(strcmp(password, users[i].pass) == 0){
                return true;
            }
        }
    }

    return false;
}

int get_user_count(void) {
    return user_count;
}

struct user * get_users(void) {
    return users;
}