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
    struct timespec tim;
    int index;
    int targ_x,my_x,ex_x;
    int targ_y,my_y,ex_y;
    
    
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
    
    mexSnd.type = TAXI_TO_MASTER;
    mexSnd.msgc[0]=my_id;
  
    sops.sem_num = 0;
    sops.sem_op = -1;
    sops.sem_flg = IPC_NOWAIT;
    my_x = taxi_list[my_id].x;
    my_y = taxi_list[my_id].y;
    tim.tv_nsec=0;
    tim.tv_nsec=(long)100000000;

    /*spostamento del taxi nella mappa*/
    wait_zero(my_ks->sem_sync_round, 0);
    
    while(1){
        
        check_zero(my_ks->sem_sync_round, WAIT);
        wait_zero(my_ks->sem_sync_round, START);

        if(taxi_list[my_id].target != -1){
            
            index = taxi_list[my_id].target;
            targ_x = maps[index].x;
            targ_y = maps[index].y;
            printf("TAXI = %d POS = %d TARG = %d \n",my_id, taxi_list[my_id].pos, taxi_list[my_id].target);
            while(index!=-1 && maps[(my_x)*my_mp->width+my_y].val_source != 1 ) {
                if(my_x<targ_x){
                    if(semop(maps[(my_x+1)*my_mp->width+my_y].c_sem_id,&sops,1)!=-1 ){
                        if(maps[(my_x+1)*my_mp->width+my_y].val_source != 0 ){
                            printf("TAXI %d mando messaggio 1 \n", my_id);
                            mexSnd.msgc[1]=(my_x+1)*my_mp->width+my_y;  
                            msgsnd(my_ks->msgq_id, &mexSnd, sizeof(mexSnd)-sizeof(long), 0);  
                        }
                        maps[my_x*my_mp->width+my_y].num_taxi = -1;
                        sem_relase(maps[my_x*my_mp->width+my_y].c_sem_id, 0);
                        maps[(my_x+1)*my_mp->width+my_y].num_taxi=my_id;
                        my_x +=1;
                        nanosleep(&tim, NULL);
                    }
                    
                }else if(my_x>targ_x){
                    if(semop(maps[(my_x-1)*my_mp->width+my_y].c_sem_id,&sops,1)!=-1 ){
                        if(maps[(my_x-1)*my_mp->width+my_y].val_source != 0 ){
                            printf("TAXI %d mando messaggio 2 \n",my_id);
                            mexSnd.msgc[1]=(my_x-1)*my_mp->width+my_y;  
                            msgsnd(my_ks->msgq_id, &mexSnd, sizeof(mexSnd)-sizeof(long), 0);   
                        }
                        maps[my_x*my_mp->width+my_y].num_taxi = -1;
                        sem_relase(maps[my_x*my_mp->width+my_y].c_sem_id, 0);
                        maps[(my_x-1)*my_mp->width+my_y].num_taxi=my_id;
                        my_x -=1;
                        nanosleep(&tim, NULL);


                    }
                }
        
                if(my_y<targ_y){
                    if(semop(maps[(my_y)*my_mp->width+my_y+1].c_sem_id,&sops,1)!=-1 ){
                        if(maps[(my_x)*my_mp->width+my_y+1].val_source != 0){
                             printf("TAXI %d mando messaggio 3 \n", my_id);
                            mexSnd.msgc[1]=(my_x)*my_mp->width+my_y+1;  
                            msgsnd(my_ks->msgq_id, &mexSnd, sizeof(mexSnd)-sizeof(long), 0); 
                        }
                        maps[my_x*my_mp->width+my_y].num_taxi = -1;
                        sem_relase(maps[my_x*my_mp->width+my_y].c_sem_id, 0);
                        maps[(my_x)*my_mp->width+my_y+1].num_taxi=my_id;
                        my_y +=1;
                        nanosleep(&tim, NULL);
                   }
                }else if(my_y>targ_y){
                    if(semop(maps[(my_x)*my_mp->width+my_y-1].c_sem_id,&sops,1)!=-1 ){
                        if(maps[(my_x)*my_mp->width+my_y-1].val_source ==1){
                            printf("TAXI %d mando messaggio 4 \n", my_id);
                            mexSnd.msgc[1]=(my_x)*my_mp->width+my_y-1;  
                            msgsnd(my_ks->msgq_id, &mexSnd, sizeof(mexSnd)-sizeof(long), 0);   
                        }
                        maps[my_x*my_mp->width+my_y].num_taxi = -1;
                        sem_relase(maps[my_x*my_mp->width+my_y].c_sem_id, 0);
                        maps[(my_x)*my_mp->width+my_y-1].num_taxi=my_id;
                        my_y -=1;
                        nanosleep(&tim, NULL);
                    }
                }
                    if(my_x==ex_x && my_y==ex_y){
                        break;
                    }
                    ex_x=my_x;
                    ex_y=my_y;
            }

            taxi_list[my_id].x = my_x;
            taxi_list[my_id].y = my_y;    
        }
         
       wait_zero(my_ks->sem_sync_round, END);  
    }
    
    
}

