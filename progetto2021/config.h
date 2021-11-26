#ifndef _CONF_LIB_
#define _CONF_LIB_
#define TAXI_TO_SOURCE 1
#define SOURCE_TO_TAXI 2
#define TAXI_TO_MASTER 3
#define SOURCE_TO_MASTER 4
#define WAIT 1
#define START 2
#define END 3

#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <signal.h>

#define TEST_ERROR    exit(EXIT_FAILURE);
                        

typedef struct maps_config{
    /*creazione dei processi SO_SOURCES*/
    int source;
    /*SO_HOLES blocchi dove non possono passare*/
    int holes;
    /*creazione della mappa con grandezze x e y*/
    int height;
    int width;
    int num_taxi;       /*id del taxi nella cella*/
    int max_taxi_cell;  /*num massimo di taxi nella cella*/
    int min_taxi_cell;  /*num minimo di taxi nella cella*/
    int timensec_min;   /*tempo minimo di attraversamento di una casella*/
    int timensec_max;   /*tempo massimo di attraverso di una casella*/
    int durantion;      /*tempo di dura*/
    int top_cells;      /*numero di celle maggiormente attraversate*/
    int timeout;        /*tempo di sosta dei taxi*/


}maps_config;

maps_config* init_maps_config();

int find_value(char *);

#endif