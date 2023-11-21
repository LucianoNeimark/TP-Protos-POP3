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
#include <errno.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <unistd.h>
#include "buffer.h"
#include "netutils.h"
#include "tests.h"
#include "pop3cmd.h"
#include "command-handler.h"
#include "include/manager_handler.h"
#include "include/metrics.h"  // temporal (hasta que corramos pop3_handle_connection)
#include "args.h"
#include "selector.h"

#define SELECTOR_SIZE 1024

static bool done = false;

static int setupManagerSocket(char *addr, int port);

//llamamos a nuestros metodos de leer y escribir para que los use el selector cuando le toca a cada cliente.
// Donde escribiamos ahora copiamos al buffer y seteamos la intencion
// Cuando me toca se llama a estos metodos de abajo dependiendo la intencion que haya seteado antes.
static const fd_handler pop3Handlers = {
    .handle_read = pop3Read,
    .handle_write = pop3Write,
    .handle_block = pop3Block,
    .handle_close = pop3Close,
};

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
 * @param caddr información de la conexión entrante.
 */

static void pop3_handle_connection(/*int fd, const struct sockaddr *caddr*/ struct selector_key *key) {

  printf("handlecone\n");

  struct sockaddr_storage client_addr;
  socklen_t client_addr_len = sizeof(client_addr);

  int client_fd = accept(key->fd, (struct sockaddr *)&client_addr, &client_addr_len);
  printf("client_fd: %d\n key_fd: %d\n", client_fd, key->fd);

  if (client_fd == -1) {
    printf("%s\n", "Error al aceptar cliente");
  }

  //creo el client
  Client *client = calloc(1, (sizeof(struct Client)));

  if (client == NULL) {printf("%s\n", "Error al crear cliente");}

  client->name = NULL;
  client->fd = client_fd;
  client->state = AUTHORIZATION;
  client->parser = pop3cmd_parser_init();

  buffer_init(&client->serverBuffer, BUFFER_SIZE, client->serverBuffer_data);

  buffer_init(&client->clientBuffer, BUFFER_SIZE, client->clientBuffer_data);

  // client->serverBuffer = serverBuffer;
  // client->clientBuffer = &clientBuf;
   char *s = "+OK POP3 server ready\r\n";

    // Since the output buffer is empty, we can write directly to it
    for (int i=0; s[i]; i++) {
        buffer_write(&client->serverBuffer, s[i]);
    }

    client->serverBuffer_size = strlen(s);

  // memcpy(&client->serverBuffer.data, "hola", 5);
  // buffer_write_adv(&client->serverBuffer, 5);
  // sock_blocking_write(client_fd, &client->serverBuffer);

  if (selector_fd_set_nio(client_fd) == -1) {
    // goto; //TODO
    printf("%s\n", "Error al setear nio\n");
  }

  selector_status status = selector_register(key->s, client_fd, &pop3Handlers, OP_WRITE, client);
  if (status != SELECTOR_SUCCESS) {
    // goto handle_error;
  }

  metrics_new_connection();
   
    // holaquieroimprimir
    // sock_blocking_write(client->fd, serverBuffer); ANTES 


return;
  

    // // leemos comando
    // {
    //     bool error = false;
    //     size_t buffsize;
    //     ssize_t n;
    //     struct pop3cmd_parser pop3cmd_parser = *pop3cmd_parser_init();
    //     client_state current_state;
        
    //     do {
    //         uint8_t *ptr = buffer_write_ptr(&client->clientBuffer, &buffsize);
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
            



    //         // if(n > 0) {
    //         //     buffer_write_adv(&clientBuf, n); 
    //         //     /* const enum pop3cmd_state st = */ pop3cmd_consume(&clientBuf, &pop3cmd_parser, &error);
    //         //     if(pop3cmd_parser.finished) {
    //         //       current_state = executeCommand(&pop3cmd_parser, client);
    //         //       parser_reset(&pop3cmd_parser);
    //         //     } else {
    //         //       printf("Not finished\n");
    //         //     }

    //         // } else {
    //         //     break;
    //         // }   
    //     } while(current_state != CLOSED);
    //     printf("Sali\n");
    //     // if(!pop3cmd_is_done(pop3cmd_parser.state, &error)) {
    //     //     error = true;
    //     // }
    //     // pop3cmd_parser_close(&pop3cmd_parser);
    //     // return error;
    // }

    close(client->fd);
    free(client);

}

/** rutina de cada hilo worker */

// static void *handle_connection_pthread(void *args) {
//   const struct connection *c = args;
//   pthread_detach(pthread_self());
//   pop3_handle_connection(c->fd, (struct sockaddr *)&c->addr);
//   free(args);

//   return 0;
// }



 /**
 * atiende a los clientes de forma concurrente con I/O bloqueante.
 */
// int serve_pop3_concurrent_blocking(const int server) {
//   for (;!done;) {
//     struct sockaddr_in6 caddr;
//     socklen_t caddrlen = sizeof (caddr);
//     // Wait for a client to connect
//     const int fd = accept(server, (struct sockaddr*)&caddr, &caddrlen);



//     if (fd < 0) {
//       perror("unable to accept incoming socket");
//     } else {
//       // TODO(juan): limitar la cantidad de hilos concurrentes
//       struct connection* c = malloc(sizeof (struct connection));
//       if (c == NULL) {
//         // lo trabajamos iterativamente
//         pop3_handle_connection(fd, (struct sockaddr*)&caddr);
//       } else {
//         pthread_t tid;
//         c->fd = fd;
//         c->addrlen = caddrlen;
//         memcpy(&(c->addr), &caddr, caddrlen);
//         if (pthread_create(&tid, 0, handle_connection_pthread, c)) {
//           free(c);

//           // lo trabajamos iterativamente
//           pop3_handle_connection(fd, (struct sockaddr*)&caddr);
//         }
//       }
//     }
//   }  
//   return 0;
// }

int
main( int argc,  char **argv) {

  int ret = -1;

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

    const char *err_msg = 0;

    const int server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    if(server < 0) {
        err_msg = "unable to create socket";
        goto finally;
    }

    const struct selector_init init = {
        .signal = SIGALRM,
        .select_timeout = {
            .tv_sec  = 10,
            .tv_nsec = 0,
        },
    };

    fd_selector selector = NULL;
    selector_status selectStatus = selector_init(&init);
    if (selectStatus != SELECTOR_SUCCESS) goto finally;

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
    
    fprintf(stdout, "Listening on TCP %s:%d\n", args->POP3_addr, args->POP3_port);

    int managerSocket = setupManagerSocket(args->mng_addr, args->mng_port);
    if (managerSocket == -1) goto finally;

    // registrar sigterm es útil para terminar el programa normalmente.
    // esto ayuda mucho en herramientas como valgrind.
    signal(SIGTERM, sigterm_handler);
    signal(SIGINT,  sigterm_handler);

    // Trabajamos aca
    if ( server != -1 && selector_fd_set_nio(server) == -1) {
      goto finally;
    }
    
    selector = selector_new(SELECTOR_SIZE);
    if (selector == NULL) goto finally;

    const fd_handler passiveHandler = {
            .handle_read = pop3_handle_connection, //TODO Es el saludo, registra el cliente y sus cosas en el selector.
            .handle_write = NULL,
            .handle_close = NULL,
            .handle_block = NULL,
    };

    selectStatus = selector_register(selector, server, &passiveHandler, OP_READ, NULL);
    if(selectStatus != SELECTOR_SUCCESS) goto finally;

    const fd_handler managerHandler = {
            .handle_read = manager_handle_connection, 
            .handle_write = NULL, 
            .handle_close = NULL, 
            .handle_block = NULL
    };

    selectStatus = selector_register(selector, managerSocket, &managerHandler, OP_READ, NULL);
    if(selectStatus != SELECTOR_SUCCESS) goto finally;

    while (!done) {
        selectStatus = selector_select(selector);
        if (selectStatus != SELECTOR_SUCCESS) goto finally;
    }



    err_msg = 0;
    ret = 0;

    

  finally:
    // TODO: handle selectStatus cases

    if(err_msg) {
        perror(err_msg);
        ret = 1;
    }
    if(server >= 0) {
        close(server);
    }
    return ret;
}

static int setupManagerSocket(char *addr, int port) {

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

    if (inet_pton(AF_INET, addr, &manager_addr.sin_addr) <= 0) {
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