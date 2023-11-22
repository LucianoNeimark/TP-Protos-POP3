#include "include/metrics.h"

struct metrics server_metrics;

void metrics_init() {
    memset(&server_metrics, 0, sizeof(struct metrics));

    server_metrics.hist_connections = 0;
    server_metrics.conc_connections = 0;
    server_metrics.bytes_transferred = 0;
}

void metrics_new_connection() {
    server_metrics.hist_connections++;
    server_metrics.conc_connections++;
}

void metrics_close_connection() {
    server_metrics.conc_connections--;
}

void metrics_send_bytes(int bytes_transferred) {
    server_metrics.bytes_transferred += bytes_transferred;
}

struct metrics * get_metrics() {
    return &server_metrics;
}