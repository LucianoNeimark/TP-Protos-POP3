// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "include/socket_setup.h"

int setupServerSocket(char *addr, int port) {

    struct sockaddr_in6 serveradr;
    const int server = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);

    if(server < 0) {
        LogError("Error creating server socket: %s", strerror(errno));
        goto socket_error;
    }


    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

    memset(&serveradr, 0, sizeof(serveradr));
    serveradr.sin6_port = htons(port);
    serveradr.sin6_family = AF_INET6;
    serveradr.sin6_addr = in6addr_any;

    if (inet_pton(AF_INET, addr, &serveradr.sin6_addr)< 0) {
        LogError("Invalid address");
        goto socket_error;
    }

    if (bind(server, (struct sockaddr *)&serveradr, sizeof(serveradr)) < 0 )
    {
        LogError("Unable to bind socket");
        goto socket_error;

    }

    if (listen(server, 20) < 0)
    {
        LogError("Unable to listen");
        goto socket_error;

    }

    LogInfo("Listening on TCP %s:%d", addr, port);

    return server;

    socket_error:
    if (server != -1) close(server);
    return -1;
}

int setupManagerSocket(char *addr, int port) {

    const int managerSocket = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
    
    if(managerSocket < 0) {
        LogError("Error creating manager socket: %s", strerror(errno));
        goto socket_error;
    }

    // Para reusar el socket
    setsockopt(managerSocket, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int));

    struct sockaddr_in6 manager_addr;
    memset(&manager_addr, 0, sizeof(manager_addr));
    manager_addr.sin6_port = htons(port);
    manager_addr.sin6_family = AF_INET6;
    manager_addr.sin6_addr = in6addr_any;


    if (inet_pton(AF_INET, addr, &manager_addr.sin6_addr) < 0) {
        LogError("Error parsing manager address: %s", strerror(errno));
        goto socket_error;
    }

    if(bind(managerSocket, (struct sockaddr*) &manager_addr, sizeof(manager_addr)) < 0) {
        LogError("Error binding manager socket: %s", strerror(errno));
        goto socket_error;
    }

    LogInfo("Manager listening on UDP %s:%d", addr, port);

    return managerSocket;

    socket_error:
    if (managerSocket != -1) close(managerSocket);
    return -1;
}


int setupClientSocket(const char *address, const char *port, struct addrinfo *server_addr) {
    struct addrinfo *aux_addr;
    struct addrinfo hints = {0};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    int status = getaddrinfo(address, port, &hints, &aux_addr);
    if (status != 0) {
        LogError("getaddrinfo error: %s", gai_strerror(status));
        return -1;
    }

    int client_socket = socket(aux_addr->ai_family, aux_addr->ai_socktype, aux_addr->ai_protocol);
    if (client_socket == -1) {
        LogError("Error establishing client socket: %s", strerror(errno));
        freeaddrinfo(aux_addr);
        return -1;
    }

    if (connect(client_socket, aux_addr->ai_addr, aux_addr->ai_addrlen) == -1) {
        LogError("Error connecting to client socket: %s", strerror(errno));
        close(client_socket);
        freeaddrinfo(aux_addr);
        return -1;
    }
    
    memcpy(server_addr, aux_addr, sizeof(struct addrinfo));
    freeaddrinfo(aux_addr);
    return client_socket;
}
