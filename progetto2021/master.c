#define _GNU_SOURCE
#include "master_lib.h"
maps_config *my_mp;
slot* maps;
int gc_id_shm, mp_id_shm,so_id_shm,tx_id_shm,key_id_shm,sem_sync;
int msgq_id_sm, msgq_id,msgq_id_so,msgq_id_ds,msgq_id_end,state,new_id,ex_pos,start;
int  succ,aborti,inve,num_ho,top_taxi,taxi_succes,ex_top_taxi,ex_taxi_succes;/*variabili da stampare*/
int *position_taxi, *position_so, *array_id_taxi,*holes, i,j,aspetta, *pos_source;
int taxi_list_pos,source_list_pos, num_so,ex_id,ex_pos,mex_so,my_x,my_y,ok,sem,x,num_source;
int val_top_taxi, val_taxi_succes,ex_val_tpt,ex_val_tss,top_time,val_time,ex_top_time,ex_val_time;
struct message mexSnd;
struct message mexRcv;
struct message mexRcvDS;
struct message mexRcvSM;    
char* args_tx[6] = {TAXI},*args_so[6] = {SOURCE};
pid_t* taxi_pid,*so_pid;
keys_storage *my_ks;
taxi_data* taxi_list;
source_data* source_list;
struct timeval timer;

void handle_signal(int signum){

        if(signum==SIGALRM){
                ex_taxi_succes=taxi_succes;
                ex_top_taxi=top_taxi;
                ex_top_time=top_time;
                ex_val_tpt=val_top_taxi;
                ex_val_tss=val_taxi_succes;
                ex_val_time=val_time;
                top_taxi=calculate_top_taxi(taxi_list, my_mp->num_taxi+1);
                val_top_taxi=taxi_list[top_taxi].move;
                taxi_succes=calculate_taxi_succes(taxi_list, my_mp->num_taxi+1);
                val_taxi_succes=taxi_list[taxi_succes].exp_so;
                top_time=calculate_taxi_time(taxi_list, my_mp->num_taxi+1);
                val_time=taxi_list[top_time].time;
                if(val_top_taxi<ex_val_tpt){
                    top_taxi=ex_top_taxi;
                    val_top_taxi = ex_val_tpt;
                }
                if(val_taxi_succes<ex_val_tss){
                    taxi_succes = ex_taxi_succes;
                    val_taxi_succes = ex_val_tss;
                }
                if(val_time<ex_val_time){
                    top_time = ex_top_time;
                    val_time = ex_val_time;
                }
                print_maps(maps,my_mp,pos_source,num_source);
                alarm(1);
           
        } else {
                    int i, j;
            printf("GAME ENDED\nPrinting chessboard and statistics.\n\n");
            for(i=1; i<my_mp->num_taxi+1; i++){
                    kill(taxi_pid[i], SIGTERM);  
            }
            shmctl(taxi_list_pos,IPC_RMID,NULL);
            for(i=0; i<my_mp->source; i++){
                kill(source_list[i].my_pid, SIGTERM);
            }
            shmctl(source_list_pos,IPC_RMID,NULL);
            print_metrics(maps,my_mp, position_so,top_taxi,taxi_succes,succ,aborti,inve,val_top_taxi,val_taxi_succes,top_time,val_time,num_source);
            /*rimozione semafori e code*/
            semctl(sem_sync, 0, IPC_RMID);
            msgctl(msgq_id, IPC_RMID, 0); 
            msgctl(msgq_id_so, IPC_RMID, 0);
            msgctl(msgq_id_sm, IPC_RMID, 0);
            msgctl(msgq_id_ds, IPC_RMID, 0);
            msgctl(msgq_id_end, IPC_RMID, 0);
            msgctl(state, IPC_RMID, 0);
            /*free dei puntatori*/
            free(array_id_taxi);
            free(taxi_pid);
            free(so_pid);
            /*pulizia semafori*/
            clean_sem_maps(my_mp->height, my_mp->width, maps);
            /*rimozione ipcs*/
            shmctl(mp_id_shm, IPC_RMID, NULL);
            shmctl(gc_id_shm, IPC_RMID, NULL);
            shmctl(key_id_shm, IPC_RMID, NULL);
            shmctl(so_id_shm, IPC_RMID, NULL);
            shmctl(tx_id_shm, IPC_RMID, NULL);
                /*fine gioco SIGINT*/
                printf("The game ended because the SIGINT signal was received.\n");
                exit(EXIT_FAILURE);
        }
    }
void time_stamp(){
    struct timeval pass;
    long s,u,n;
    gettimeofday(&pass,NULL);
    s=pass.tv_sec - timer.tv_sec;
    u=pass.tv_usec - timer.tv_usec;
    n = s + (u * 0.0000001);
    if(n>=(my_mp->durantion)){
                    int i, j;
            printf("GAME ENDED\nPrinting chessboard and statistics.\n\n");
            for(i=1; i<my_mp->num_taxi+1; i++){
                    kill(taxi_pid[i], SIGTERM);  
            }
            shmctl(taxi_list_pos,IPC_RMID,NULL);
            for(i=0; i<my_mp->source; i++){
                kill(source_list[i].my_pid, SIGTERM);
            }
            shmctl(source_list_pos,IPC_RMID,NULL);
            print_metrics(maps,my_mp, position_so,top_taxi,taxi_succes,succ,aborti,inve,val_top_taxi,val_taxi_succes,top_time,val_time,num_source);
            /*rimozione semafori e code*/
            semctl(sem_sync, 0, IPC_RMID);
            msgctl(msgq_id, IPC_RMID, 0); 
            msgctl(msgq_id_so, IPC_RMID, 0);
            msgctl(msgq_id_sm, IPC_RMID, 0);
            msgctl(msgq_id_ds, IPC_RMID, 0);
            msgctl(msgq_id_end, IPC_RMID, 0);
            msgctl(state, IPC_RMID, 0);
            /*free dei puntatori*/
            free(array_id_taxi);
            free(taxi_pid);
            free(so_pid);
            /*pulizia semafori*/
            clean_sem_maps(my_mp->height, my_mp->width, maps);
            /*rimozione ipcs*/
            shmctl(mp_id_shm, IPC_RMID, NULL);
            shmctl(gc_id_shm, IPC_RMID, NULL);
            shmctl(key_id_shm, IPC_RMID, NULL);
            shmctl(so_id_shm, IPC_RMID, NULL);
            shmctl(tx_id_shm, IPC_RMID, NULL);
                /*fine gioco SIGINT*/
                printf("The game ended because the SIGALRM signal was received.\n");
                exit(EXIT_SUCCESS);
        }
  
}
int main(){
    struct sigaction sa; 
    sigset_t my_mask;
    bzero(&sa, sizeof(sa));  
    sa.sa_handler = handle_signal; 
    sigaction(SIGINT, &sa, NULL);
    signal(SIGALRM,handle_signal);
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
        TEST_ERROR
    }
    /*coda di messaggio tra taxi e master se ha raggiunto la source*/
    if((msgq_id = msgget(IPC_PRIVATE,IPC_CREAT|0666))==-1){
        TEST_ERROR
    }
    /*coda di messaggi tra taxi e source*/
    if((msgq_id_so = msgget(IPC_PRIVATE,IPC_CREAT|0666))==-1){
        TEST_ERROR
    }
    /*coda di messaggi tra source e master */
    if((msgq_id_sm = msgget(IPC_PRIVATE,IPC_CREAT|0666))==-1){
        TEST_ERROR
    }
    /*coda di messagi tra taxi e master per sapere se ha raggiunto la destinazione*/
    if((msgq_id_ds = msgget(IPC_PRIVATE,IPC_CREAT|0666))==-1){
        TEST_ERROR
    }
    /*coda di messaggi per creare un taxi al posto di quello morto*/
    if((msgq_id_end = msgget(IPC_PRIVATE,IPC_CREAT|0666))==-1){
        TEST_ERROR
    }
    if((state =msgget(IPC_PRIVATE,IPC_CREAT|0666))==-1){
        TEST_ERROR
    }
    /*sincronizzazione tra i taxi*/
    if((sem_sync=semget(IPC_PRIVATE, 4, IPC_CREAT|0666))==-1){
        TEST_ERROR
    }
    /*allocazione array lista taxi*/
    if((taxi_list_pos=shmget(IPC_PRIVATE,sizeof(taxi_data)*my_mp->num_taxi, IPC_CREAT|0666))== -1){
        TEST_ERROR
    }
    if((taxi_list=shmat(taxi_list_pos, NULL, 0))== (void *) -1){
        TEST_ERROR
    }
    if((source_list_pos=shmget(IPC_PRIVATE,sizeof(source_data)*my_mp->source, IPC_CREAT|0666))== -1){
        TEST_ERROR
    }
    if((source_list=shmat(source_list_pos, NULL, 0))== (void *) -1){
        TEST_ERROR
    }
    /*creazione mappa*/
    maps = create_maps(my_mp->height, my_mp->width,mp_id_shm,my_mp->timensec_min,my_mp->timensec_max,my_mp->min_taxi_cell,my_mp->max_taxi_cell);
    num_ho = my_mp->holes;
    /*set dei round*/
    for(i=0; i<4; i++){
                switch(i){
                        case 0:
                        if((init_sem_to_val(sem_sync, i, my_mp->num_taxi+1))==-1){
                                TEST_ERROR 
                        }
                        break;

                        case WAIT: 
                        if((init_sem_to_val(sem_sync, i, 1))==-1){
                                TEST_ERROR 
                        }
                        break;

                        case START:
                        if((init_sem_to_val(sem_sync,i, 0))==-1){
                                TEST_ERROR 
                        }
                        break;

                        case END: 
                        if((init_sem_to_val(sem_sync, i, 0))==-1){
                                TEST_ERROR 
                        }
                }
    }

    randomize_holes(num_ho, my_mp, maps);
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

    my_ks=fill_storage_shm(key_id_shm, gc_id_shm, mp_id_shm, msgq_id,msgq_id_so,msgq_id_sm,msgq_id_ds,msgq_id_end,state,sem_sync);
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

    args_so[1] = integer_to_string_arg(key_id_shm); /*id insieme delle chiavi*/
    args_so[4] = integer_to_string_arg(source_list_pos);
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
    succ = 0;                               /*numero di source andate a buonfine*/
    aborti = 0;                             /*numero di source prese da un taxi, ma che non hanno raggiunto la destinazione*/ 
    i=0;
    top_taxi=0;
    taxi_succes=0;
    top_time=0;
    val_taxi_succes=0;
    val_top_taxi=0;
    val_time=0;
    
    printf("Quante richieste source posso avere ad ogni ciclo(max %d source e taxi %d) =>",my_mp->source,my_mp->num_taxi);
    scanf("%d",&num_source);
    pos_source=calloc(num_source,sizeof(int*));
    gettimeofday(&timer,NULL);
    alarm(1);
    while(1){
        for (x=0;x<my_mp->source;x++){
            kill(source_list[x].my_pid,SIGUSR1);
        }
        inve+=num_source;
        check_taxi(my_mp,maps,taxi_list,key_id_shm,taxi_list_pos,position_taxi,taxi_pid);
            for(x=0;x<num_source;x++){
                msgrcv(my_ks->msgq_id_sm,&mexRcvSM,sizeof(mexRcvSM)-sizeof(long),mexRcvSM.type,0);
                
                pos_source[x]=mexRcvSM.msgc[0];
            }
        pos_source[x]=-1;
        time_stamp();
                compute_targets(taxi_list,my_mp->num_taxi,maps,pos_source);
                /*avvio del movimento dei taxi con target!=-1*/
                increase_resource(sem_sync,START,my_mp->num_taxi);
                sem_reserve(sem_sync, WAIT);
                check_zero(sem_sync, START); 
                sem_relase(sem_sync, WAIT);
                        while(x>0){
                            msgrcv(my_ks->msgq_id,&mexRcv,sizeof(mexRcv)-sizeof(long),mexRcv.type,0);
                            if(mexRcv.msgc[2]==1){
                                position_taxi[mexRcv.msgc[0]]=mexRcv.msgc[1];
                                maps[mexRcv.msgc[1]].num_taxi=mexRcv.msgc[0];
                                succ++;
                                inve--; 
                            }else if(mexRcv.msgc[2]==-1){
                                    new_id=mexRcv.msgc[0];
                                    ex_pos=mexRcv.msgc[1];
                                    maps[ex_pos].num_taxi = 0;
                                    position_taxi[new_id]=0;
                                    create_new_taxi(my_mp,maps,new_id,taxi_list,key_id_shm,taxi_list_pos,position_taxi,sem_sync,ex_pos,taxi_pid);    
                            }else if(mexRcv.msgc[2]==0){
                                inve--;
                                aborti++;
                                    new_id=mexRcv.msgc[0];
                                    ex_pos=mexRcv.msgc[1];
                                    maps[ex_pos].num_taxi = 0;
                                    position_taxi[new_id]=0;
                                    create_new_taxi(my_mp,maps,new_id,taxi_list,key_id_shm,taxi_list_pos,position_taxi,sem_sync,ex_pos,taxi_pid);
                            }
                            x--;
                            time_stamp();
                        }
                    increase_resource(sem_sync,END,my_mp->num_taxi);
                    check_zero(sem_sync,END);
               
    }
}   