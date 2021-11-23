
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
    struct message mexSndSO;
    struct message mexSndSM;
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
    if(((my_taxi=shmat(atoi(argv[5]),NULL,0))==(void*)-1)){
        TEST_ERROR;
    }
    my_id = atoi(argv[2]);
    source[my_id].my_pid = getpid();
    source[my_id].origin = atoi(argv[3]);
    source[my_id].destin = randomize_dest(source[my_id].origin, my_mp,source,maps);
    mexRcv.type = TAXI_TO_SOURCE;
    mexSndSO.type = SOURCE_TO_TAXI;
    mexSndSM.type = SOURCE_TO_MASTER;


    int to_kill;
    sleep(2);
        /*la source riceve messaggio dal taxi dandogli il suo id e posizione*/
    while(1){
        i=0;
        while(i<1){
            /*source al master*/
            mexSndSM.msgc[0]=source[my_id].origin;
            mexSndSM.msgc[1]=my_id;
            msgsnd(my_ks->msgq_id_sm,&mexSndSM,sizeof(mexSndSM)-sizeof(long),0);
            /*taxi al source*/
            msgrcv(my_ks->msgq_id_ts,&mexRcv,sizeof(mexRcv)-sizeof(long),mexRcv.type,0);
                if(mexRcv.msgc[2]==1){
                    source[mexRcv.msgc[1]].my_taxi = mexRcv.msgc[0];
                    /*messaggio da inviare al taxi*/
                    mexSndSO.msgc[0]=source[my_id].destin;
                    
                    msgsnd(my_ks->msgq_id_st,&mexSndSO,sizeof(mexSndSO)-sizeof(long),0);
                    /*taxi al source con destinazione*/
                    msgrcv(my_ks->msgq_id_ns,&mexRcv,sizeof(mexRcv)-sizeof(long),mexRcv.type,0);
                        source[mexRcv.msgc[0]].my_taxi = 0;
                        if(mexRcv.msgc[2]==-1){ /*nel caso non raggiunga la destinazione*/
                            printf("posizione nuova sou %d pos %d \n",mexRcv.msgc[0],mexRcv.msgc[1]);
                            source[mexRcv.msgc[0]].origin = mexRcv.msgc[1];
                        }   
                }
        }
    }

    
    /*source che impostano una loro destinazione */
}