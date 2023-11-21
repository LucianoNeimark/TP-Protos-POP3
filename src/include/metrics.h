#ifndef METRICS_H
#define METRICS_H

#include <string.h>
#include <stddef.h>
#include <stdlib.h>

struct metrics {
    int hist_connections;
    int conc_connections;
    int bytes_transferred;
};

void metrics_init();

void metrics_new_connection();

void metrics_close_connection();

void metrics_send_bytes(int bytes_transferred);

struct metrics * get_metrics();

#endif