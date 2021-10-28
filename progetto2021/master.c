#include "config.h"
#include "sem_lib.h"
#include <stdlib.h>
#include <unistd.h>
#define _GNU_SOURCE
#include "master_lib.h"
#include "taxi_lib.h"

maps_config *my_mp;
slot* maps;
int gc_id_shm, mp_id_shm, ho_id_shm, key_id_shm, msgq_id,sem_set_tx,sem_sync_round;
int *position_taxi, *position_source;
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
    /*alloco la lista di posizione dei source*/
    position_so = calloc(my_mp->source, sizeof(int*));
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
    if((ho_id_shm=shmget(IPC_PRIVATE, (num_ho+1)*sizeof(int), IPC_CREAT|0666))==-1){
            TEST_ERROR;
    }
        round_holes = randomize_holes(ho_id_shm, num_ho, my_mp, maps);
    /*alloco la memoria condivisa*/
    if((key_id_shm=shmget(IPC_PRIVATE,sizeof(keys_storage), IPC_CREAT|0666))==-1){
        TEST_ERROR;
    }
    
    my_ks = fill_storage_shm(key_id_shm, gc_id_shm, mp_id_shm, msgq_id,sem_sync_round);
    args_tx[1] = integer_to_string_arg(key_id_shm); /*id insieme delle chiavi*/
    /*args[2] id del taxi*/
    args_tx[3] = integer_to_string_arg(taxi_list_pos);
    position_taxi = randomize_coordinate_taxi(taxi_list, maps, my_mp);
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
    wait_zero(sem_sync_round,0);
    

    int num_so = 3;
    mexRcv.type = TAXI_TO_MASTER;
    while(1){
        
        increase_resource(sem_sync_round,START, my_mp->num_taxi);
        
        compute_targets(taxi_list, my_mp->num_taxi,maps);
        print_maps(maps, my_mp);
        sem_reserve(sem_sync_round, WAIT);
        printf("ho superato wait1 \n");
        /*da qui ho problemi*/
        check_zero(sem_sync_round, START);
        printf("ho superato start1 \n");
        sem_relase(sem_sync_round, WAIT);
        printf("ho superato wait 2\n");
        while(num_so>0){
            printf("sto aspettando il messaggio \n");
            msgrcv(my_ks->msgq_id, &mexRcv, sizeof(mexRcv)-sizeof(long), mexRcv.type,0);
            printf("messaggio arrivato da %d \n",mexRcv.msgc[0]);
            num_so-=1;
            
        }
        printf("sono uscito dal while 2\n");
        increase_resource(sem_sync_round,END, my_mp->num_taxi+1);

        print_maps(maps, my_mp);
        check_zero(sem_sync_round,END);
        if((shmctl(ho_id_shm, IPC_RMID, NULL))==-1){
                            TEST_ERROR
        }
    
    }
}   