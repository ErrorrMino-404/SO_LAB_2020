#include "config.h"
#include "sem_lib.h"
#include <signal.h>
#include <stdio.h>
#include <strings.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>
#define _GNU_SOURCE
#include "taxi_lib.h"

void handle_signal(){
    kill(getppid(),SIGINT);
}

int main(int argc,char *argv[]){
    slot* maps;
    keys_storage* my_ks;
    maps_config* my_mp;
    int* arr_target;
    int keys_id, my_id;
    taxi_data* taxi_list;
    struct message mexSnd;
    struct message mexRcv;
    struct sembuf sops;


    
    /* configurazione memoria condivisa*/
    keys_id = atoi(argv[1]);
    if((my_ks = shmat(keys_id,NULL,0))==(void*)-1){
        TEST_ERROR;
    }
    if((my_mp=shmat(my_ks->conf_id,NULL,0))==(void*)-1){
        TEST_ERROR;
    }
    if((maps=shmat(my_ks->maps_id,NULL,0))==(void*)-1){
        TEST_ERROR;
    }
    if((taxi_list=shmat(atoi(argv[3]),NULL,0))==(void*)-1){
        TEST_ERROR;
    }


    
    /*inizializzo struct del taxi*/
    my_id = atoi(argv[2]);
    /*impostare il tempo di vita*/
    taxi_list[my_id].my_pid = getpid();
    taxi_list[my_id].pos = atoi(argv[4]);

    /*alloco il taxi nella mappa*/
    allocate_taxi(taxi_list[my_id].x,taxi_list[my_id].y,my_id, maps, my_mp->width );
    /*Comunicazione dei taxi con il master*/
    
  
    wait_zero(my_ks->sem_sync_round,0);


    /*spostamento del taxi nella mappa*/


    /*while(1){
        check_zero(my_ks->sem_sync_round, WAIT);
        if((arr_target=(int*)shmat(my_ks->round_source_id,NULL,0))==(void*)-1){
            TEST_ERROR;
        }
        wait_zero(my_ks->sem_sync_round,START);
        
    }*/
}

