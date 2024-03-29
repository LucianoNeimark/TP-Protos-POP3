// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "include/pop3.h"
#include "include/command_handler.h"

#define COMMAND_LENGTH 4

void user_free(Client * client) {
    if (client == NULL) {
        return;
    }
    if (client->name != NULL) {
        free(client->name);
    }
    if (client->password != NULL) {
        free(client->password);
    }

}

void pop3Read(struct selector_key *key) {
    struct Client *client = key->data;

    if (key->data == NULL) {
        return;
    }
    stm_handler_read(&client->stm, key);

}

void pop3Write(struct selector_key *key) {
    struct Client *client = key->data;
    stm_handler_write(&client->stm, key);
}


unsigned int pop3ReadCommand(struct selector_key* key) {
    struct Client *client = key->data;
    size_t limit;
    uint8_t *buffer2 = buffer_write_ptr(&client->clientBuffer, &limit);
    size_t bytes_read = recv(client->fd, buffer2, limit, MSG_NOSIGNAL);
    buffer_write_adv(&client->clientBuffer, bytes_read);
    stm_state_t state;
    selector_status status;

    if(client->state == CLOSED){
        return CLOSE_STATE;
    }

    if (bytes_read == 0 && !buffer_can_read(&client->clientBuffer)) {
        return CLOSE_STATE;
    }

    state =  parseCommandInBuffer(key);

    if (state == ERROR_STATE) {

        status = SELECTOR_SUCCESS;
        goto error_handling;
    }


    if (buffer_can_read(&client->serverBuffer)) {
        status = selector_set_interest_key(key, OP_WRITE);
        if (status != SELECTOR_SUCCESS) {
            goto error_handling;
        }
        return state;
    }

    return state;

    error_handling:

        if (state == ERROR_STATE)
        status = selector_set_interest_key(key, OP_WRITE);
        if (status != SELECTOR_SUCCESS) {
            selector_set_interest_key(key, OP_NOOP);
        }

        return ERROR_STATE;

}

unsigned int pop3WriteCommand(struct selector_key *key) {
    struct Client *client = key->data;

    size_t limit;
    ssize_t count;
    uint8_t *buffer = buffer_read_ptr(&client->serverBuffer, &limit);
    count = send(client->fd, buffer, limit, MSG_NOSIGNAL);
    metrics_send_bytes(count);
    buffer_read_adv(&client->serverBuffer, count);
    stm_state_t state;

    selector_status status;

    if (buffer_can_read(&client->serverBuffer)) {
        status = selector_set_interest_key(key, OP_WRITE);
        if (status != SELECTOR_SUCCESS) {
            goto error_handling;
        }
        return WRITE;
    }

    if(buffer_can_read(&client->clientBuffer)){
        state = parseCommandInBuffer(key);
        if (state == ERROR_STATE) {
            goto error_handling;
        }
        return state;
    }

    if(client->state == CLOSED){
        return CLOSE_STATE;
    }

    status = selector_set_interest_key(key, OP_READ);

    if (status != SELECTOR_SUCCESS) {
        goto error_handling;
    }

    return READ;

    error_handling:
        selector_set_interest_key(key, OP_NOOP);
        return ERROR_STATE;
}



unsigned int pop3ReadFile(struct selector_key* key){
    struct Client *client = key->data;

    size_t limit;
    uint8_t *buffer;
    ssize_t count;

    buffer = buffer_write_ptr(&client->serverBuffer, &limit);

    char *line = read_first_line_file(client->activeFile, client);
    struct buffer * serverBuffer =&client->serverBuffer;

    selector_status status;

    if (line == NULL) {
        client->fileDoneReading = true;
        count = snprintf((char *)buffer, limit, "%s", "\r\n.\r\n");
        buffer_write_adv(&client->serverBuffer, count);

    } else {
        
        size_t i;
        for(i=0; i < strlen(line) ;i++) {
            if(line[i] == '\n') {
                if (i == 0 || line[i-1] != '\r' ){
                    buffer_write(serverBuffer, '\r');
                    buffer_write(serverBuffer, '\n');
                }else{
                    buffer_write(serverBuffer, '\n');
                }
                client->newLine = true;
            } else if (line[i] == '.') {
                if (client->newLine) {
                    buffer_write(serverBuffer, '.');
                    buffer_write(serverBuffer, '.');
                    client->newLine = false;
                }else{
                    buffer_write(serverBuffer, line[i]);
                    client->newLine = false;
                }

            }
            else {
                buffer_write(serverBuffer, line[i]);
                client->newLine = false;
            }
        }

    }

    status = selector_set_interest_key(key, OP_WRITE);

    if (status != SELECTOR_SUCCESS) {
        goto error_handling;
    }
    if(line != NULL){
        free(line);    
    }
    
    return WRITE_FILE;

    error_handling:
        selector_set_interest_key(key, OP_NOOP);
        return ERROR_STATE;
}

unsigned int pop3WriteFile(struct selector_key* key) {
    struct Client *client = key->data;
    selector_status status;

    if (buffer_can_read(&client->serverBuffer)) {
        size_t size = 0;
        char *rbuffer = (char *)buffer_read_ptr(&client->serverBuffer, &size);
        int bytes_read = (int)send(client->fd, rbuffer, size, MSG_NOSIGNAL);
        metrics_send_bytes(bytes_read);

        buffer_read_adv(&client->serverBuffer, bytes_read);

        if(buffer_can_write(&client->serverBuffer)){
            status = selector_set_interest_key(key, OP_WRITE);
            
            if (status != SELECTOR_SUCCESS) {
                goto error_handling;
            }
            return WRITE_FILE;
        }
    }

    if (!client->fileDoneReading) {
       return pop3ReadFile(key);
    } else {
        status = selector_set_interest_key(key, OP_READ);
        if (status != SELECTOR_SUCCESS) {
            goto error_handling;
        }
        return READ;
    }

    error_handling:
        status = selector_set_interest_key(key, OP_NOOP);
        return ERROR_STATE;
}



stm_state_t parseCommandInBuffer(struct selector_key* key) {
    struct Client *client = key->data;
    bool error = false;
    pop3cmd_state state;
    selector_status status;

    if (buffer_can_read(&client->clientBuffer)){
        state = pop3cmd_consume(&client->clientBuffer, client->parser, &error);
        
    } else {
        parser_reset(client->parser);
        status = selector_set_interest_key(key, OP_READ);
        if (status != SELECTOR_SUCCESS) {
            goto error_handling;
        }
        return READ;
    }

    size_t limit;
    uint8_t *buffer = buffer_read_ptr(&client->clientBuffer, &limit);

    char command[BUFFER_SIZE] = {0};
    memcpy(command, buffer, COMMAND_LENGTH);
    command[COMMAND_LENGTH] = '\0';

    if(state == ERROR){
        LogError("Unable to parse command %s from %s", command, sockaddr_to_human_buffered((struct sockaddr*)&client->addr));
    } else {
        LogInfo("Recieved command %s from %s", command, sockaddr_to_human_buffered((struct sockaddr*)&client->addr));
    }

    if (client->parser->finished) {
        stm_state_t state = executeCommand(client->parser, key);
        parser_reset(client->parser);
        status = selector_set_interest_key(key, OP_WRITE);
        
        if (status != SELECTOR_SUCCESS) {
            goto error_handling;
        }

        return state;
    }
    
    return READ;
    
    error_handling:
        selector_set_interest_key(key, OP_NOOP);
        return ERROR_STATE;
}



unsigned int pop3ReadList(struct selector_key* key) {

    struct Client *client = key->data;
    selector_status status;

    while(client->lastFileList != -1 && client->lastFileList < (int) client->file_cant && client->files[client->lastFileList].to_delete){
            client->lastFileList++;
    }



    if(client->lastFileList == (int) client->file_cant){
        client->lastFileList = -1;
        size_t limitEnd;
        uint8_t *bufferEnd;
        ssize_t countEnd;

        bufferEnd = buffer_write_ptr(&client->serverBuffer, &limitEnd);
        countEnd = (size_t) snprintf((char*)bufferEnd,4,"%s", ".\r\n");
        buffer_write_adv(&client->serverBuffer, countEnd);
        status = selector_set_interest_key(key, OP_WRITE);

        if (status != SELECTOR_SUCCESS) {
            goto error_handling;
        }
        return WRITE;

    
    }

    if(client->lastFileList == -1){
        client->lastFileList = 0;
        size_t limitFirst;
        uint8_t *bufferFirst;
        ssize_t countFirst;

        bufferFirst = buffer_write_ptr(&client->serverBuffer, &limitFirst);
        int cant_files_undel = 0;
        for(int j =0 ; j<(int) client->file_cant; j++){
            if(!client->files[j].to_delete){
                cant_files_undel++;
            }
        }


        countFirst = snprintf((char*)bufferFirst,limitFirst, "+OK %d message%s\r\n", cant_files_undel, cant_files_undel > 1 ? "s" : "");
        buffer_write_adv(&client->serverBuffer, countFirst);
        selector_set_interest_key(key, OP_WRITE);
        return WRITE_LIST;
    }

    size_t limit;
    uint8_t *buffer;
    ssize_t count;

    
    buffer = buffer_write_ptr(&client->serverBuffer, &limit);

    count = snprintf((char*)buffer,limit,"%d %d\r\n", client->files[client->lastFileList].file_id, client->files[client->lastFileList].file_size);
    buffer_write_adv(&client->serverBuffer, count);
    client->lastFileList++;
    status = selector_set_interest_key(key, OP_WRITE);
    if(status != SELECTOR_SUCCESS){
        goto error_handling;
    }
    return WRITE_LIST;

error_handling:
    selector_set_interest_key(key, OP_NOOP);
    return ERROR_STATE;
}


unsigned int pop3WriteList(struct selector_key* key){

    struct Client *client = key->data;

    size_t limit;
    ssize_t count;
    uint8_t *buffer = buffer_read_ptr(&client->serverBuffer, &limit);
    count = send(client->fd, buffer, limit, MSG_NOSIGNAL);
    metrics_send_bytes(count);
    buffer_read_adv(&client->serverBuffer, count);

    selector_status status;


    if (buffer_can_read(&client->serverBuffer)) {  //Si no logre enviar todo vuelvo a entrar a enviar.
       status =  selector_set_interest_key(key, OP_WRITE);
         if(status != SELECTOR_SUCCESS){
              goto error_handling;
            }
        return WRITE_LIST;
    }


    return pop3ReadList(key);
    
    error_handling:
        selector_set_interest_key(key, OP_NOOP);
        return ERROR_STATE;


}



void closeConnection(unsigned int state, struct selector_key *key) {
    struct Client *client = key->data;

    if (key->fd != -1) {
        selector_unregister_fd(key->s, key->fd);
    }
    struct state_machine *stm = &client->stm;
    parser_destroy(client->parser);
    stm_handler_close(stm, key);
    user_free(client);
    metrics_close_connection();
    close(key->fd);
    free(key->data);
}


void pop3Error(unsigned int n, struct selector_key *key) {
    size_t limit;
    struct Client *client = key->data;
    uint8_t * sBuffer = buffer_write_ptr(&client->serverBuffer, &limit);

    if(limit < strlen("-ERR\r\n")){
        selector_set_interest_key(key, OP_WRITE);
        return;
    }

    int count = snprintf((char*)sBuffer, limit, "-ERR\r\n");
    buffer_write_adv(&client->serverBuffer, count);
    selector_set_interest_key(key, OP_WRITE);
    return;

}

void pop3Block(struct selector_key *key){}