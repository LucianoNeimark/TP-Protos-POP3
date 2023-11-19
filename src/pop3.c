#include "pop3.h"
#include "command-handler.h"



void pop3Read(struct selector_key *key) {
    struct Client *client = key->data;

    client_state current_state;

    size_t buffsize;

    // IF COUNT < 0

    size_t n;

    do {

        uint8_t *ptr = buffer_write_ptr(&client->clientBuffer, &buffsize);
        n = recv(client->fd, ptr, buffsize, 0); 
        buffer_write_adv(&client->clientBuffer, n);
        bool error = false;


        if (n>0) {
            while (buffer_can_read(&client->clientBuffer)) {
                pop3cmd_consume(&client->clientBuffer, client->parser, &error);
                if (client->parser->finished) {
                    current_state = executeCommand(client->parser, client);
                    parser_reset(client->parser);
                }
            }
            selector_set_interest_key(key, OP_WRITE);
        }
    } while (current_state!=CLOSED);

    // do {
    //     uint8_t *ptr = buffer_write_ptr(&client->clientBuffer, &buffsize);
    //         // printf("aca sigo\n");
    //         n = recv(client->fd, ptr, buffsize, 0); 
    //         buffer_write_adv(&client->clientBuffer, n);
            
    //         if (n>0){
    //           while(buffer_can_read(&client->clientBuffer)) /*&& buffer_fits(serverBuffer, free_buffer_space))*/ {
    //             printf("%s\n", client->clientBuffer.read);
    //             pop3cmd_consume(&client->clientBuffer, &pop3cmd_parser, &error); // vuelvo con el comando ya calculado.
    //             if (pop3cmd_parser.finished) {
    //               current_state = executeCommand(&pop3cmd_parser, client);
    //               parser_reset(&pop3cmd_parser);
    //             }
    //           }
    //         }


    recv(((Client *)(key->data))->fd, &((Client *)(key->data))->clientBuffer, BUFFER_SIZE, 0); 
    // tenemos que leer lo que tiene el server buffer
    printf("hola read\n");
    // sock_blocking_read(((Client *)(key->data))->fd, ((Client *)(key->data))->serverBuffer);
    printf("chau read\n");
    // buffer_reset(((Client *)(key->data))->serverBuffer);
    // buffer_reset(((Client *)(key->data))->clientBuffer);
    // selector_set_interest_key(key, OP_WRITE);
}

void pop3Write(struct selector_key *key) {
    // printf("holaaaaxaaaaai\n");

    struct Client *client = key->data;
    
    if (client->serverBuffer_size > 0) {
        size_t size = 0;
        char *rbuffer = (char *)buffer_read_ptr(&client->serverBuffer, &size);
        int bytes_read = (int)send(client->fd, rbuffer, size, 0);
        buffer_read_adv(&client->serverBuffer, bytes_read);
        client->serverBuffer_size -= bytes_read;
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
    // tenemos que cerrar el socket
    printf("hola close \n");
    // sock_close(((Client *)(key->data))->fd);
    printf("chau close \n");
}