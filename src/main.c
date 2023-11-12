/**
 * main.c - servidor proxy socks concurrente
 *
 * Interpreta los argumentos de línea de comandos, y monta un socket
 * pasivo. Por cada nueva conexión lanza un hilo que procesará de
 * forma bloqueante utilizando el protocolo SOCKS5.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>

#include <sys/socket.h>
#include <netinet/in.h>

#include <unistd.h>
#include "buffer.h"
#include "netutils.h"
#include "tests.h"
#include "pop3cmd.h"
#include "command-handler.h"
#include "args.h"

static bool done = false;

struct POP3args* args;

static void sigterm_handler(const int signal) {
    printf("signal %d, cleaning up and exiting\n",signal);
    done = true;
    exit(0);
}

/**
 * estructura utilizada para transportar datos entre el hilo
 * que acepta sockets y los hilos que procesa cada conexión
 */

struct connection {
  int fd; 
  socklen_t addrlen;
  struct sockaddr_in6 addr;
};


// #include "socks5.h

/**
 * maneja cada conexión entrante
 *
 * @param fd   descriptor de la conexión entrante.
 * @param caddr información de la conexiónentrante.
 */

static void pop3_handle_connection(int fd, const struct sockaddr *caddr) {

    //creo el client
    Client *client = malloc(sizeof(Client));
    client->name = NULL;
    client->fd = fd;
    client->isLogged = false;


    struct buffer serverBuf;
    buffer *serverBuffer = &serverBuf;
    uint8_t serverDirectBuff[1024];
    buffer_init(&serverBuf, N(serverDirectBuff), serverDirectBuff);

    struct buffer clientBuf;
    uint8_t clientDirectBuff[1024];
    buffer_init(&clientBuf, N(clientDirectBuff), clientDirectBuff);

    client->serverBuffer = serverBuffer;
    client->clientBuffer = &clientBuf;


    // enviar saludo
    memcpy(serverDirectBuff, "+OK POP3 server ready\r\n", 23);
    buffer_write_adv(serverBuffer, 23);
    sock_blocking_write(client->fd, serverBuffer);

    // leemos comando
    {
        bool error = false;
        size_t buffsize;
        ssize_t n;
        struct pop3cmd_parser pop3cmd_parser = *pop3cmd_parser_init();

        
        do {
            uint8_t *ptr = buffer_write_ptr(&clientBuf, &buffsize);
            n = recv(client->fd, ptr, buffsize, 0); 
            if(n > 0) {
                buffer_write_adv(&clientBuf, n); 
                /* const enum pop3cmd_state st = */ pop3cmd_consume(&clientBuf, &pop3cmd_parser, &error);
                if(pop3cmd_parser.finished) {
                  executeCommand(&pop3cmd_parser, client);
                  parser_reset(&pop3cmd_parser);
                } else {
                  printf("Not finished\n");
                }

            } else {
                break;
            }   
        } while(true);
        // if(!pop3cmd_is_done(pop3cmd_parser.state, &error)) {
        //     error = true;
        // }
        // pop3cmd_parser_close(&pop3cmd_parser);
        // return error;
    }

    close(client->fd);
    free(client);

}

/** rutina de cada hilo worker */

static void *handle_connection_pthread(void *args) {
  const struct connection *c = args;
  pthread_detach(pthread_self());
  pop3_handle_connection(c->fd, (struct sockaddr *)&c->addr);
  free(args);

  return 0;
}



 /**
 * atiende a los clientes de forma concurrente con I/O bloqueante.
 */
int serve_pop3_concurrent_blocking(const int server) {
  for (;!done;) {
    struct sockaddr_in6 caddr;
    socklen_t caddrlen = sizeof (caddr);
    // Wait for a client to connect
    const int fd = accept(server, (struct sockaddr*)&caddr, &caddrlen);



    if (fd < 0) {
      perror("unable to accept incoming socket");
    } else {
      // TODO(juan): limitar la cantidad de hilos concurrentes
      struct connection* c = malloc(sizeof (struct connection));
      if (c == NULL) {
        // lo trabajamos iterativamente
        pop3_handle_connection(fd, (struct sockaddr*)&caddr);
      } else {
        pthread_t tid;
        c->fd = fd;
        c->addrlen = caddrlen;
        memcpy(&(c->addr), &caddr, caddrlen);
        if (pthread_create(&tid, 0, handle_connection_pthread, c)) {
          free(c);

          // lo trabajamos iterativamente
          pop3_handle_connection(fd, (struct sockaddr*)&caddr);
        }
      }
    }
  }  
  return 0;
}






int
main( int argc,  char **argv) {

    args = (struct POP3args*)malloc(sizeof(struct POP3args));

    parse_args(argc, argv, args);

    // unsigned port = 1110;

    // if(argc == 1) {
    //     // utilizamos el default
    // } else if(argc == 2) {
    //     char *end     = 0;
    //     const long sl = strtol(argv[1], &end, 10);

    //     if (end == argv[1]|| '\0' != *end 
    //        || ((LONG_MIN == sl || LONG_MAX == sl) && ERANGE == errno)
    //        || sl < 0 || sl > USHRT_MAX) {
    //         fprintf(stderr, "port should be an integer: %s\n", argv[1]);
    //         return 1;
    //     }
    //     port = sl;
    // } else {
    //     fprintf(stderr, "Usage: %s <port>\n", argv[0]);
    //     return 1;
    // }
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port        = htons(args->POP3_port);

    const char *err_msg;

    const int server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(server < 0) {
        err_msg = "unable to create socket";
        goto finally;
    }

    fprintf(stdout, "Listening on TCP port %d\n", args->POP3_port);

    // man 7 ip. no importa reportar nada si falla.
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int));

    if(bind(server, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
        err_msg = "unable to bind socket";
        goto finally;
    }

    if (listen(server, 20) < 0) {
        err_msg = "unable to listen";
        goto finally;
    }
    
    // registrar sigterm es útil para terminar el programa normalmente.
    // esto ayuda mucho en herramientas como valgrind.
    signal(SIGTERM, sigterm_handler);
    signal(SIGINT,  sigterm_handler);

    

    err_msg = 0;
    int ret = serve_pop3_concurrent_blocking(server);

    

finally:
    if(err_msg) {
        perror(err_msg);
        ret = 1;
    }
    if(server >= 0) {
        close(server);
    }
    return ret;
}