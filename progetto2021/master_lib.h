#ifndef _M_LIB_
#define _M_LIB_
#define MAX_SOURCE 20
#include "maps.h"
#include "math.h"

#define TAXI "taxi"
#define SOURCE "source"
typedef struct _keys_storage {
    int ks_shm_id;          /*id del key storage*/
    int so_pos;
    int conf_id;            /*configurazione id*/
    int maps_id;            /*id mappa */
    int msgq_id;            /*id della coda dei messaggi*/
    int msgq_id_ts;         /*id messaggi condivisi tra taxi e source*/
    int msgq_id_sm;         /*messaggi condivisi tra source e master*/
    int msgq_id_st;         /*messaggi condivisi tra source e taxi */
    int msgq_id_ns;         /*messaggio che il taxi invia al source che non ha raggiunto la destinazione*/
    int msgq_id_end;        /*coda dei messaggi di terminazione taxi causa allarme*/
    int state;
    int sem_sync_round;     /*id del secondo semaforo di sincronizzazione player-master-pawn*/
}keys_storage;

struct message {
    long type;
    int msgc[3];
};
typedef struct _taxi_data{
    pid_t my_pid;
    int target;     /*richiesta so da raggiungere taxi*/
    int dest;       /*destinazione della source*/
    int pos;        /*posizione all'interno della mappa*/
    int car_so;     /*source che prende in carico il taxi*/
    int move;       /*movimento del taxi*/
    int exp_so;     /*source prese in carico*/
    int time;       /*timer delle tempistiche di viaggio viene resettato ogni volta*/
    int x;
    int y;
}taxi_data;

typedef struct _source_data{
    pid_t my_pid;
    int origin;  /*origine in cui viene generata la richiesta*/
    int destin;  /*destinazione che si deve recare */
    int my_taxi; /*il taxi incaricato*/
    int x;       /*posizione sulla mappa*/
    int y;
}source_data;

keys_storage* fill_storage_shm(int,int,int,int,int,int,int,int,int,int,int);

int get_rand_so(int,int);

void randomize_holes(int, maps_config*, slot*);

/*restituisce un char* che corrisponde all'int passato come argomento*/
char* integer_to_string_arg(int);

int* randomize_coordinate_taxi (taxi_data*,slot*, maps_config*,int);

int* randomize_coordinate_source (source_data*, slot*, maps_config*,int);

void print_metrics(maps_config*,int*);

/*targa del taxi che deve raggiungere quella posizione*/
void compute_targets(taxi_data*, int, slot*, int*);

int calculate_top_taxi(taxi_data*,int);

int calculate_taxi_succes(taxi_data*,int);

int calculate_taxi_time(taxi_data *, int);

void create_new_taxi(maps_config*,slot*,int,taxi_data*,int,int,int*,int,int,pid_t*);

void check_taxi(maps_config*,slot*,taxi_data*,int,int,int*,pid_t*);
#endif