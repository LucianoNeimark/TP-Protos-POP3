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


int setupManagerSocket(char *addr, int port);

int setupClientSocket(const char *address, const char *port, struct addrinfo *server_addr);

#endif