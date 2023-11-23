#include "pop3.h"
#include "command-handler.h"

#define COMMAND_LENGTH 4



//Se llama a la funcion que tenga guardado el cliente adentro. El caso default es el de leer comando salvo que se recurra a un retr un list que son multilinea
// y retr requiere manipulacion de la salida.
void pop3Read(struct selector_key *key) {
    // printf("Entre al read\n");
    struct Client *client = key->data;
    stm_handler_read(&client->stm, key);
}

void pop3Write(struct selector_key *key) {
    struct Client *client = key->data;
    stm_handler_write(&client->stm, key);
}

char * byte_stuffing(char* line) {
    if (line == NULL) {
        return NULL;
    }
    size_t len = strlen(line);
    if (len == 0) {
        char* empty_result = malloc(3);  
        if (empty_result == NULL) {
            return NULL;  
        }
        strcpy(empty_result, "\r\n");
        return empty_result;
    }



    char* result = malloc(len+4);  
    if (result == NULL) {
        return NULL;  
    }


    size_t resultIndex = 0;
    for (size_t i = 0; i < len; ++i) {
        if(i==0) {
            if (line[i] == '.') {
                result[resultIndex++] = '.';
            }
        }
        result[resultIndex++] = line[i];
    
    }


    strcpy(result + resultIndex, "\r\n");
    return result;
}


unsigned int pop3ReadCommand(struct selector_key* key) {
    // printf("Entre al read command\n");
    struct Client *client = key->data;
    size_t limit;
    uint8_t *buffer2 = buffer_write_ptr(&client->clientBuffer, &limit);
    size_t bytes_read = recv(client->fd, buffer2, limit, 0x4000); // leo lo que me manda el cliente
    buffer_write_adv(&client->clientBuffer, bytes_read); // avanzo el puntero de escritura

    if(client->state == CLOSED){
        return CLOSE_STATE;
    }

    if (bytes_read == 0 && !buffer_can_read(&client->clientBuffer)) {
        client->state = CLOSED;
        return ERROR_STATE;
    }

    stm_state_t res =  parseCommandInBuffer(key);

    

    if (buffer_can_read(&client->serverBuffer)) { // Si despues de parsear el comando tengo algo para enviarle hay que escribir
        selector_set_interest_key(key, OP_WRITE);
        // printf("Tengo algo para enviarle al cliente. res deberia ser write? %s\n", res==WRITE?"y lo es ":"y no lo es");
        return res;
    }

    return res;

}

unsigned int pop3WriteCommand(struct selector_key *key) {
    // printf("Entre al write command\n");
    struct Client *client = key->data;

    size_t limit;
    ssize_t count;
    uint8_t *buffer = buffer_read_ptr(&client->serverBuffer, &limit);
    count = send(client->fd, buffer, limit, 0x4000);
    metrics_send_bytes(count);
    buffer_read_adv(&client->serverBuffer, count);

    if (buffer_can_read(&client->serverBuffer)) {  //Si no logre enviar todo vuelvo a entrar a enviar.
    // printf("Todavia no termine de enviar todo\n");
        selector_set_interest_key(key, OP_WRITE);
        return WRITE;
    }

    if(buffer_can_read(&client->clientBuffer)){
        // printf("todavia quedan cosas por procesar del cliente\n");
        return parseCommandInBuffer(key);
    }

    if(client->state == CLOSED){
        closeConnection(key);
    }

    selector_set_interest_key(key, OP_READ); // Si no quedan cosas por procesar entonces vuelvo a leer del cliente
    return READ;
}



unsigned int pop3ReadFile(struct selector_key* key){
    // printf("Entre al read file\n");
    struct Client *client = key->data;

    size_t limit;
    uint8_t *buffer;
    ssize_t count;

    buffer = buffer_write_ptr(&client->serverBuffer, &limit);

    // Read a line from the file
    char *line = read_first_line_file(client->activeFile, client);
    struct buffer * serverBuffer =&client->serverBuffer;

    if (line == NULL) {
        // No more lines to read, indicate completion
        client->fileDoneReading = true;
        // enviar /r/n./r/n
        count = snprintf((char *)buffer, limit, "%s", "\r\n.\r\n");
        buffer_write_adv(&client->serverBuffer, count);

    } else {
        size_t i;;
        for(i=0; i < strlen(line) ;i++) {
            if(line[i] == '\n') {
                buffer_write(serverBuffer, '\r');
                buffer_write(serverBuffer, '\n');
                client->newLine = true;
            } else if (line[i] == '.') {
                if (client->newLine) {
                    buffer_write(serverBuffer, '.');
                    buffer_write(serverBuffer, '.');
                    client->newLine = false;
                }

            }
            else {
                buffer_write(serverBuffer, line[i]);
                client->newLine = false;
            }
        }

    }

    selector_set_interest_key(key, OP_WRITE);
    return WRITE_FILE;
}

unsigned int pop3WriteFile(struct selector_key* key) {
    struct Client *client = key->data;
    //Le mando al usuario lo que lei del file
    if (buffer_can_read(&client->serverBuffer)) {
        size_t size = 0;
        char *rbuffer = (char *)buffer_read_ptr(&client->serverBuffer, &size);
        // printf("El buffer del server tiene: %s\n", rbuffer);
        int bytes_read = (int)send(client->fd, rbuffer, size, 0x4000);
        metrics_send_bytes(bytes_read);

        buffer_read_adv(&client->serverBuffer, bytes_read);

        if(buffer_can_write(&client->serverBuffer)){
            selector_set_interest_key(key, OP_WRITE);
            return WRITE_FILE;
        }
    }

    if (!client->fileDoneReading) {
       return pop3ReadFile(key);
    } else {
        // client-> write = pop3WriteCommand;
        // client->read = pop3ReadCommand;
        selector_set_interest_key(key, OP_READ);
        return READ;
    }
}



stm_state_t parseCommandInBuffer(struct selector_key* key) {
//Lo que leo se lo tengo que dar al parser.
    struct Client *client = key->data;
    bool error = false;
    pop3cmd_state state;
    if (buffer_can_read(&client->clientBuffer)){
        state = pop3cmd_consume(&client->clientBuffer, client->parser, &error); // le doy al parser lo que lei del cliente
    } else {
        parser_reset(client->parser);
        selector_set_interest_key(key, OP_READ);
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
        // printf("El Ejecute el comando entonces el server buffer tiene nueva info.\n");
        parser_reset(client->parser);
        selector_set_interest_key(key, OP_WRITE);
        // printf("El estado es %s\n", state==WRITE?"write":"read");
        return state;
    }
    // printf("es aca!!!!\n\n");
    return READ;
}



unsigned int pop3ReadList(struct selector_key* key) {
    // printf("Entre al read list\n");
    struct Client *client = key->data;

    if(client->lastFileList == (int) client->file_cant){
        client->lastFileList = -1;
        // escrivbime un \r\rn.
        size_t limitEnd;
        uint8_t *bufferEnd;
        ssize_t countEnd;

        bufferEnd = buffer_write_ptr(&client->serverBuffer, &limitEnd);
        countEnd = (size_t) snprintf((char*)bufferEnd,4,"%s", ".\r\n");
        buffer_write_adv(&client->serverBuffer, countEnd);

        selector_set_interest_key(key, OP_WRITE);
        return WRITE;
    }

    if(client->lastFileList == -1){
        client->lastFileList = 0;
        // escribime en el budder el OK
        size_t limitFirst;
        uint8_t *bufferFirst;
        ssize_t countFirst;

        bufferFirst = buffer_write_ptr(&client->serverBuffer, &limitFirst);
        countFirst = snprintf((char*)bufferFirst,limitFirst, "+OK %d message%s\r\n", client->file_cant, client->file_cant > 1 ? "s" : "");
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
    selector_set_interest_key(key, OP_WRITE);
    return WRITE_LIST;
}


unsigned int pop3WriteList(struct selector_key* key){
    // printf("Entre al write list\n");
    struct Client *client = key->data;

    size_t limit;
    ssize_t count;
    uint8_t *buffer = buffer_read_ptr(&client->serverBuffer, &limit);
    count = send(client->fd, buffer, limit, 0x4000);
    metrics_send_bytes(count);
    buffer_read_adv(&client->serverBuffer, count);
    // printf("el budder es : %s\n", buffer);

    if (buffer_can_read(&client->serverBuffer)) {  //Si no logre enviar todo vuelvo a entrar a enviar.
        selector_set_interest_key(key, OP_WRITE);
        return WRITE_LIST;
    }

    // printf("termine de escribir el list\n");
    return pop3ReadList(key);


}

void pop3Block(struct selector_key *key) {
    // tenemos que bloquear el socket
    // sock_blocking(((Client *)(key->data))->fd);
}

void closeConnection(struct selector_key *key) {
    if (key->fd != -1) {
        selector_unregister_fd(key->s, key->fd);
        close(key->fd);
    }
    LogInfo("Connection closed from %s", sockaddr_to_human_buffered((struct sockaddr*)&((struct Client *) key->data)->addr));
    // printf("Cerrando el socket\n");
}

void pop3Close(struct selector_key *key) {
    // printf("Entre al close\n");
    struct state_machine *stm = &((struct Client *) key->data)->stm;
    stm_handler_close(stm, key);
}

void pop3Error(unsigned int n, struct selector_key *key) {
    // printf("aca me estoy uebdo\n");
    selector_unregister_fd(key->s, key->fd);
        close(key->fd);
}