#include "config.h"
#include "sem_lib.h"
#include <stdio.h>
#include <sys/sem.h>
#include <unistd.h>
#define _GNU_SOURCE
#include "master_lib.h"

maps_config *my_mp;
slot* maps;
int gc_id_shm, mp_id_shm, ho_id_shm, key_id_shm, msgq_id,sem_set_tx,sem_sync_round;
int* array_taxi_id;
pid_t* taxi_pid;
keys_storage *my_ks;

int main(){
    
    int num_ho;
    int* round_holes;
    int* position_taxi;
    int taxi_list_pos,taxi_id_shm;

    int i;
    taxi_data* taxi_list;
    struct message mexSnd;
    struct message mexRcv;
    char* args[6] = {TAXI};

    /*allocazione e inizializzazione della memoria condivisa */
    if((gc_id_shm=shmget(IPC_PRIVATE, sizeof(maps_config), IPC_CREAT|0666))==-1){
                TEST_ERROR;
    }
    my_mp = init_maps_config(gc_id_shm);
    /*alloco array per taxi*/
    array_taxi_id = calloc(my_mp->num_taxi,sizeof(int));
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
    for(i=0; i<4; i++){
                switch(i){
                        case 0:
                        if((init_sem_to_val(sem_sync_round, i, my_mp->num_taxi+1))==-1){
                                TEST_ERROR 
                        }
                        break;

                        case 1: /*WAITING*/
                        if((init_sem_to_val(sem_sync_round, i, 1))==-1){
                                TEST_ERROR 
                        }
                        break;

                        case 2: /*STARTING*/
                        if((init_sem_to_val(sem_sync_round, i, 0))==-1){
                                TEST_ERROR 
                        }
                        break;

                        case 3: /*ENDING*/
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
    maps = create_maps(my_mp->height, my_mp->width,mp_id_shm);
    num_ho = my_mp->holes;
    printf("il valore di num_fl -> %d \n", num_ho);
    /*alloco la memoria condivisa*/
    if((key_id_shm=shmget(IPC_PRIVATE,sizeof(keys_storage), IPC_CREAT|0666))==-1){
        TEST_ERROR;
    }
    my_ks = fill_storage_shm(key_id_shm, gc_id_shm, mp_id_shm, msgq_id,sem_sync_round,sem_set_tx);
    /*allocazione array lista pedine*/
    if((taxi_list_pos=shmget(IPC_PRIVATE,sizeof(taxi_data)*my_mp->num_taxi, IPC_CREAT|0666))== -1){
                TEST_ERROR;
    }
    if((taxi_list=shmat(taxi_list_pos, NULL, 0))== (void *) -1){
                TEST_ERROR;
    }
      if((taxi_id_shm=shmget(IPC_PRIVATE, (my_mp->num_taxi+1)*sizeof(int), IPC_CREAT|0666))==-1){
        TEST_ERROR;
    }
    args[1] = integer_to_string_arg(key_id_shm); /*id insieme delle chiavi*/
    /*args[2] id del taxi*/
    args[3] = integer_to_string_arg(taxi_list_pos);
    position_taxi = randomize_coordinate_taxi(taxi_list, maps, my_mp,taxi_id_shm);
    for(i=0; i<my_mp->num_taxi; i++){
        sem_reserve(my_ks->sem_set_pl, i );
        switch (fork()) {
            case -1:
                TEST_ERROR
                break;
            case 0:
                args[2]= integer_to_string_arg(i); /*id del taxi*/
                args[4]=integer_to_string_arg(position_taxi[i]); /*posizione del taxi*/
                execve(TAXI,args,NULL);
                TEST_ERROR
            default:
                break;
        }
       sem_relase(my_ks->sem_set_pl, ((i+1)%(my_mp->num_taxi)));

    }
    sleep(1);


    if((ho_id_shm=shmget(IPC_PRIVATE, (num_ho+1)*sizeof(int), IPC_CREAT|0666))==-1){
        TEST_ERROR;
    }
    round_holes = randomize_holes(ho_id_shm, num_ho, my_mp, maps);

    print_maps(maps, my_mp);
    if((shmctl(ho_id_shm, IPC_RMID, NULL))==-1){
                        TEST_ERROR
    }

}