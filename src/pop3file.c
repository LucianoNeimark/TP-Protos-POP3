#include "pop3file.h"
#include <stdio.h>

#define MAX_SIZE_PATH 256

static int get_path(char * path, char *file_name) {
    return snprintf(path, MAX_SIZE_PATH, "%s%s%s", args->directory, "/", file_name);
}

static int get_user_path(char * path, char *file_name, char * user) {
    return snprintf(path, MAX_SIZE_PATH, "%s%s%s", file_name, "/", user);
}

static int get_file_path(char * dest, char * path, char * user, char *file_name) {
    return snprintf(dest, MAX_SIZE_PATH, "%s%s%s%s%s", path, "/", user, "/", file_name);
}

#include <stdio.h>
#include <stdlib.h>

ssize_t custom_getline(char **lineptr, size_t *n, FILE *stream) {
    if (lineptr == NULL || n == NULL || stream == NULL) {
        return -1;
    }

    int c;
    size_t count = 0;

    // Allocate initial buffer or expand existing buffer
    if (*lineptr == NULL || *n == 0) {
        *n = 128;  // Initial buffer size
        *lineptr = malloc(*n);
        if (*lineptr == NULL) {
            return -1;  // Allocation error
        }
    }

    while ((c = fgetc(stream)) != EOF) {
        if (count >= *n - 1) {
            // Expand the buffer if needed
            *n *= 2;
            char *temp = realloc(*lineptr, *n);
            if (temp == NULL) {
                return -1;  // Allocation error
            }
            *lineptr = temp;
        }

        (*lineptr)[count++] = (char)c;

        if (c == '\n') {
            break;  // Stop reading at newline
        }
    }

    if (count == 0) {
        return -1;  // No characters read
    }

    (*lineptr)[count] = '\0';  // Null-terminate the string

    return count;
}


int populate_array(Client * client){
    file * f  = malloc(sizeof (file));
    struct stat file_info;

    char * path = malloc(sizeof(char) * MAX_SIZE_PATH);

    if(get_path(path, client->name) == -1) {
        printf("Error al obtener el path\n");
        return -1;
    }

    DIR * direc = opendir(path);

    if(direc == NULL) {
        printf("Error al abrir el directorio\n");
        return -1;
    }

    int i = 0;
    client->active_file_size = 0;
    struct dirent * file_iter;
    while((file_iter = readdir(direc)) != NULL) {

        if(file_iter->d_type == 8){ //FIXME DESHARDCODEAR EL 8
            char * user_path = malloc(sizeof(char) * MAX_SIZE_PATH);
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

char* read_file(char *file_name, Client * client) {
    char * file_path = malloc(sizeof(char) * MAX_SIZE_PATH);
    get_file_path(file_path, args->directory, client->name, file_name);

    int file_descriptor = open(file_path, O_RDONLY);
    if (file_descriptor == -1) {
        perror("Error opening file");
        return NULL;
    }

    off_t file_size = lseek(file_descriptor, 0, SEEK_END);
    lseek(file_descriptor, 0, SEEK_SET);

    char *file_content = malloc(file_size + 1);  // +1 for null terminator
    if (file_content == NULL) {
        perror("Error allocating memory for file content");
        close(file_descriptor);
        return NULL;
    }

    ssize_t bytes_read = read(file_descriptor, file_content, file_size);
    if (bytes_read == -1) {
        perror("Error reading file");
        close(file_descriptor);
        free(file_content);
        return NULL;
    }

    // Null-terminate the content
    file_content[bytes_read] = '\0';

    // Close the file
    close(file_descriptor);

    return file_content;
}

char* read_first_line_file(char *file_name, Client * client){
     // Check if the file is already open

    char * file_path = malloc(sizeof(char) * MAX_SIZE_PATH);
    get_file_path(file_path, args->directory, client->name, file_name);
    
    if (client->fileState.file == NULL) {
        // Open the file
        client->fileState.file = fopen(file_path, "r");
        if (client->fileState.file == NULL) {
            perror("Error opening file");
            return NULL;
        }
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t bytesRead = custom_getline(&line, &len, client->fileState.file);

    if (bytesRead == -1) {
        // Close the file when we reach the end
        fclose(client->fileState.file);
        client->fileState.file = NULL;
        free(line);
        return NULL;
    }

    // Remove newline character
    if (line[bytesRead - 1] == '\n') {
        line[bytesRead - 1] = '\0';
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