#include "pop3.h"
#include "command-handler.h"



void pop3Read(struct selector_key *key) {

    struct Client *client = key->data;

    //Primero tengo que leer lo que escrinio el usuario y hacerlo "persistir" en su buffer interno asi cuando se vuelve a llamar sigue todo ok.

    size_t bytes_read = recv(client->fd, client->clientBuffer_data, BUFFER_SIZE, 0); // leo lo que me manda el cliente
    buffer_write_adv(&client->clientBuffer, bytes_read); // avanzo el puntero de escritura

    //Lo que leo se lo tengo que dar al parser.
    bool error = false;
    pop3cmd_consume(&client->clientBuffer, client->parser, &error); // le doy al parser lo que lei del cliente
    if (client->parser->finished) {
        executeCommand(client->parser, client);
        parser_reset(client->parser);
    }

    if (buffer_can_read(&client->serverBuffer)) {
        selector_set_interest_key(key, OP_WRITE);
    }

    // IF COUNT < 0

    // do {

    //     uint8_t *ptr = buffer_write_ptr(&client->clientBuffer, &buffsize);
    //     n = recv(client->fd, ptr, buffsize, 0); 
    //     buffer_write_adv(&client->clientBuffer, n);
    //     bool error = false;


    //     if (n>0) {
    //         while (buffer_can_read(&client->clientBuffer)) {
    //             pop3cmd_consume(&client->clientBuffer, client->parser, &error);
    //             if (client->parser->finished) {
    //                 current_state = executeCommand(client->parser, client);
    //                 parser_reset(client->parser);
    //             }
    //         }
    //         selector_set_interest_key(key, OP_WRITE);
    //     }
    // } while (current_state!=CLOSED);


}

void pop3Write(struct selector_key *key) {
    printf("holaaaaxaaaaai\n");

    struct Client *client = key->data;
    
    if (buffer_can_read(&client->serverBuffer)) {
        size_t size = 0;
        char *rbuffer = (char *)buffer_read_ptr(&client->serverBuffer, &size);
        int bytes_read = (int)send(client->fd, rbuffer, size, 0);
        buffer_read_adv(&client->serverBuffer, bytes_read);
        // client->serverBuffer_size -= bytes_read;
    }

    if (client->state == CLOSED){
        selector_unregister_fd(key->s, client->fd);
    }

    
    selector_set_interest_key(key, OP_READ);
    
}

void pop3Block(struct selector_key *key) {
    // tenemos que bloquear el socket
    printf("hola block\n");
    // sock_blocking(((Client *)(key->data))->fd);
    printf("chau block\n");
}

void pop3Close(struct selector_key *key) {
    close(key->fd);
}