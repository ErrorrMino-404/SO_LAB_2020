#ifndef _MAPS_LIB_
#define _MAPS_LIB_
#define RED "[0;31m" 
#define GRN "[0;32m"
#define YEL "[0;33m"

#include "config.h"
#include "sem_lib.h"
#include <sys/shm.h>


typedef struct _slot{
    /*id del semaforo associato alla casella della mappa*/
    key_t sem_id;
    /*se in quella zona è attiva una SO_SOURCE*/
    int val_source;
    int val_holes;
    int num_taxi;
    int attr; /*quanto è stata attreversata la casella*/
    int top_cells;
    int tmp_attr;   /*tempo di attraversamento*/
    /*grandezza della scacchiera*/
    int x;
    int y;
} slot;

slot* create_maps (int, int, int,int,int,int,int);

void print_maps(slot*, maps_config*,int*,int);

void print_metrics(slot*,maps_config*,int*,int,int,int,int,int,int,int,int,int,int);

void clean_sem_maps(int, int, slot*);

void color(char*);

void reset();


#endif