#ifndef _M_LIB_
#define _M_LIB_
#include "config.h"
#include "maps.h"
#include "sem_lib.h"

#define TAXI "taxi"
#define SOURCE "source"
typedef struct _keys_storage {
    int ks_shm_id;          /*id del key storage*/
    int conf_id;            /*configurazione id*/
    int maps_id;            /*id mappa */
    int msgq_id;            /*id della coda dei messaggi*/
    int round_source_id ;   /*id del puntatore al source*/
    int sem_set_tx;
    int sem_sync_round;     /*id del secondo semaforo di sincronizzazione player-master-pawn*/
}keys_storage;

struct message {
    long type;
    int msgc[2];
};
typedef struct _taxi_data{
    pid_t my_pid;
    int target;     /*richiesta so da raggiungere taxi*/
    int dest;       /*destinazione della source*/
    int pos;        /*posizione all'interno della mappa*/
    int x;
    int y;
}taxi_data;

typedef struct _source_data{
    pid_t my_pid;
    int origin;  /*origine in cui viene generata la richiesta*/
    int destin;  /*destinazione che si deve recare */
    int my_taxi; /*il taxi incaricato*/
}source_data;

keys_storage* fill_storage_shm(int, int, int, int, int);

int get_rand_so(int,int);

int* randomize_holes(int, int, maps_config*, slot*);

/*restituisce un char* che corrisponde all'int passato come argomento*/
char* integer_to_string_arg(int);

int* randomize_coordinate_taxi (taxi_data*,slot*, maps_config*,int);

/*targa del taxi che deve raggiungere quella posizione*/
void compute_targets(taxi_data*, int, slot*);
#endif