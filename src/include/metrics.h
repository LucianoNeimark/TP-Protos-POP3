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



/**
*
* Inicializa las metricas del parser
*/
void metrics_init(void);


/**
*
* Se llama cuando se crea una nueva conexión. Aumenta la variable de CONC y la de HIST
*/
void metrics_new_connection(void);



/**
*
* Decrementa la variable de CONC
*/
void metrics_close_connection(void);


/**
* @param bytes_transferred cantidad de bytes transferidos
* Aumenta la cantidad de bytes transferidos
*/

void metrics_send_bytes(int bytes_transferred);


/**
*
* Devuelva una estructura con toda la información acerca de las métricas
*/
struct metrics * get_metrics(void);

#endif