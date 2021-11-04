#define _GNU_SOURCE

#include "master_lib.h"
#include "sem_lib.h"
#include "source_lib.h"

void handle_signal(){
    kill(getppid(),SIGINT);
}

int main (int argc, char *argv[]){
    slot* maps;
    keys_storage* my_ks;
    maps_config* my_mp;
    int keys_id, my_id,i;
    taxi_data* my_taxi;
    source_data* source;
    struct message mexSnd;
    struct message mexRcv;
    struct sigaction sa;

    bzero(&sa, sizeof(sa));  
    sa.sa_handler = handle_signal; 
    sigaction(SIGINT, &sa, NULL);


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
    if(((source=shmat(atoi(argv[4]),NULL,0))==(void*)-1)){
        TEST_ERROR;
    }
    my_id = atoi(argv[2]);
    source[my_id].my_pid = my_id;
    source[my_id].origin = atoi(argv[3]);
    source[my_id].destin = randomize_dest(atoi(argv[3]), my_mp);
    mexRcv.type = TAXI_TO_SOURCE;
    mexSnd.type = SOURCE_TO_TAXI;
   
    mexSnd.msgc[1] = source[my_id].destin;
    mexSnd.msgc[0] = my_id;
        /*la source riceve messaggio dal taxi dandogli il suo id e posizione*/
        i = 1;
        while(i>0){
            msgrcv(my_ks->msgq_id, &mexRcv, sizeof(mexRcv)-sizeof(long), mexRcv.type,0);
                source[my_id].my_taxi = mexRcv.msgc[0];
                printf("source=%d taxi = %d \n",my_id,source[my_id].my_taxi);
                /*invio della mia destinazione al taxi*/
                msgsnd(my_ks->msgq_id_so, &mexSnd,sizeof(mexSnd)-sizeof(long),0);
                
                i--;
            
        }

    
 
    /*source che impostano una loro destinazione */
}