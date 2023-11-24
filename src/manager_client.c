// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "include/socket_setup.h"
#include "include/constants.h"

struct manager_client {
    struct addrinfo server_addr;

    struct sockaddr_storage ret_addr;
    socklen_t ret_addr_len;

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

    int client_socket = setupClientSocket(argv[1], argv[2], &client.server_addr);

    while (true) {
        char buffer[UDP_BUFFER_SIZE];
        char * s = fgets(buffer, sizeof(buffer), stdin);

        if (s == NULL) {
            break;
        }
        
        memset(client.read_buffer, 0, UDP_BUFFER_SIZE);
        memset(client.write_buffer, 0, UDP_BUFFER_SIZE);

        if (sendto(client_socket, s, strlen(s), 0,
                   client.server_addr.ai_addr, client.server_addr.ai_addrlen) < 0) {
            fprintf(stdout, "Error sending message to server: %s\n", strerror(errno));
            return -1;
        }

        ssize_t bytes_read = recvfrom(client_socket, client.read_buffer, UDP_BUFFER_SIZE, 0,
                               (struct sockaddr *) &client.ret_addr, &client.ret_addr_len);

        if (bytes_read <= 0) {
            fprintf(stdout, "Error receiving message from server: %s\n", strerror(errno));
            return -1;
        }

    }

    return 1;
}