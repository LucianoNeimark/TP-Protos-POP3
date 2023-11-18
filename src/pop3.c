#include "pop3.h"

void pop3Read(struct selector_key *key) {
    recv(((Client *)(key->data))->fd, &((Client *)(key->data))->clientBuffer, BUFFER_SIZE, 0); 
    // tenemos que leer lo que tiene el server buffer
    printf("hola read\n");
    // sock_blocking_read(((Client *)(key->data))->fd, ((Client *)(key->data))->serverBuffer);
    printf("chau read\n");
    // buffer_reset(((Client *)(key->data))->serverBuffer);
    // buffer_reset(((Client *)(key->data))->clientBuffer);
    selector_set_interest_key(key, OP_WRITE);
}

void pop3Write(struct selector_key *key) {
    printf("holaaaaaaaaai\n");

    struct Client *client = key->data;

    // size_t limit;
    // buffer_read_ptr(&client->serverBuffer, &limit);

    // // printf("holaaa\n");

    // buffer_read_adv(&client->serverBuffer, 23);

    
    if (client->serverBuffer_size > 0) {
        size_t size = 0;
        char *rbuffer = (char *)buffer_read_ptr(&client->serverBuffer, &size);
        int bytes_read = (int)send(client->fd, rbuffer, size, 0);
        buffer_read_adv(&client->serverBuffer, bytes_read);
        client->serverBuffer_size -= bytes_read;
    }

    

    // if (buffer_can_read(&client->serverBuffer)) {
    //     selector_set_interest_key(key, OP_WRITE);
    // }

    // size_t message_len = strlen("hola");

    // memcpy(&client->serverBuffer.data, "hola", 5);
    // buffer_write_adv(&client->serverBuffer, 5);

    //seteo intencion. Si no se puede escribir, se va a volver a llamar a esta funcion

    // sock_blocking_write(client->fd, &client->serverBuffer);
    


    // size_t message_len = strlen(message);
    // memcpy(client->serverBuffer->data, message, message_len);
    // buffer_write_adv(client->serverBuffer, message_len);
    // sock_blocking_write(client->fd, client->serverBuffer);


    // tenemos que imprimir lo que tiene el server buffer
    // printf("hola write\n");
    // printf("serverBuffer: %s", ((Client *)(key->data))->serverBuffer->data);
    // printf("fd: %d", ((Client *)(key->data))->fd);

    // sock_blocking_write(((Client *)(key->data))->fd, ((Client *)(key->data))->serverBuffer);

    
    // printf("chau write\n");
    selector_set_interest_key(key, OP_READ);
    
}

void pop3Block(struct selector_key *key) {
    // tenemos que bloquear el socket
    printf("hola block\n");
    // sock_blocking(((Client *)(key->data))->fd);
    printf("chau block\n");
}

void pop3Close(struct selector_key *key) {
    // tenemos que cerrar el socket
    printf("hola close \n");
    // sock_close(((Client *)(key->data))->fd);
    printf("chau close \n");
}