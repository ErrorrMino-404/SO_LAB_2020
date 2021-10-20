#ifndef _M_LIB_
#define _M_LIB_
#include "maps.h"
#define TAXI "taxi"
typedef struct _keys_storage {
    int ks_shm_id;          /*id del key storage*/
    int conf_id;            /*configurazione id*/
    int maps_id;            /*id mappa */
    int msgq_id;            /*id della coda dei messaggi*/
    int round_source_id ;   /*id del puntatore al source*/
    int sem_sync_round;     /*id del secondo semaforo di sincronizzazione player-master-pawn*/
    int sem_set_pl;         /*id del set di semafori circolari per la sincronizzazione tra giocatori*/
}keys_storage;

struct message {
    long type;
    int msgc[2];
};
typedef struct _taxi_data{
    pid_t my_pid;
    int pos; /*posizione all'interno della mappa*/
    int x;
    int y;
}taxi_data;

keys_storage* fill_storage_shm(int, int, int, int,  int);

int get_rand_so(int,int);

int* randomize_holes(int, int, maps_config*, slot*);

/*restituisce un char* che corrisponde all'int passato come argomento*/
char* integer_to_string_arg(int);

void randomize_coordinate_taxi (taxi_data*,slot*,int, int,int);
#endif