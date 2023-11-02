/**
 * request.c -- parser del hello de SOCKS5
 */
#include <stdio.h>
#include <stdlib.h>

#include "pop3cmd.h"

extern void 
pop3cmd_parser_init(struct pop3cmd_parser *p) {
    p->state     = pop3cmd_version;
    p->remaining = 0;
}

extern enum pop3cmd_state
pop3cmd_parser_feed(struct pop3cmd_parser *p, const uint8_t b) {
    switch(p->state) {
        case pop3cmd_version:
            if(0x05 == b) {
                p->state = pop3cmd_nmethods;
            } else {
                p->state = pop3cmd_error_unsupported_version;
            }
            break;

        case pop3cmd_nmethods:
            p->remaining = b;
            p->state     = pop3cmd_methods;

            if(p->remaining <= 0) {
                p->state = pop3cmd_done;
            }

            break;

        case pop3cmd_methods:
            if(NULL != p->on_authentication_method) {
                p->on_authentication_method(p, b);
            }
            p->remaining--;
            if(p->remaining <= 0) {
                p->state = pop3cmd_done;
            }
            break;
        case pop3cmd_done:
        case pop3cmd_error_unsupported_version:
            // nada que hacer, nos quedamos en este estado
            break;
        default:
            fprintf(stderr, "unknown state %d\n", p->state);
            abort();
    }

    return p->state;
}

extern bool 
pop3cmd_is_done(const enum pop3cmd_state state, bool *errored) {
    bool ret;
    switch (state) {
        case pop3cmd_error_unsupported_version:
            if (0 != errored) {
                *errored = true;
            }
            /* no break */
        case pop3cmd_done:
            ret = true;
            break;
        default:
            ret = false;
            break;
    }
   return ret;
}

extern const char *
pop3cmd_error(const struct pop3cmd_parser *p) {
    char *ret;
    switch (p->state) {
        case pop3cmd_error_unsupported_version:
            ret = "unsupported version";
            break;
        default:
            ret = "";
            break;
    }
    return ret;
}

extern void 
pop3cmd_parser_close(struct pop3cmd_parser *p) {
    /* no hay nada que liberar */
}

extern enum pop3cmd_state
pop3cmd_consume(buffer *b, struct pop3cmd_parser *p, bool *errored) {
    enum pop3cmd_state st = p->state;

    while(buffer_can_read(b)) {
        const uint8_t c = buffer_read(b);
        st = pop3cmd_parser_feed(p, c);
        if (pop3cmd_is_done(st, errored)) {
            break;
        }
    }
    return st;
}

extern int
pop3cmd_marshall(buffer *b, const uint8_t method) {
    size_t n;
    uint8_t *buff = buffer_write_ptr(b, &n);
    if(n < 2) {
        return -1;
    }
    buff[0] = 0x05;
    buff[1] = method;
    buffer_write_adv(b, 2);
    return 2;
}

