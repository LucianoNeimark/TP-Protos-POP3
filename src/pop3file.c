#include "pop3file.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "args.h"
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include<sys/types.h>
#include <sys/stat.h>


#define MAX_SIZE_PATH 256



static int get_path(char * path, char *file_name) {
    return snprintf(path, MAX_SIZE_PATH, "%s%s%s", args->directory, "/", file_name);
}

static int get_user_path(char * path, char *file_name, char * user) {
    return snprintf(path, MAX_SIZE_PATH, "%s%s%s", file_name, "/", user);
}


int populate_array(Client * client){
    file * f  = malloc(sizeof (file));
    struct stat file_info;

    char * path = malloc(sizeof(char) * MAX_SIZE_PATH);

    if(get_path(path,client->name) == -1) {
        printf("Error al obtener el path\n");
        return -1;
    }

    DIR * direc = opendir(path);

    if(direc == NULL) {
        printf("Error al abrir el directorio\n");
        return -1;
    }

    int i = 0;
    struct dirent * file_iter;
    while((file_iter = readdir(direc)) != NULL) {

        if(file_iter->d_type == 8){ //FIXME DESHARDCODEAR EL 8
            char * user_path = malloc(sizeof(char) * MAX_SIZE_PATH);
            get_user_path(user_path,path,file_iter->d_name);
            stat(user_path, &file_info);
            strcpy(f->file_name ,file_iter->d_name);
            f->file_id = i+1;
            f->file_size = file_info.st_size;
            client->files[i] = *f;
            i++;
            free(user_path);
      }
    }
    client->files[i].file_id = -1;

    closedir(direc);
    free(path);
    free(f);
    return 1;
}
