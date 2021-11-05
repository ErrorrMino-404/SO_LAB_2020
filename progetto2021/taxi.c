#define _POSIX_C_SOURCE 199309L
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
    struct message mexSndSO;
    struct message mexRcv;
    struct sembuf sops;
    struct timespec tim;
    int index,i,targ_y,my_y,targ_x,my_x,ex_x,ex_y;
    int dest_x,dest_y;
    struct sigaction sa;

    bzero(&sa, sizeof(sa));  
    sa.sa_handler = handle_signal; 
    sigaction(SIGINT, &sa, NULL);
    
    
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
    taxi_list[my_id].dest = -1;
     
    maps[atoi(argv[4])].num_taxi = my_id;
    /*alloco il taxi nella mappa*/
    allocate_taxi(taxi_list[my_id].x,taxi_list[my_id].y,my_id, maps, my_mp->width );
    /*Comunicazione dei taxi con il master*/
    wait_zero(my_ks->sem_sync_round, 0);
    mexSnd.type = TAXI_TO_MASTER;
    mexSndSO.type = TAXI_TO_SOURCE;
    mexRcv.type = SOURCE_TO_TAXI;
    mexSnd.msgc[0]=my_id;
  
    sops.sem_num = 0;
    sops.sem_op = -1;
    sops.sem_flg = IPC_NOWAIT;

    tim.tv_nsec=0;
    tim.tv_nsec=(long)100000000;
    /*spostamento del taxi nella mappa*/
    while(1){
        check_zero(my_ks->sem_sync_round, WAIT);
        wait_zero(my_ks->sem_sync_round, START);
        if(taxi_list[my_id].target != -1){
            my_x = maps[atoi(argv[4])].x;
            my_y = maps[atoi(argv[4])].y;
            index = taxi_list[my_id].target;
            targ_x = maps[index].x;
            targ_y = maps[index].y;
            while(index!=-1 ) {
                    if(my_x<targ_x ){
                        if(semop(maps[(my_x+1)*my_mp->width+my_y].c_sem_id,&sops,1)!=-1){

                            maps[my_x*my_mp->width+my_y].num_taxi = -1;
                            sem_relase(maps[my_x*my_mp->width+my_y].c_sem_id, 0);
                            maps[(my_x+1)*my_mp->width+my_y].num_taxi=my_id;
                            my_x +=1;
                            nanosleep(&tim, NULL);
                        }
                    }
                     if(my_x>targ_x ){
                        if(semop(maps[(my_x-1)*my_mp->width+my_y].c_sem_id,&sops,1)!=-1 ){
                            
                            maps[my_x*my_mp->width+my_y].num_taxi = -1;
                            sem_relase(maps[my_x*my_mp->width+my_y].c_sem_id, 0);
                            maps[(my_x-1)*my_mp->width+my_y].num_taxi=my_id;
                            my_x -=1;
                            nanosleep(&tim, NULL);
                        }
                    }if(my_y<targ_y ){
                        if(semop(maps[(my_y)*my_mp->width+my_y+1].c_sem_id,&sops,1)!=-1){       

                            maps[my_x*my_mp->width+my_y].num_taxi = -1;
                            sem_relase(maps[my_x*my_mp->width+my_y].c_sem_id, 0);
                            maps[(my_x)*my_mp->width+my_y+1].num_taxi=my_id;
                            my_y +=1;
                            nanosleep(&tim, NULL);
                        }
                    }
                     if(my_y>targ_y ){
                        if(semop(maps[(my_x)*my_mp->width+my_y-1].c_sem_id,&sops,1)!=-1){

                            maps[my_x*my_mp->width+my_y].num_taxi = -1;
                            sem_relase(maps[my_x*my_mp->width+my_y].c_sem_id, 0);
                            maps[(my_x)*my_mp->width+my_y-1].num_taxi=my_id;
                            my_y -=1;
                            nanosleep(&tim, NULL);
                        }
                    }if(my_x == targ_x && my_y == targ_y){
                            if(maps[(my_x)*my_mp->width+my_y].val_source != -1){
                                taxi_list[my_id].pos = (my_x)*my_mp->width+my_y;
                                /*messaggio inviato al source*/
                                mexSndSO.msgc[0] = atoi(argv[2]);
                                mexSndSO.msgc[1] = (my_x)*my_mp->width+my_y;
                                mexSndSO.msgc[2] = maps[(my_x)*my_mp->width+my_y].val_source;
                                msgsnd(my_ks->msgq_id_so, &mexSndSO, sizeof(mexSndSO)-sizeof(long),mexSndSO.type,0);
                                /*messaggio da ricevere dalla source*/
                                
                                
                                    msgrcv(my_ks->msgq_id_so, &mexRcv,sizeof(mexRcv)-sizeof(long),mexRcv.type,0);
                                    taxi_list[my_id].dest = mexRcv.msgc[1];
                                    i++;
                                
                                taxi_list[my_id].car_so = maps[(my_x)*my_mp->width+my_y].val_source;
                                mexSnd.msgc[1]= taxi_list[my_id].pos;
                                mexSnd.msgc[2] = taxi_list[my_id].dest;
                                msgsnd(my_ks->msgq_id, &mexSnd,sizeof(mexSnd)-sizeof(long),mexSnd.type,0);
                                index = -1;
                            }
                    }
                                 
            }
      
            taxi_list[my_id].x = my_x;
            taxi_list[my_id].y = my_y;  
            taxi_list[my_id].pos = my_x*my_mp->width+my_y;  
            
        }
        wait_zero(my_ks->sem_sync_round, END);
        check_zero(my_ks->sem_sync_round, WAIT);
        wait_zero(my_ks->sem_sync_round, START);
                /*ora iniziamo lo spostamento per la source verso la destinazione */
        int index2 = taxi_list[my_id].dest;
        if(taxi_list[my_id].dest != -1){
            dest_x = maps[taxi_list[my_id].dest].x;
            dest_y = maps[taxi_list[my_id].dest].y;
            my_x = maps[taxi_list[my_id].pos].x;
            my_y = maps[taxi_list[my_id].pos].y;
            while(index2!=-1){
                if(my_x>dest_x){
                        if(semop(maps[(my_x-1)*my_mp->width+my_y].c_sem_id,&sops,1)!=-1){

                            maps[my_x*my_mp->width+my_y].num_taxi = -1;
                            sem_relase(maps[my_x*my_mp->width+my_y].c_sem_id, 0);
                            maps[(my_x-1)*my_mp->width+my_y].num_taxi=my_id;
                            my_x -=1;
                            nanosleep(&tim, NULL);
                        }
                }
                if(my_x<dest_x){
                        if(semop(maps[(my_x+1)*my_mp->width+my_y].c_sem_id,&sops,1)!=-1){

                            maps[my_x*my_mp->width+my_y].num_taxi = -1;
                            sem_relase(maps[my_x*my_mp->width+my_y].c_sem_id, 0);
                            maps[(my_x+1)*my_mp->width+my_y].num_taxi=my_id;
                            my_x +=1;
                            nanosleep(&tim, NULL);
                        }
                }
                if(my_y>dest_y){
                        if(semop(maps[(my_x)*my_mp->width+my_y-1].c_sem_id,&sops,1)!=-1){

                            maps[my_x*my_mp->width+my_y].num_taxi = -1;
                            sem_relase(maps[my_x*my_mp->width+my_y].c_sem_id, 0);
                            maps[(my_x)*my_mp->width+my_y-1].num_taxi=my_id;
                            my_y -=1;
                            nanosleep(&tim, NULL);
                        }
                }
                if(my_y<dest_y){
                        if(semop(maps[(my_x)*my_mp->width+my_y+1].c_sem_id,&sops,1)!=-1){
                            maps[my_x*my_mp->width+my_y].num_taxi = -1;
                            sem_relase(maps[my_x*my_mp->width+my_y].c_sem_id, 0);
                            maps[(my_x)*my_mp->width+my_y+1].num_taxi=my_id;
                            my_y +=1;
                            nanosleep(&tim, NULL);
                        }
                }
                if(my_y==dest_y && my_x==dest_x){
                    
                    printf("raggiunto la destinazione\n");
                    taxi_list[my_id].pos = (dest_x)*my_mp->width+dest_y;
                    mexSnd.msgc[1]= taxi_list[my_id].pos;
                    msgsnd(my_ks->msgq_id_so, &mexSnd,sizeof(mexSnd)-sizeof(long),mexSnd.type,0); 
                    printf("messaggio inviato \n"); 
                    index2 = -1;          
                }
            }
        }
  

        wait_zero(my_ks->sem_sync_round, END);  
    }
}

