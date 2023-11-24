// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <stdio.h>     /* for printf */
#include <stdlib.h>    /* for exit */
#include <limits.h>    /* LONG_MIN et al */
#include <string.h>    /* memset */
#include <errno.h>
#include <getopt.h>

#include "../include/args.h"

static unsigned short
port(const char *s) {
    char *end     = 0;
    const long sl = strtol(s, &end, 10);

    if (end == s|| '\0' != *end
        || ((LONG_MIN == sl || LONG_MAX == sl) && ERANGE == errno)
        || sl < 0 || sl > USHRT_MAX) {
        LogError("Port should in in the range of 1-65536: %s\n", s);
        exit(1);
    }
    return (unsigned short)sl;
}

static void
directory(char * dest, char * src){
    strcpy(dest,src);
}

static void
version(void) {
    LogInfo(        "POP3 version 0.0\n"
                    "ITBA Protocolos de Comunicación 2023/2 -- Grupo 5\n"
                    "MIT License \n"
                    "Copyright (c) 2023 Grupo 5 \n"
            );
}

static void
usage(const char *progname) {
    LogInfo("Usage: %s [OPTION]...\n"
        "\n"
        "   -h                 Imprime la ayuda y termina.\n"
        "   -p <POP3 port>     Puerto entrante conexiones POP3.\n"
        "   -P <conf port>     Puerto entrante conexiones configuracion\n"
        "   -u <name>:<pass>   Usuario y contraseña de un usuario que puede usar el servidor.\n"
        "   -v                 Imprime información sobre la versión y termina.\n"
        "   -d <path-to-mails> Especifica la carpeta donde se guardarán los usuarios y sus mails.\n"
        "\n",
        progname);
    exit(0);
}

void 
parse_args(const int argc, char **argv, struct POP3args *args) {
    memset(args, 0, sizeof(*args)); // sobre todo para setear en null los punteros de users

    args->POP3_addr = "::";
    args->POP3_port = 1110;

    args->mng_addr   = "::";
    args->mng_port   = 9090;

    args->disectors_enabled = true;

    int c;

    while (true) {
        int option_index = 0;
        static struct option long_options[] = {
            { 0,           0,                 0, 0 }
        };

        c = getopt_long(argc, argv, "hp:P:u:vd:t:f:", long_options, &option_index);
        if (c == -1)
            break;

        switch (c) {
            case 'h':
                usage(argv[0]);
                break;
            case 'p':
                args->POP3_port = port(optarg);
                break;
            case 'P':
                args->mng_port   = port(optarg);
                break;
            case 'u':
                user_add(optarg);
                break;
            case 'v':
                version();
                exit(0);
            case 'd':
                directory(args->directory,optarg);
                break;
            default:
                exit(1);
        }
    }
    if (optind < argc) {
        LogError("Argument not accepted: ");
        while (optind < argc) {
            LogErrorRaw("%s ", argv[optind++]);
        }
        LogErrorRaw("\n");
        exit(1);
    }
}