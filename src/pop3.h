#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include "buffer.h"


typedef struct Client {
    uint32_t fd;
    struct buffer* serverBuffer;
    struct buffer* clientBuffer;
    char * name;
    char * password;

}Client;