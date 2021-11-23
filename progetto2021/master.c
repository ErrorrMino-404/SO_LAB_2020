
#define _GNU_SOURCE
#include "master_lib.h"
maps_config *my_mp;
slot* maps;
int gc_id_shm,sem_set_tx, mp_id_shm, ho_id_shm, so_id_shm,tx_id_shm,key_id_shm,sem_sync_round;
int msgq_id_sm, msgq_id,msgq_id_so,msgq_id_ds,msgq_id_ns,msgq_id_end,state,new_id,ex_pos,start;
int  succ,aborti,inve,fd[2];/*variabili da stampare*/
int *position_taxi, *position_so, *array_id_taxi;
int taxi_list_pos,source_list_pos, num_so,ex_id,ex_pos,mex_so;
struct message mexSnd;
struct message mexRcv;
struct message mexRcvDS;
struct message mexRcvSM;
char* args_tx[6] = {TAXI},*args_so[6] = {SOURCE};
pid_t* taxi_pid,*so_pid;
keys_storage *my_ks;
taxi_data* taxi_list;
source_data* source_list;

void handle_signal(int signum){
        int i, j;
    switch(signum){
        case SIGINT:
            printf("GAME ENDED\nPrinting chessboard and statistics.\n\n");
            for(i=1; i<my_mp->num_taxi+1; i++){
                    kill(taxi_pid[i], SIGTERM);  
            }
            for(i=0; i<my_mp->source; i++){
                kill(source_list[i].my_pid, SIGTERM);
            }
            print_metrics(my_mp, array_id_taxi);
            /*rimozione semafori e code*/
            semctl(sem_sync_round, 0, IPC_RMID);
            msgctl(msgq_id, IPC_RMID, 0); 
            msgctl(msgq_id_so, IPC_RMID, 0);
            msgctl(msgq_id_sm, IPC_RMID, 0);
            msgctl(msgq_id_ds, IPC_RMID, 0);
            msgctl(msgq_id_ns, IPC_RMID, 0);
            msgctl(msgq_id_end, IPC_RMID, 0);
            /*free dei puntatori*/
            free(array_id_taxi);
            free(taxi_pid);
            /*pulizia semafori*/
            clean_sem_maps(my_mp->height, my_mp->width, maps);
            /*rimozione ipcs*/
            shmctl(mp_id_shm, IPC_RMID, NULL);
            shmctl(gc_id_shm, IPC_RMID, NULL);
            shmctl(key_id_shm, IPC_RMID, NULL);
            shmctl(ho_id_shm, IPC_RMID, NULL);
            shmctl(so_id_shm, IPC_RMID, NULL);
            shmctl(tx_id_shm, IPC_RMID, NULL);
            exit(EXIT_FAILURE);
        break;

        default:
        break;
    }

}

int main(){
    
    int num_ho,top_taxi,taxi_succes;
    int* holes;
    int i,j,aspetta;
    int my_x,my_y,ok,sem,x,new;
    struct sigaction sa;
    struct sigaction action;
    sigset_t my_mask;



    bzero(&sa, sizeof(sa));  
    sa.sa_handler = handle_signal; 
    sigaction(SIGINT, &sa, NULL);


    /*allocazione e inizializzazione della memoria condivisa */
    if((gc_id_shm=shmget(IPC_PRIVATE, sizeof(maps_config), IPC_CREAT|0666))==-1){
                TEST_ERROR;
    }
    my_mp = init_maps_config(gc_id_shm);
    array_id_taxi = calloc(my_mp->num_taxi,sizeof(int*));
    taxi_pid = calloc(my_mp->num_taxi,sizeof(pid_t));
    so_pid = calloc (my_mp->source, sizeof(pid_t));
    /*alloco e inizializzo la mappa*/
    if((mp_id_shm=shmget(IPC_PRIVATE, my_mp->height*my_mp->width*my_mp->height*sizeof(slot*), IPC_CREAT|0666))==-1){
                TEST_ERROR;
    }
    /*coda di messaggio tra taxi e master se ha raggiunto la source*/
    if((msgq_id = msgget(IPC_PRIVATE,IPC_CREAT|0666))==-1){
        TEST_ERROR;
    }
    /*coda di messaggi tra taxi e source*/
    if((msgq_id_so = msgget(IPC_PRIVATE,IPC_CREAT|0666))==-1){
        TEST_ERROR;
    }
    /*coda di messaggi tra source e master */
    if((msgq_id_sm = msgget(IPC_PRIVATE,IPC_CREAT|0666))==-1){
        TEST_ERROR;
    }
    /*coda di messagi tra taxi e master per sapere se ha raggiunto la destinazione*/
    if((msgq_id_ds = msgget(IPC_PRIVATE,IPC_CREAT|0666))==-1){
        TEST_ERROR;
    }

    if((msgq_id_ns = msgget(IPC_PRIVATE,IPC_CREAT|0666))==-1){
        TEST_ERROR;
    }
    /*coda di messaggi per creare un taxi al posto di quello morto*/
    if((msgq_id_end = msgget(IPC_PRIVATE,IPC_CREAT|0666))==-1){
        TEST_ERROR;
    }
    if((state =msgget(IPC_PRIVATE,IPC_CREAT|0666))==-1){
        TEST_ERROR;
    }
    /*sincronizzazione tra i taxi*/
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
    if((source_list_pos=shmget(IPC_PRIVATE,sizeof(source_data)*my_mp->source, IPC_CREAT|0666))== -1){
                TEST_ERROR;
    }
    if((source_list=shmat(source_list_pos, NULL, 0))== (void *) -1){
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
    if((sem_set_tx=semget(IPC_PRIVATE, my_mp->num_taxi, IPC_CREAT|0666))==-1){
        TEST_ERROR;
    }
    for(i=0; i<my_mp->num_taxi; i++){
                if((init_sem_to_val(sem_set_tx, i, 0))==-1){
                        TEST_ERROR 
                }
    }
    if((ho_id_shm=shmget(IPC_PRIVATE, (num_ho+1)*sizeof(int), IPC_CREAT|0666))==-1){
            TEST_ERROR;
    }
    holes = randomize_holes(ho_id_shm, num_ho, my_mp, maps);
    if((so_id_shm=shmget(IPC_PRIVATE, (my_mp->source+1)*sizeof(int), IPC_CREAT|0666))==-1){
            TEST_ERROR;
    }
     if((tx_id_shm=shmget(IPC_PRIVATE, (my_mp->num_taxi+1)*sizeof(int), IPC_CREAT|0666))==-1){
            TEST_ERROR;
    } 
    /*alloco la memoria condivisa*/
    if((key_id_shm=shmget(IPC_PRIVATE,sizeof(keys_storage), IPC_CREAT|0666))==-1){
        TEST_ERROR;
    }
    if(pipe(fd)<0){
        TEST_ERROR;
    }
    my_ks=fill_storage_shm(key_id_shm, gc_id_shm, mp_id_shm, msgq_id,msgq_id_so,msgq_id_sm,msgq_id_ds,msgq_id_ns,msgq_id_end,state,sem_sync_round,fd[0],fd[1]);
    args_tx[1] = integer_to_string_arg(key_id_shm); /*id insieme delle chiavi*/
    /*args[2] id del taxi*/
    args_tx[3] = integer_to_string_arg(taxi_list_pos);
    position_taxi = randomize_coordinate_taxi(taxi_list, maps, my_mp,tx_id_shm);
    /*Creazione dei processi taxi*/
    for(i=1; i<my_mp->num_taxi+1; i++){
        

        switch (taxi_pid[i]=fork()) {
            case -1:
                TEST_ERROR
                break;
            case 0:
                array_id_taxi[i]= i;
                args_tx[2]= integer_to_string_arg(i); /*id del taxi*/
                args_tx[4]=integer_to_string_arg(position_taxi[i]); /*posizione del taxi*/
                execve(TAXI,args_tx,NULL);
                TEST_ERROR
            default:
                break;
        }
        

    }
    sleep(2);
    args_so[1] = integer_to_string_arg(key_id_shm); /*id insieme delle chiavi*/
    args_so[4] = integer_to_string_arg(source_list_pos);
    args_so[5] = integer_to_string_arg(taxi_list_pos);
    position_so = randomize_coordinate_source(source_list,maps,my_mp, so_id_shm);
    for(i =0; i<my_mp->source;i++){
        switch(so_pid[i]=fork()){
            case -1:
                TEST_ERROR;
                break;
            case 0:
                args_so[2]=integer_to_string_arg(i);
                args_so[3]=integer_to_string_arg(position_so[i]);
                execve(SOURCE,args_so,NULL);
                TEST_ERROR;
                
            default:
                break;
        }
    }
    mexRcv.type = TAXI_TO_MASTER;
    mexRcvSM.type = SOURCE_TO_MASTER;
    mexRcvDS.type = TAXI_TO_MASTER;
    succ = 0; /*numero di source andate a buonfine*/
    aborti = 0; /*numero di source prese da un taxi, ma che non hanno raggiunto la destinazione*/
    inve = my_mp->source; /*richieste source non ancora raggiunte*/
    i=0;
    

    while(1){
        check_taxi(my_mp,maps,taxi_list,key_id_shm,taxi_list_pos,position_taxi);
        top_taxi = calculate_top_taxi(taxi_list, my_mp->num_taxi+1);
        taxi_succes = calculate_taxi_succes(taxi_list, my_mp->num_taxi+1);
        start=my_mp->num_taxi;
        j=0;
            printf("superato \n");
            x = 0;
            int pos_source[5];
            while(x<2){
                if((msgrcv(my_ks->msgq_id_sm,&mexRcvSM,sizeof(mexRcvSM)-sizeof(long),mexRcvSM.type,0)==-1)){
                    TEST_ERROR
                }
                pos_source[x]=mexRcvSM.msgc[0];
                printf("messaggio ricevuto da %d posizione %d \n",mexRcvSM.msgc[1],mexRcvSM.msgc[0]);
                x++;
                
            }
            pos_source[x]=-1;
            compute_targets(taxi_list,my_mp->num_taxi,maps,pos_source);
            print_maps(maps,my_mp,pos_source, top_taxi,taxi_succes,succ,aborti,inve);
            increase_resource(sem_sync_round,START,my_mp->num_taxi);
            sem_reserve(sem_sync_round, WAIT);
            check_zero(sem_sync_round, START); 
            sem_relase(sem_sync_round, WAIT);
                printf("superato tutto \n");
                    while(x>0){
                        if((msgrcv(my_ks->msgq_id,&mexRcv,sizeof(mexRcv)-sizeof(long),mexRcv.type,0))==-1){
                            TEST_ERROR;
                        }
                        if(mexRcv.msgc[2]==1){
                            printf("taxi arrivato \n");
                            position_taxi[mexRcv.msgc[0]]=mexRcv.msgc[1];
                            maps[mexRcv.msgc[1]].num_taxi=mexRcv.msgc[0];
                            kill(source_list[taxi_list[mexRcv.msgc[0]].car_so].my_pid,SIGTERM);
                            succ++;
                            inve--; 
                        }else if(mexRcv.msgc[2]==-1){ 
                            printf("taxi non arrivato \n");
                                new_id=mexRcv.msgc[0];
                                ex_pos=mexRcv.msgc[1];
                                maps[ex_pos].num_taxi = 0;
                                position_taxi[new_id]=0;
                                create_new_taxi(my_mp,maps,new_id,taxi_list,key_id_shm,taxi_list_pos,position_taxi,array_id_taxi,sem_sync_round,ex_pos,0);
                        }else if(mexRcv.msgc[2]==0){
                                printf("taxi non arrivato dist\n");
                            aborti++;
                                new_id=mexRcv.msgc[0];
                                ex_pos=mexRcv.msgc[1];
                                maps[ex_pos].num_taxi = 0;
                                position_taxi[new_id]=0;
                                source_list[taxi_list[new_id].car_so].origin=ex_pos;
                                maps[ex_pos].val_source = taxi_list[new_id].car_so;
                                create_new_taxi(my_mp,maps,new_id,taxi_list,key_id_shm,taxi_list_pos,position_taxi,array_id_taxi,sem_sync_round,ex_pos,0);
                            
                        }
                        x--;
                    }
                increase_resource(sem_sync_round,END,my_mp->num_taxi);
                check_zero(sem_sync_round,END);
                printf("termino end \n");
        
    }
    

}   