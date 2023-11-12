#include <stdio.h>     /* for printf */
#include <stdlib.h>    /* for exit */
#include <limits.h>    /* LONG_MIN et al */
#include <string.h>    /* memset */
#include <errno.h>
#include <getopt.h>

#include "args.h"

static unsigned short
port(const char *s) {
     char *end     = 0;
     const long sl = strtol(s, &end, 10);

     if (end == s|| '\0' != *end
        || ((LONG_MIN == sl || LONG_MAX == sl) && ERANGE == errno)
        || sl < 0 || sl > USHRT_MAX) {
         fprintf(stderr, "port should in in the range of 1-65536: %s\n", s);
         exit(1);
     }
     return (unsigned short)sl;
}

static void
user(char *s, struct users *user) {
    char *p = strchr(s, ':');
    if(p == NULL) {
        fprintf(stderr, "password not found\n");
        exit(1);
    } else {
        *p = 0;
        p++;
        user->name = s;
        user->pass = p;
    }

}


static void
directory(char * dest, char * src){

    printf("src: %s\n",src);
    printf("dest: %s\n",dest);
    strcpy(dest,src);
}

static void
version(void) {
    fprintf(stderr, "POP3 version 0.0\n"
                    "ITBA Protocolos de Comunicación 2023/2 -- Grupo 5\n"
                    "AQUI VA LA LICENCIA\n");
}

static void
usage(const char *progname) {
    fprintf(stderr,
        "Usage: %s [OPTION]...\n"
        "\n"
        "   -h                 Imprime la ayuda y termina.\n"
        "   -l <POP3 addr>     Dirección donde servirá el servidor POP3.\n"
        // "   -L <conf  addr>  Dirección donde servirá el servicio de management.\n"
        "   -p <POP3 port>     Puerto entrante conexiones POP3.\n"
        // "   -P <conf port>   Puerto entrante conexiones configuracion\n"
        "   -u <name>:<pass>   Usuario y contraseña de usuario que puede usar el proxy. Hasta 10.\n"
        "   -v                 Imprime información sobre la versión versión y termina.\n"
        "   -d <path-to-mails> Especifica la carpeta donde se guardarán los usuarios y sus mails.\n"
        "\n",
        progname);
    exit(1);
}

void 
parse_args(const int argc, char **argv, struct POP3args *args) {
    memset(args, 0, sizeof(*args)); // sobre todo para setear en null los punteros de users

    args->POP3_addr = "127.0.0.1";
    args->POP3_port = 1110;

    // args->mng_addr   = "127.0.0.1";
    // args->mng_port   = 8080;

    args->disectors_enabled = true;

    int c;
    int nusers = 0;

    while (true) {
        int option_index = 0;
        static struct option long_options[] = {
            { "doh-ip",    required_argument, 0, 0xD001 },
            { "doh-port",  required_argument, 0, 0xD002 },
            { "doh-host",  required_argument, 0, 0xD003 },
            { "doh-path",  required_argument, 0, 0xD004 },
            { "doh-query", required_argument, 0, 0xD005 },
            { 0,           0,                 0, 0 }
        };

        c = getopt_long(argc, argv, "hl::L::p::o::u:vd:t:f:", long_options, &option_index); //Estaba el long_options
        if (c == -1)
            break;

        switch (c) {
            case 'h':
                usage(argv[0]);
                
                break;
            case 'l':
                args->POP3_addr = optarg;
                break;
            // case 'L':
            //     args->mng_addr = optarg;
            //     break;
            case 'N':
                args->disectors_enabled = false;
                break;
            case 'p':
                args->POP3_port = port(optarg);
                break;
            // case 'P':
            //     args->mng_port   = port(optarg);
            //     break;
            case 'u':
                if(nusers >= MAX_USERS) {
                    fprintf(stderr, "maximum number of command line users reached: %d.\n", MAX_USERS);
                    exit(1);
                } else {
                    printf("user: %s\n", optarg);
                    user(optarg, args->users + nusers);
                    nusers++;
                }
                break;
            case 'v':
                version();
                exit(0);
            case 'd':
                printf("directory: %s\n", optarg);
                directory(args->directory,optarg);
                break;

            default:
                fprintf(stderr, "unknown argument %d.\n", c);
                exit(1);
        }
        args->nusers = nusers;
    }
    if (optind < argc) {
        fprintf(stderr, "argument not accepted: ");
        while (optind < argc) {
            fprintf(stderr, "%s ", argv[optind++]);
        }
        fprintf(stderr, "\n");
        exit(1);
    }
}

bool
check_username(char * username, char* password, struct users *users){
    //check if in the users structure the username matches with the password
    for(size_t i = 0; i < args->nusers; i++){
        printf("username: %s\n", username);
        printf("password: %s\n", password);
        
        if(strcmp(username, users[i].name) == 0){
            if(strcmp(password, users[i].pass) == 0){
                return true;
            }
        }
    }
    return false;
    
}