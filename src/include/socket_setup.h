#ifndef SOCKET_SETUP_H
#define SOCKET_SETUP_H

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

#include "logger.h"

/*
* Crea un socket TCP para el servidor que permite conexiones IPv4 e IPv6
*/
int setupServerSocket(char *addr, int port);


/*
* Crea un socket UDP para el manager que permite conexiones IPv4 e IPv6
*/
int setupManagerSocket(char *addr, int port);


/*
* Establece la conexion UDP entre el cliente y el manager.
*/
int setupClientSocket(const char *address, const char *port, struct addrinfo *server_addr);

#endif