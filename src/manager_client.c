#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "include/socket_setup.h"
#include "include/constants.h"

struct manager_client {
    struct addrinfo server_addr;

    uint8_t read_buffer[UDP_BUFFER_SIZE];
    uint8_t write_buffer[UDP_BUFFER_SIZE];
} ;

int main(int argc, char **argv) {

    if (argc != 3) {
        fprintf(stdout, "Usage: %s <server_addr> <server_port>\n", argv[0]);
        return -1;
    }

    struct manager_client client;
    memset(&client.server_addr, 0, sizeof(struct addrinfo));

    int managerSocket = setupClientSocket(argv[1], argv[2], &client.server_addr);

    printf("socket: %d\n", managerSocket);

    while (true) {
        // char buffer[1024];
        // if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        //     if (write(managerSocket, buffer, strlen(buffer)) < 0) {
        //         printf("Write failed\n");
        //     }

        // }

        // client.manager_addr_len = sizeof(client.manager_addr);
        // memset(client.read_buffer, 0, UDP_BUFFER_SIZE);
        // memset(client.write_buffer, 0, UDP_BUFFER_SIZE);

        // ssize_t bytes_read = recvfrom(key->fd, the_manager.manager_buffer, UDP_BUFFER_SIZE, 0,
        //                        (struct sockaddr *) &the_manager.manager_addr, &the_manager.manager_addr_len);

        // if (bytes_read <= 0) {
        //     fprintf(stdout, "Error receiving message from server: %s", strerror(errno));
        //     return ;
        // }

        // if (sendto(key->fd, the_manager.server_buffer, sizeof(the_manager.server_buffer), 0,
        //            (struct sockaddr *) &the_manager.manager_addr, the_manager.manager_addr_len) < 0) {
        //     fprintf(stdout, "Error sending message to server: %s", strerror(errno));
        // }

    }

    return 1;
}