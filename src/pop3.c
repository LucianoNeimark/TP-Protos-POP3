#include "pop3.h"
#include "command-handler.h"





//Se llama a la funcion que tenga guardado el cliente adentro. El caso default es el de leer comando salvo que se recurra a un retr un list que son multilinea
// y retr requiere manipulacion de la salida.
void pop3Read(struct selector_key *key) {
    printf("Entre al read\n");
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
    printf("Entre al read command\n");
    struct Client *client = key->data;
    size_t limit;      
    uint8_t *buffer2 = buffer_write_ptr(&client->clientBuffer, &limit);
    size_t bytes_read = recv(client->fd, buffer2, limit, 0x4000); // leo lo que me manda el cliente
    buffer_write_adv(&client->clientBuffer, bytes_read); // avanzo el puntero de escritura

    

    if (bytes_read == 0 && !buffer_can_read(&client->clientBuffer)) { 
        printf("El cliente se desconecto\n");
        client->state = CLOSED;
        selector_unregister_fd(key->s, client->fd);
        return ERROR_STATE;
    }

    printf("Antes del parse command In buffer\n");

    stm_state_t res =  parseCommandInBuffer(key);

    printf("Despues del parse command In buffer\n");


    if (res == ERROR_STATE) {
        return ERROR_STATE;
    }

  
    if (buffer_can_read(&client->serverBuffer)) { // Si despues de parsear el comando tengo algo para enviarle hay que escribir
        selector_set_interest_key(key, OP_WRITE);
        printf("Tengo algo para enviarle al cliente\n");
        return res;
    }

    return res;

}

unsigned int pop3WriteCommand(struct selector_key *key) {
    
    struct Client *client = key->data;

    size_t limit;
    ssize_t count;
    uint8_t *buffer = buffer_read_ptr(&client->serverBuffer, &limit);
    count = send(client->fd, buffer, limit, 0x4000);
    buffer_read_adv(&client->serverBuffer, count);

    if (buffer_can_read(&client->serverBuffer)) {  //Si no logre enviar todo vuelvo a entrar a enviar.
        selector_set_interest_key(key, OP_WRITE);
        return WRITE;
    }

    if(buffer_can_read(&client->clientBuffer)){
        printf("todavia quedan cosas por procesar del cliente\n");
        return parseCommandInBuffer(key);
    }
    selector_set_interest_key(key, OP_READ); // Si no quedan cosas por procesar entonces vuelvo a leer del cliente
    return READ;
}



unsigned int pop3ReadFile(struct selector_key* key){
    printf("Entre al read file\n");
    struct Client *client = key->data;

    size_t limit;
    uint8_t *buffer;
    ssize_t count;

    buffer = buffer_write_ptr(&client->serverBuffer, &limit);

    // Read a line from the file
    char *line = read_first_line_file(client->activeFile, client);
    if (line == NULL) {
        // No more lines to read, indicate completion
        client->fileDoneReading = true;
        // enviar /r/n./r/n
        count = snprintf((char *)buffer, limit, ".\r\n");
        buffer_write_adv(&client->serverBuffer, count);


    } else {
        // Append the line to the buffer
        char * byteStuffedLine = byte_stuffing(line);
        count = snprintf((char *)buffer, limit, "%s", byteStuffedLine);
        buffer_write_adv(&client->serverBuffer, count);
       
        free(line); // Free the memory allocated by getline
    }

    selector_set_interest_key(key, OP_WRITE);
    return WRITE_FILE;
}

stm_state_t parseCommandInBuffer(struct selector_key* key) {
//Lo que leo se lo tengo que dar al parser.
    struct Client *client = key->data;
    bool error = false;
    if (buffer_can_read(&client->clientBuffer)){
        printf("Entre porque tengo data del cliente");
        pop3cmd_consume(&client->clientBuffer, client->parser, &error); // le doy al parser lo que lei del cliente
        printf("Volvi de consumir del parser\n");
    } else {
        parser_reset(client->parser);
        selector_set_interest_key(key, OP_READ);
        return READ;
    }

    if (client->parser->finished) {
        printf("El parser termino\n");
        stm_state_t state = executeCommand(client->parser, key);
        printf("El Ejecute el comando entonces el server buffer tiene nueva info.\n");
        parser_reset(client->parser); 
        selector_set_interest_key(key, OP_WRITE);
        return state;
    } 
    
    return ERROR_STATE;
}

unsigned int pop3WriteFile(struct selector_key* key) {
    printf("Entre al write file\n");
    struct Client *client = key->data;
    //Le mando al usuario lo que lei del file
    if (buffer_can_read(&client->serverBuffer)) {
        size_t size = 0;
        char *rbuffer = (char *)buffer_read_ptr(&client->serverBuffer, &size);
        int bytes_read = (int)send(client->fd, rbuffer, size, 0x4000);
        buffer_read_adv(&client->serverBuffer, bytes_read);
        buffer_reset(&client->serverBuffer);
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



void pop3Block(struct selector_key *key) {
    // tenemos que bloquear el socket
    // sock_blocking(((Client *)(key->data))->fd);
}

void pop3Close(struct selector_key *key) {
   close(key->fd);
   printf("Cerrando el socket\n");
//    exit(1);
}

void pop3Error(unsigned int n, struct selector_key *key) {
    selector_unregister_fd(key->s, key->fd);
        close(key->fd);
}