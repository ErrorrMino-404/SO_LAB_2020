#define _GNU_SOURCE
#include "master_lib.h"


maps_config *my_mp;
slot* maps;
int gc_id_shm,sem_set_tx, mp_id_shm, ho_id_shm, so_id_shm,tx_id_shm,key_id_shm,sem_sync_round;
int msgq_id_sm, msgq_id,msgq_id_so;
int  succ,aborti,inve;/*variabili da stampare*/
int *position_taxi, *position_so, *array_id_taxi;
pid_t*taxi_pid,*so_pid;
keys_storage *my_ks;
taxi_data* taxi_list;
source_data* source_list;
clock_t start,stop;


void handle_signal(int signum){
        if(signum==SIGUSR1){
            int i,status;
            for(i=1;i<my_mp->num_taxi+1;i++){
                if(taxi_list[i].my_pid==-1){
                    status =i;
                } 
            }
                /*fine gioco causa SIGALRM*/
        } else if(signum == SIGINT) {
                /*fine gioco SIGINT*/
                        int i, j;

            stop=clock();
            printf("GAME ENDED\nPrinting chessboard and statistics.\n\n");
            for(i=1; i<my_mp->num_taxi+1; i++){
                    kill(taxi_list[i].my_pid, SIGTERM);  
            }
            for(i=0; i<my_mp->source; i++){
                kill(source_list[i].my_pid, SIGTERM);
            }
            print_metrics(my_mp, array_id_taxi);
            /*rimozione semafori e code*/
            semctl(sem_sync_round, 0, IPC_RMID);
            msgctl(msgq_id, IPC_RMID, 0);
            msgctl(msgq_id_so, IPC_RMID, 0);
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
                printf("The game ended because the SIGINT signal was received.\n");
                exit(EXIT_FAILURE);
        }
}
void handle_taxi(){
    printf("BELLA PER TUTTI %d\n",getpid());
    
}
int main(){
    
    int num_ho,top_taxi,taxi_succes;
    int* holes;
    int pos_source[3];
    int taxi_list_pos,source_list_pos, num_so,ex_id,ex_pos;
    int i,j,x;
    struct message mexSnd,mexRcvSM,mexRcv;
    struct sigaction action;
    sigset_t zeromask;
    char* args_tx[6] = {TAXI};
    char* args_so[6] = {SOURCE};

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
    /*coda di messaggio tra taxi e master*/
    if((msgq_id = msgget(IPC_PRIVATE,IPC_CREAT|0666))==-1){
        TEST_ERROR;
    }
    /*coda di messaggi tra taxi e source*/
    if((msgq_id_so = msgget(IPC_PRIVATE,IPC_CREAT|0666))==-1){
        TEST_ERROR;
    }
    /*coda di messaggi tra source e master*/
    if((msgq_id_sm = msgget(IPC_PRIVATE,IPC_CREAT|0666))==-1){
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
    
    my_ks = fill_storage_shm(key_id_shm, gc_id_shm, mp_id_shm, msgq_id,msgq_id_so,msgq_id_sm,sem_sync_round,sem_set_tx);
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
    increase_resource(sem_sync_round, 0, my_mp->num_taxi+1);

    signal(SIGUSR1,handle_signal);
    i = 0;
    j = 0;
    num_so = 0;
    mexRcv.type = TAXI_TO_MASTER;
    mexRcvSM.type = SOURCE_TO_MASTER;
    ex_id = -1;
    succ = 0; /*numero di source andate a buonfine*/
    aborti = 0; /*numero di source prese da un taxi, ma che non hanno raggiunto la destinazione*/
    inve = 0; /*richieste source non ancora raggiunte*/

    while(1){
        /*deve aspettare il messaggio dalle source*/
        top_taxi = calculate_top_taxi(taxi_list, my_mp->num_taxi+1);
        taxi_succes = calculate_taxi_succes(taxi_list, my_mp->num_taxi+1);
        printf("sono dentro 1\n");
        x = num_so;
        while(x < 2 ){
            if((msgrcv(my_ks->msgq_id_sm, &mexRcvSM, sizeof(mexRcvSM)-sizeof(long),mexRcvSM.type,0))==-1){
                TEST_ERROR;
            }
            pos_source[x] = mexRcvSM.msgc[0];
           /* printf("ho ricevuto il messaggio %d \n", pos_source[x]);*/
            x++;
            
        }
        inve = x;
        i =x;
        pos_source[x]=-1;
        j=0;
        num_so = 0;
        compute_targets(taxi_list,  my_mp->num_taxi,maps,pos_source);
       /* print_maps(maps,my_mp,pos_source, top_taxi,taxi_succes,succ,aborti,inve);*/
        increase_resource(sem_sync_round,START,my_mp->num_taxi);
        sem_reserve(sem_sync_round, WAIT);
        check_zero(sem_sync_round, START);
        sem_relase(sem_sync_round, WAIT);

        /*aspettare messaggio di aver preso la source*/
        while(i>0){
            if((msgrcv(my_ks->msgq_id, &mexRcv, sizeof(mexRcv)-sizeof(long), mexRcv.type,0))==-1){
                TEST_ERROR;
            }
            position_taxi[mexRcv.msgc[0]] = mexRcv.msgc[1];
            if(mexRcv.msgc[2] == -1){
                /*devo eliminare il blocco che rimane fermo per troppo*/
                ex_id = mexRcv.msgc[0];
                ex_pos = mexRcv.msgc[1];
                    if(ex_id != -1){
                        printf("non sono arrivato alla source \n");
                        kill(taxi_list[ex_id].my_pid, SIGKILL);                                    
                            int sem = 0;
                            int my_x,my_y,ok;
                            args_tx[1] = integer_to_string_arg(key_id_shm);
                            args_tx[3] = integer_to_string_arg(taxi_list_pos);
                                        
                            srand(time(NULL));
                                while(sem<1){
                                    my_x = rand()%my_mp->height;
                                    my_y = rand()%my_mp->width;
                                    sem=1;
                                    for(ok = 1; ok <my_mp->num_taxi+1;ok++){
                                        if(position_taxi[ok]==my_x*my_mp->width+my_y || maps[my_x*my_mp->width+my_y].val_holes!=0
                                            || my_x*my_mp->width+my_y==ex_pos){
                                                sem = 0;
                                        }
                                    }
                                }
                                position_taxi[ex_id] = my_x*my_mp->width+my_y;
                                maps[my_x*my_mp->width+my_y].num_taxi = ex_id;
                                array_id_taxi[ex_id]= ex_id;

                                    switch (taxi_pid[ex_id]=fork()) {
                                        case -1:
                                            TEST_ERROR
                                        break;
                                        case 0:
                                            args_tx[2]= integer_to_string_arg(ex_id); /*id del taxi*/
                                            args_tx[4]=integer_to_string_arg(position_taxi[ex_id]); /*posizione del taxi*/
                                            execve(TAXI,args_tx,NULL);
                                            TEST_ERROR;
                                        break;
                                        default:
                                        break;
                                        }
                                ex_pos = -1;
                                ex_id = -1;    
                                increase_resource(sem_sync_round,START,1);
                                sem_reserve(sem_sync_round, WAIT);
                                check_zero(sem_sync_round, START);
                                sem_relase(sem_sync_round, WAIT);   
                    }
                x--;
            }else{
                inve--;
                x--;
                pos_source[x]=-1;
                taxi_list[mexRcv.msgc[0]].exp_so +=1;
                maps[mexRcv.msgc[1]].num_taxi = mexRcv.msgc[0];
                maps[mexRcv.msgc[1]].val_source = -1;
                j++;
            }
            i--;
        }
        for(x=0;x<2;x++){
            if(pos_source[x]!=-1){
                pos_source[num_so] = pos_source[x];
                num_so++;
            }
        }
        increase_resource(sem_sync_round,END,my_mp->num_taxi);
        increase_resource(sem_sync_round,START,my_mp->num_taxi);
        sem_reserve(sem_sync_round, WAIT);
        check_zero(sem_sync_round, START);
        sem_relase(sem_sync_round, WAIT);
        
        while(j>0){
            if((msgrcv(my_ks->msgq_id, &mexRcv, sizeof(mexRcv)-sizeof(long), mexRcv.type,0))==-1){
                TEST_ERROR;
            }
            if(mexRcv.msgc[2] == -1){
                
                aborti++;
                printf("non sono arrivato taxi=%d source=%d pos=%d \n",mexRcv.msgc[0], taxi_list[mexRcv.msgc[0]].car_so,mexRcv.msgc[1]);
                maps[mexRcv.msgc[1]].num_taxi = 0;
                maps[mexRcv.msgc[1]].val_source = taxi_list[mexRcv.msgc[0]].car_so;
                source_list[ taxi_list[mexRcv.msgc[0]].car_so].origin = mexRcv.msgc[1];
                pos_source[num_so] = mexRcv.msgc[1];     /*non avendo raggiunto la destinazione la richiesta rimane sul posto*/
                position_so[taxi_list[mexRcv.msgc[0]].car_so] = mexRcv.msgc[1]; /*aggiorno la posizione della source in position_so generale*/
                ex_id = mexRcv.msgc[0];
                ex_pos = mexRcv.msgc[1];
                position_taxi[mexRcv.msgc[0]] = -1; /*siccome devo eliminare Taxi aggiorno la sua posizione nell'array*/
                /*uccido il taxi e ne creo uno nuovo con id e posizione differente*/
                    if(ex_id != -1){
                        kill(taxi_list[ex_id].my_pid, SIGKILL);                                    
                            int sem = 0;
                            int my_x,my_y,ok;
                            args_tx[1] = integer_to_string_arg(key_id_shm);
                            args_tx[3] = integer_to_string_arg(taxi_list_pos);
                            srand(time(NULL));
                                while(sem<1){
                                    my_x = rand()%my_mp->height;
                                    my_y = rand()%my_mp->width;
                                    sem=1;
                                    for(ok = 1; ok <my_mp->num_taxi+1;ok++){
                                        if(position_taxi[ok]==my_x*my_mp->width+my_y || maps[my_x*my_mp->width+my_y].val_holes!=0
                                            || my_x*my_mp->width+my_y==ex_pos){
                                                sem = 0;
                                        }
                                    }
                                }
                                position_taxi[ex_id] = my_x*my_mp->width+my_y;
                                maps[my_x*my_mp->width+my_y].num_taxi = ex_id;
                                array_id_taxi[ex_id]= ex_id;
                                    switch (taxi_pid[ex_id]=fork()) {
                                        case -1:
                                            TEST_ERROR
                                        break;
                                        case 0:
                                            args_tx[2]= integer_to_string_arg(ex_id); /*id del taxi*/
                                            args_tx[4]=integer_to_string_arg(position_taxi[ex_id]); /*posizione del taxi*/
                                            execve(TAXI,args_tx,NULL);
                                            TEST_ERROR;
                                        break;
                                        default:
                                        break;
                                        }
                                ex_pos = -1;
                                ex_id = -1;

                                increase_resource(sem_sync_round,START,1);
                                sem_reserve(sem_sync_round, WAIT);
                                check_zero(sem_sync_round, START);
                                sem_relase(sem_sync_round, WAIT);   
                    }  
                num_so++;
            }else if(mexRcv.msgc[2] == 1){
                kill(source_list[taxi_list[mexRcv.msgc[0]].car_so].my_pid,SIGSTOP);
                succ++;
                position_taxi[mexRcv.msgc[0]] = mexRcv.msgc[1];
                maps[mexRcv.msgc[1]].num_taxi = mexRcv.msgc[0];
                
            }
            j --;
        }

        printf("sono arrivato alla fine \n");
       /* print_maps(maps,my_mp,pos_source, top_taxi,taxi_succes,succ,aborti,inve); */
        increase_resource(sem_sync_round,END,my_mp->num_taxi);
        check_zero(sem_sync_round,END);
    
    
    }
}   