#include "include/socket_setup.h"

int setupManagerSocket(char *addr, int port) {

    const int managerSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    
    if(managerSocket < 0) {
        fprintf(stdout, "Error creating manager socket: %s\n", strerror(errno));
        goto manager_error;
    }

    // Para reusar el socket
    setsockopt(managerSocket, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int));

    struct sockaddr_in manager_addr;
    memset(&manager_addr, 0, sizeof(manager_addr));
    manager_addr.sin_family     = AF_INET;
    manager_addr.sin_port      = htons(port);

    if (inet_pton(AF_INET, addr, &manager_addr.sin_addr) < 0) {
        fprintf(stdout, "Error parsing manager address: %s\n", strerror(errno));
        goto manager_error;
    }

    if(bind(managerSocket, (struct sockaddr*) &manager_addr, sizeof(manager_addr)) < 0) {
        fprintf(stdout, "Error binding manager socket: %s\n", strerror(errno));
        goto manager_error;
    }

    fprintf(stdout, "Manager listening on UDP %s:%d\n", addr, port);

    return managerSocket;

    manager_error:
    if (managerSocket != -1) close(managerSocket);
    return -1;
}


int setupClientSocket(const char *address, const char *port, struct addrinfo *server_addr) {
    struct addrinfo hints = {0};
    hints.ai_family = AF_UNSPEC;      // Allow IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;  // TCP socket

    // Get address information
    int status = getaddrinfo(address, port, &hints, &server_addr);
    if (status != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        return -1;
    }

    // Create a socket
    int clientSocket = socket(server_addr->ai_family, server_addr->ai_socktype, server_addr->ai_protocol);
    if (clientSocket == -1) {
        perror("socket");
        freeaddrinfo(server_addr);
        return -1;
    }

    // Connect to the server
    if (connect(clientSocket, server_addr->ai_addr, server_addr->ai_addrlen) == -1) {
        perror("connect");
        close(clientSocket);
        freeaddrinfo(server_addr);
        return -1;
    }

    return clientSocket;
}