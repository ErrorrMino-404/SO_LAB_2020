#include <unistd.h>
#define _GNU_SOURCE
#include "master_lib.h"
#include "taxi_lib.h"

maps_config *my_mp;
slot* maps;
int gc_id_shm, mp_id_shm, ho_id_shm, key_id_shm, msgq_id,sem_set_tx,sem_sync_round;
int *position_taxi;
pid_t* taxi_pid;
keys_storage *my_ks;

int main(){
    
    int num_ho;
    int* round_holes, *position_so;
    int taxi_list_pos,taxi_id_shm;
    int i;
    taxi_data* taxi_list;
    struct message mexSnd;
    struct message mexRcv;
    char* args_tx[6] = {TAXI};
    char* args_so[6] = {SOURCE};

    /*allocazione e inizializzazione della memoria condivisa */
    if((gc_id_shm=shmget(IPC_PRIVATE, sizeof(maps_config), IPC_CREAT|0666))==-1){
                TEST_ERROR;
    }

    my_mp = init_maps_config(gc_id_shm);
    
    /*alloco la lista di posizione dei taxi*/
    position_taxi = calloc(my_mp->num_taxi,sizeof(int*));
    
    /*alloco spazio per la lista dei pid*/
    taxi_pid = calloc(my_mp->num_taxi,sizeof(pid_t));
    
    /*alloco e inizializzo la mappa*/
    if((mp_id_shm=shmget(IPC_PRIVATE, my_mp->height*my_mp->width*my_mp->height*sizeof(slot*), IPC_CREAT|0666))==-1){
                TEST_ERROR;
    }

    /*coda di messaggio*/
    if((msgq_id = msgget(IPC_PRIVATE,IPC_CREAT|0666))==-1){
        TEST_ERROR;
    }

    if((sem_sync_round=semget(IPC_PRIVATE, 4, IPC_CREAT|0666))==-1){
                TEST_ERROR
    }
      /*allocazione array lista taxi*/
    if((taxi_list_pos=shmget(IPC_PRIVATE,sizeof(taxi_data)*my_mp->num_taxi, IPC_CREAT|0666))== -1){
                TEST_ERROR;
    }
    if((taxi_list=shmat(taxi_list_pos, NULL, 0))== (void *) -1){
                TEST_ERROR;
    }
      if((taxi_id_shm=shmget(IPC_PRIVATE, (my_mp->num_taxi+1)*sizeof(int), IPC_CREAT|0666))==-1){
        TEST_ERROR;
    }
    /*creazione mappa*/
    maps = create_maps(my_mp->height, my_mp->width,mp_id_shm);
    num_ho = my_mp->holes;

    /*set dei round*/
    for(i=0; i<4; i++){
                switch(i){
                        case 0:
                        if((init_sem_to_val(sem_sync_round, i, my_mp->num_taxi+1))==-1){
                                TEST_ERROR 
                        }
                        break;

                        case WAIT: 
                        if((init_sem_to_val(sem_sync_round, i, 1))==-1){
                                TEST_ERROR 
                        }
                        break;

                        case START:
                        if((init_sem_to_val(sem_sync_round, i, 0))==-1){
                                TEST_ERROR 
                        }
                        break;

                        case END: 
                        if((init_sem_to_val(sem_sync_round, i, 0))==-1){
                                TEST_ERROR 
                        }
                }
    }
    if((sem_set_tx = semget(IPC_PRIVATE, my_mp->num_taxi, IPC_CREAT|0666))==-1){
        TEST_ERROR;
    }
    for(i=0; i<my_mp->num_taxi; i++){
        if((init_sem_to_val(sem_set_tx, i, 0))==-1){
            TEST_ERROR;
        }
    }
  
    /*alloco la memoria condivisa*/
    if((key_id_shm=shmget(IPC_PRIVATE,sizeof(keys_storage), IPC_CREAT|0666))==-1){
        TEST_ERROR;
    }
    
    my_ks = fill_storage_shm(key_id_shm, gc_id_shm, mp_id_shm, msgq_id,sem_sync_round);
    args_tx[1] = integer_to_string_arg(key_id_shm); /*id insieme delle chiavi*/
    /*args[2] id del taxi*/
    args_tx[3] = integer_to_string_arg(taxi_list_pos);
    position_taxi = randomize_coordinate_taxi(taxi_list, maps, my_mp,taxi_id_shm);

    /*Creazione dei processi taxi*/
    for(i=0; i<my_mp->num_taxi; i++){
        switch (taxi_pid[i]=fork()) {
            case -1:
                TEST_ERROR
                break;
            case 0:
                args_tx[2]= integer_to_string_arg(i); /*id del taxi*/
                args_tx[4]=integer_to_string_arg(position_taxi[i]); /*posizione del taxi*/
                execve(TAXI,args_tx,NULL);
                TEST_ERROR
            default:
                break;
        }
    }
    wait_zero(sem_sync_round, 0);
    args_so[1] = integer_to_string_arg(key_id_shm);
    /*genero le source*/
    for(i=0; i < my_mp->source; i++){
        switch (fork()) {
            case -1:
                TEST_ERROR
                break;
            case 0:
                execve(SOURCE,args_so,NULL);
                TEST_ERROR;
                break;
            default:
                break;
        }
    }
    if((ho_id_shm=shmget(IPC_PRIVATE, (num_ho+1)*sizeof(int), IPC_CREAT|0666))==-1){
        TEST_ERROR;
    }
    round_holes = randomize_holes(ho_id_shm, num_ho, my_mp, maps);

    print_maps(maps, my_mp);
    if((shmctl(ho_id_shm, IPC_RMID, NULL))==-1){
                        TEST_ERROR
    }
 
    compute_targets(taxi_list,1,my_mp->num_taxi,maps);
    printf("posizione da raggiungere x = %d e y = %d \n", maps[1].x, maps[1].y);
    for(i=0;i < my_mp->num_taxi; i++){
            printf("posizione %d attuale %d \n",i, taxi_list[i].pos);
            printf("posizione %d da raggiungere %d \n",i, taxi_list[i].target);
            printf("x = %d e y = %d \n",taxi_list[i].x,taxi_list[i].y);
            printf("\n");
    }
}   