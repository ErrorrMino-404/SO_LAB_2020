#ifndef _CONF_LIB_
#define _CONF_LIB_
#define MASTER_TO_TAXI 1
#define TAXI_TO_MASTER 3
#define WAIT 1
#define START 2
#define END 3

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <signal.h>
#include <math.h>
#define TEST_ERROR      printf("%s:%d: PID=%5d: Error %d (%s)\n",__FILE__,__LINE__,getpid(),errno,strerror(errno));\
                        printf("exit:EXIT_FAILURE\n");\
                        raise(SIGINT);

typedef struct maps_config{
    /*creazione dei processi SO_SOURCES*/
    int source;
    /*SO_HOLES blocchi dove non possono passare*/
    int holes;
    /*creazione della mappa con grandezze x e y*/
    int height;
    int width;
    int num_taxi; /*id del taxi nella cella*/
    int max_taxi_cell; /*num massimo di taxi nella cella*/
    int min_taxi_cell; /*num minimo di taxi nella cella*/
    
    int top_cells; /*numero di celle maggiormente attraversate*/


}maps_config;

maps_config* init_maps_config();

int find_value(char *);

#endif