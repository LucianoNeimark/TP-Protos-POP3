// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "include/pop3file.h"
#include <stdio.h>

#define MAX_SIZE_PATH 1024
#define FILE_TYPE 8

static int get_path(char * path, char *file_name) {
    return snprintf(path, MAX_SIZE_PATH, "%s/%s", args->directory, file_name);
}

static int get_user_path(char * path, char *file_name, char * user) {
    return snprintf(path, MAX_SIZE_PATH, "%s/%s", file_name, user);
}

static int get_file_path(char * dest, char * path, char * user, char *file_name) {
    return snprintf(dest, MAX_SIZE_PATH, "%s/%s/%s", path, user, file_name);
}

#include <stdio.h>
#include <stdlib.h>

ssize_t private_getline(char **lineptr, FILE *file) {
    if ( lineptr == NULL || file == NULL) {
        return -1;
    }

    int fd = fileno(file);
    if(fd < 0){
        return -1;
    }
    
    size_t count = 0;
        *lineptr = malloc(BUFFER_SIZE);
        if (*lineptr == NULL) {
            return -1;
        }

    count = read(fd, *lineptr, BUFFER_SIZE-1); 
    (*lineptr)[count] = 0; 
    return count;
}


int populate_array(Client * client){
    file * f  = malloc(sizeof (file));
    if (f == NULL) {
        LogError("Unable to allocate memory for file");
        return -1;
    }

    struct stat file_info;

    char * path = malloc(sizeof(char) * MAX_SIZE_PATH);

    if (path == NULL) {
        LogError("Unable to allocate memory for path");
        free(f);
        return -1;
    }

    if(get_path(path, client->name) == -1) {
        LogError("Unable to get path");
        free(f);
        free(path);
        return -1;
    }

    DIR * direc = opendir(path);

    if(direc == NULL) {
        LogError("Unable to open directory %s", path);
        free(f);
        free(path);
        return -1;
    }

    int i = 0;
    client->active_file_size = 0;
    struct dirent * file_iter;
    while((file_iter = readdir(direc)) != NULL) {

        if(file_iter->d_type == FILE_TYPE){
            char * user_path = malloc(sizeof(char) * MAX_SIZE_PATH);
            if (user_path == NULL) {
                LogError("Unable to allocate memory for user path");
                free(f);
                free(path);
                return -1;
            }

            get_user_path(user_path, path, file_iter->d_name);
            stat(user_path, &file_info);
            strcpy(f->file_name, file_iter->d_name);
            f->file_id = i+1;
            f->file_size = file_info.st_size;
            client->files[i] = *f;
            client->files[i].to_delete = false;
            client->active_file_size += f->file_size;
            i++;
            free(user_path);
      }
    }
    client->files[i].file_id = -1;
    client->file_cant = i;
    client->active_file_cant = i;

    closedir(direc);
    free(path);
    free(f);
    return 1;
}

char* read_first_line_file(char *file_name, Client * client){
     // Verificar que el file no este abierto
    if (client->fileState.file == NULL) {
        char * file_path = malloc(sizeof(char) * MAX_SIZE_PATH);
        get_file_path(file_path, args->directory, client->name, file_name);
        client->fileState.file = fopen(file_path, "r");
        if (client->fileState.file == NULL) {
            LogError("RETR: Unable to open file");
            return NULL;
        }
        LogInfo("RETR: Opened file %s", file_path);
        free(file_path);
    }
    char *line = NULL;
    ssize_t bytesRead = private_getline(&line, client->fileState.file);
    if(bytesRead < 0){
        LogError("RETR: Unable to read line from file");
    }

    if (bytesRead == 0) {
        LogInfo("RETR: Reached end of file. Closing file");
        // Cerramos el archivo cuando llegamos al final.
        fclose(client->fileState.file);
        client->fileState.file = NULL;
        free(line);
        return NULL;
    }
    return line;

}

int remove_file(char * file_name, Client * client) {
    char * file_path = malloc(sizeof(char) * MAX_SIZE_PATH);
    get_file_path(file_path, args->directory, client->name, file_name);

    int ret = remove(file_path);

    free(file_path);

    return ret;
}