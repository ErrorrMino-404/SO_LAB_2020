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
    int index,i,targ_y,my_y,targ_x,my_x,ex_x,ex_y,sem;
    int dest_x,dest_y,msec,trigger,index2;
    struct sigaction sa;
    unsigned int sec;
    

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
    taxi_list[my_id].target = -1;
    maps[atoi(argv[4])].num_taxi = my_id;
    /*alloco il taxi nella mappa*/
    allocate_taxi(taxi_list[my_id].x,taxi_list[my_id].y,my_id, maps, my_mp->width );
    /*Comunicazione dei taxi con il master*/

    mexSnd.type = TAXI_TO_MASTER;
    mexSndSO.type = TAXI_TO_SOURCE;
    mexRcv.type = SOURCE_TO_TAXI;
    mexSnd.msgc[0]=my_id;
  
    sops.sem_num = 0;
    sops.sem_op = -1;
    sops.sem_flg = IPC_NOWAIT;

    tim.tv_nsec=0;
    tim.tv_nsec=(long)1000000;
    sleep(2);

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
                            maps[my_x*my_mp->width+my_y].num_taxi = 0;
                            sem_relase(maps[my_x*my_mp->width+my_y].c_sem_id, 0);
                            maps[(my_x+1)*my_mp->width+my_y].num_taxi=my_id;
                            my_x +=1;
                            nanosleep(&tim, NULL);
                        }
                    }if(my_x>targ_x ){
                        if(semop(maps[(my_x-1)*my_mp->width+my_y].c_sem_id,&sops,1)!=-1){
                            
                            maps[my_x*my_mp->width+my_y].num_taxi = 0;
                            sem_relase(maps[my_x*my_mp->width+my_y].c_sem_id, 0);
                            maps[(my_x-1)*my_mp->width+my_y].num_taxi=my_id;
                            my_x -=1;
                            nanosleep(&tim, NULL);
                        }
                    }if(my_y<targ_y ){
                        if(semop(maps[(my_x)*my_mp->width+my_y+1].c_sem_id,&sops,1)!=-1){ 
                            maps[my_x*my_mp->width+my_y].num_taxi = 0;
                            sem_relase(maps[my_x*my_mp->width+my_y].c_sem_id, 0);
                            maps[(my_x)*my_mp->width+my_y+1].num_taxi=my_id;
                            my_y +=1;
                            nanosleep(&tim, NULL);
                        }
                    }if(my_y>targ_y ){
                        if(semop(maps[(my_x)*my_mp->width+my_y-1].c_sem_id,&sops,1)!=-1){
                            
                            maps[my_x*my_mp->width+my_y].num_taxi = 0;
                            sem_relase(maps[my_x*my_mp->width+my_y].c_sem_id, 0);
                            maps[(my_x)*my_mp->width+my_y-1].num_taxi=my_id;
                            my_y -=1;
                            nanosleep(&tim, NULL);
                        }
                    }
                    if(my_x == targ_x && my_y == targ_y){
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
                                    
                                taxi_list[my_id].car_so = maps[(my_x)*my_mp->width+my_y].val_source;
                                mexSnd.msgc[1]= (my_x)*my_mp->width+my_y;
                                mexSnd.msgc[2] = 1; /*andato a buonfine*/
                                msgsnd(my_ks->msgq_id, &mexSnd,sizeof(mexSnd)-sizeof(long),mexSnd.type,0);
                                index = -1;
                            }
                            
                    }if(my_x == ex_x && my_y == ex_y ){
                        maps[my_x*my_mp->width+my_y].num_taxi = 0;
                        /*nel caso in cui non venga raggiunta la posizione si randomizza nella mappa*/
                        mexSnd.msgc[1] = my_x*my_mp->width+my_y;  /*posizione attuale del taxi*/
                        mexSnd.msgc[2] = -1; /*non andato a buon fine*/
                        sem_relase(maps[my_x*my_mp->width+my_y].c_sem_id, 0);
                        msgsnd(my_ks->msgq_id, &mexSnd,sizeof(mexSnd)-sizeof(long),mexSnd.type,0);    
                        break;
                    }
                    ex_x = my_x;
                    ex_y = my_y;
                              
            }
            
            
        }
        wait_zero(my_ks->sem_sync_round, END);
        check_zero(my_ks->sem_sync_round, WAIT);
        wait_zero(my_ks->sem_sync_round, START);
        
        if(taxi_list[my_id].dest != -1){
            dest_x = maps[taxi_list[my_id].dest].x;
            dest_y = maps[taxi_list[my_id].dest].y;
            index2 = dest_x*my_mp->width+dest_y;
            my_x = taxi_list[my_id].x;
            my_y = taxi_list[my_id].y;
            printf("dest_x = %d dest_y=%d destin =%d \n",dest_x,dest_y,dest_x*my_mp->width+dest_y);
            while(index2!=-1){
                if(my_x>dest_x){
                        if(semop(maps[(my_x-1)*my_mp->width+my_y].c_sem_id,&sops,1)!=-1){

                            maps[my_x*my_mp->width+my_y].num_taxi = 0;
                            sem_relase(maps[my_x*my_mp->width+my_y].c_sem_id, 0);
                            maps[(my_x-1)*my_mp->width+my_y].num_taxi=my_id;
                            my_x -=1;
                            nanosleep(&tim, NULL);
                        }
                }
                if(my_x<dest_x){
                        if(semop(maps[(my_x+1)*my_mp->width+my_y].c_sem_id,&sops,1)!=-1){
                            maps[my_x*my_mp->width+my_y].num_taxi = 0;
                            sem_relase(maps[my_x*my_mp->width+my_y].c_sem_id, 0);
                            maps[(my_x+1)*my_mp->width+my_y].num_taxi=my_id;
                            my_x +=1;
                            nanosleep(&tim, NULL);
                        }
                }
                if(my_y>dest_y){
                        if(semop(maps[(my_x)*my_mp->width+my_y-1].c_sem_id,&sops,1)!=-1){

                            maps[my_x*my_mp->width+my_y].num_taxi = 0;
                            sem_relase(maps[my_x*my_mp->width+my_y].c_sem_id, 0);
                            maps[(my_x)*my_mp->width+my_y-1].num_taxi=my_id;
                            my_y -=1;
                            nanosleep(&tim, NULL);
                        }
                }
                if(my_y<dest_y){
                        if(semop(maps[(my_x)*my_mp->width+my_y+1].c_sem_id,&sops,1)!=-1){
                            maps[my_x*my_mp->width+my_y].num_taxi = 0;
                            sem_relase(maps[my_x*my_mp->width+my_y].c_sem_id, 0);
                            maps[(my_x)*my_mp->width+my_y+1].num_taxi=my_id;
                            my_y +=1;
                            nanosleep(&tim, NULL);
                        }
                }
                if(my_y==dest_y && my_x==dest_x){
                    
                        taxi_list[my_id].pos = my_x*my_mp->width+my_y;
                        taxi_list[my_id].dest = -1;
                        mexSnd.msgc[0] = my_id;
                        mexSnd.msgc[1] = my_x*my_mp->width+my_y;
                        mexSnd.msgc[2] = 1;
                        msgsnd(my_ks->msgq_id, &mexSnd, sizeof(mexSnd)-sizeof(long),mexSnd.type,0);
                        index2 = -1;
                    
                }
                if(my_x == ex_x && my_y == ex_y){
                    sem_relase(maps[my_x*my_mp->width+my_y].c_sem_id, 0);
                    mexSnd.msgc[0] = my_id;
                    mexRcv.msgc[1] = my_x*my_mp->width+my_y;  
                    mexSnd.msgc[2] = -1;
                    msgsnd(my_ks->msgq_id, &mexSnd, sizeof(mexSnd)-sizeof(long),mexSnd.type,0);
                    break;

                }
                    ex_x = my_x;
                    ex_y = my_y;   
  
            }
           

        }
    wait_zero(my_ks->sem_sync_round, END);  
        
    }
}

