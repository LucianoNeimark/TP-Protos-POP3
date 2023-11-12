#include <stdio.h>
#include <stdlib.h>

#include "pop3cmd.h"

extern pop3cmd_parser *  
pop3cmd_parser_init(void) {
    return parser_init();
}

extern void 
pop3cmd_parser_close(struct pop3cmd_parser *p) {
    parser_destroy(p);
}

extern enum pop3cmd_state
pop3cmd_consume(buffer *b, struct pop3cmd_parser *p, bool *errored) {
    enum pop3cmd_state st = p->state;

    while(buffer_can_read(b)) {
        const uint8_t c = buffer_read(b);
        st = parser_feed(p, c);
        if (p->finished) {
            break;
        }
    }
    return st;
}