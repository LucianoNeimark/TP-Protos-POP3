#include "pop3.h"

void pop3Read(struct selector_key *key) {
    // tenemos que leer lo que tiene el server buffer
    printf("hola read\n");
    // sock_blocking_read(((Client *)(key->data))->fd, ((Client *)(key->data))->serverBuffer);
    printf("chau read\n");
}

void pop3Write(struct selector_key *key) {

    // size_t message_len = strlen(message);
    // memcpy(client->serverBuffer->data, message, message_len);
    // buffer_write_adv(client->serverBuffer, message_len);
    // sock_blocking_write(client->fd, client->serverBuffer);


    // tenemos que imprimir lo que tiene el server buffer
    printf("hola write\n");
    printf("serverBuffer: %s", ((Client *)(key->data))->serverBuffer->data);
    printf("fd: %d", ((Client *)(key->data))->fd);

    sock_blocking_write(((Client *)(key->data))->fd, ((Client *)(key->data))->serverBuffer);

    selector_set_interest_key(key, OP_READ);
    
    printf("chau write\n");
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