
#define _GNU_SOURCE
#include "taxi_lib.h"
    slot* maps;
    keys_storage* my_ks;
    maps_config* my_mp;
    int* arr_target;
    int keys_id, my_id,end,pipe_wr,pipe_rd;
    taxi_data* taxi_list;
    struct message mexSnd;
    struct message mexSndSO;
    struct message mexRcv;
    struct message mexState;
    struct sembuf sops;
    struct timespec tim;
    struct timeval timer;

void timeCheck(){
    
    struct timeval pass;
    int s,u,n;
    int mex_so;
    int j;
    gettimeofday(&pass,NULL);
    s=pass.tv_sec - timer.tv_sec;
    u=pass.tv_usec - timer.tv_usec;
    n = s * 1000000000 + (u * 1000);
    if(n>=1000000000){
        if(taxi_list[my_id].target!=-1 && taxi_list[my_id].dest==-1){
            printf("no source %d pid=%d\n",my_id,getpid());
            sem_relase(maps[taxi_list[my_id].pos].c_sem_id, 0);
            j=taxi_list[my_id].target;
            mex_so=maps[j].val_source;
            mexSndSO.msgc[0]=my_id;
            mexSndSO.msgc[2]=-1;
            msgsnd(my_ks->msgq_id_ts,&mexSndSO,sizeof(mexSndSO)-sizeof(long),0);
            /*messaggio da taxi a source*/
            mexSnd.msgc[0]=my_id;
            mexSnd.msgc[1]=taxi_list[my_id].pos;
            mexSnd.msgc[2]=-1;
            msgsnd(my_ks->msgq_id,&mexSnd,sizeof(mexSnd)-sizeof(long),0);
            /*segnale da inviare*/
            sleep(1);
            exit(0);

        }else if(taxi_list[my_id].dest!=-1 && taxi_list[my_id].target!=-1){
            sem_relase(maps[taxi_list[my_id].pos].c_sem_id, 0);
            mexSndSO.msgc[0]=taxi_list[my_id].car_so;
            mexSndSO.msgc[1]=taxi_list[my_id].pos;
            mexSndSO.msgc[2]=-1;
            printf("nuova posizione %d \n",taxi_list[my_id].pos);
            msgsnd(my_ks->msgq_id_ns,&mexSndSO,sizeof(mexSndSO)-sizeof(long),0);
            mexSnd.msgc[0]=my_id;
            mexSnd.msgc[1]=taxi_list[my_id].pos;
            mexSnd.msgc[2]=0;
            msgsnd(my_ks->msgq_id,&mexSnd,sizeof(mexSnd)-sizeof(long),0);
            /*segnale da inviare*/
            sleep(1);
            exit(0);

        }else{
            taxi_list[my_id].my_pid=-1;
            sem_relase(maps[taxi_list[my_id].pos].c_sem_id, 0);
            maps[taxi_list[my_id].pos].num_taxi=0;
            wait_zero(my_ks->sem_sync_round, END); 
            sleep(1);
            exit(0);
    }
    }else {
        return;
    }
}
int main(int argc,char *argv[]){
    int index,i,targ_y,my_y,targ_x,my_x,ex_x,ex_y,sem;
    int dest_x,dest_y,msec,trigger,index2;
    struct sigaction sa;
    
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
    taxi_list[my_id].move = 0;
    taxi_list[my_id].exp_so = 0;
    maps[atoi(argv[4])].num_taxi = my_id;
    pipe_rd=my_ks->rd;
    pipe_wr=my_ks->wr;
    /*alloco il taxi nella mappa*/
    allocate_taxi(taxi_list[my_id].x,taxi_list[my_id].y,my_id, maps, my_mp->width );
   
    /*Comunicazione dei taxi con il master*/
    mexSnd.type = TAXI_TO_MASTER;
    mexSndSO.type = TAXI_TO_SOURCE;
    mexRcv.type = SOURCE_TO_TAXI;

    sops.sem_num = 0;
    sops.sem_op = -1;
    sops.sem_flg = IPC_NOWAIT;

    tim.tv_nsec=0;
    tim.tv_nsec=(long)1000000;
   
    gettimeofday(&timer,NULL);
    /*spostamento del taxi nella mappa*/
    while(1){

        /*alcuni processi taxi non raggiungono questo punto, i due processi che hanno il target*/
        check_zero(my_ks->sem_sync_round, WAIT);
        wait_zero(my_ks->sem_sync_round, START);
        timeCheck();
        
        if(taxi_list[my_id].target != -1){
            printf("ho il target PID=%d TAXI=%d\n",taxi_list[my_id].my_pid,my_id);
            my_x = maps[atoi(argv[4])].x;
            my_y = maps[atoi(argv[4])].y;
            index = taxi_list[my_id].target;
            targ_x = maps[index].x;
            targ_y = maps[index].y;
            while(index!=-1) { 
                timeCheck();
                    if(my_x<targ_x ){
                        if(semop(maps[(my_x+1)*my_mp->width+my_y].c_sem_id,&sops,1)!=-1){
                            maps[my_x*my_mp->width+my_y].num_taxi = 0;      /*nessun taxi sulla casella*/
                            taxi_list[my_id].move +=1;                      /*movimento del taxi*/
                            maps[my_x*my_mp->width+my_y].top_cells += 1;    /*passato un taxi sopra*/
                            sem_relase(maps[my_x*my_mp->width+my_y].c_sem_id, 0);
                            maps[(my_x+1)*my_mp->width+my_y].num_taxi=my_id;
                            my_x +=1;
                            taxi_list[my_id].pos= my_x*my_mp->width+my_y;
                            nanosleep(&tim, NULL);
                            gettimeofday(&timer,NULL);
                        }
                    }if(my_x>targ_x ){
                        if(semop(maps[(my_x-1)*my_mp->width+my_y].c_sem_id,&sops,1)!=-1){
                            maps[my_x*my_mp->width+my_y].top_cells += 1;
                            taxi_list[my_id].move +=1;
                            maps[my_x*my_mp->width+my_y].num_taxi = 0;
                            sem_relase(maps[my_x*my_mp->width+my_y].c_sem_id, 0);
                            maps[(my_x-1)*my_mp->width+my_y].num_taxi=my_id;
                            my_x -=1;
                            taxi_list[my_id].pos= my_x*my_mp->width+my_y;
                            nanosleep(&tim, NULL);
                            gettimeofday(&timer,NULL);
                            
                        }
                    }if(my_y<targ_y ){
                        if(semop(maps[(my_x)*my_mp->width+my_y+1].c_sem_id,&sops,1)!=-1){ 
                            maps[my_x*my_mp->width+my_y].top_cells += 1;
                            taxi_list[my_id].move +=1;
                            maps[my_x*my_mp->width+my_y].num_taxi = 0;
                            sem_relase(maps[my_x*my_mp->width+my_y].c_sem_id, 0);
                            maps[(my_x)*my_mp->width+my_y+1].num_taxi=my_id;
                            my_y +=1;
                            taxi_list[my_id].pos= my_x*my_mp->width+my_y;
                            nanosleep(&tim, NULL);
                            gettimeofday(&timer,NULL);
                            
                        }
                    }if(my_y>targ_y ){
                        if(semop(maps[(my_x)*my_mp->width+my_y-1].c_sem_id,&sops,1)!=-1){
                            maps[my_x*my_mp->width+my_y].top_cells += 1;
                            taxi_list[my_id].move +=1;
                            maps[my_x*my_mp->width+my_y].num_taxi = 0;
                            sem_relase(maps[my_x*my_mp->width+my_y].c_sem_id, 0);
                            maps[(my_x)*my_mp->width+my_y-1].num_taxi=my_id;
                            my_y -=1;
                            taxi_list[my_id].pos= my_x*my_mp->width+my_y;
                            nanosleep(&tim, NULL);
                            gettimeofday(&timer,NULL);
                        }
                    }
                    if(my_x == targ_x && my_y == targ_y){
                            if(maps[(my_x)*my_mp->width+my_y].val_source != -1){
                                /*taxi invia un messaggio alla source*/
                                mexSndSO.msgc[0]=my_id;
                                mexSndSO.msgc[1]= maps[(my_x)*my_mp->width+my_y].val_source;
                                mexSndSO.msgc[2]=1;
                                msgsnd(my_ks->msgq_id_ts,&mexSndSO,sizeof(mexSndSO)-sizeof(long),0);
                                /*aspetto risposta dalla source*/
                                if((msgrcv(my_ks->msgq_id_st,&mexRcv,sizeof(mexRcv)-sizeof(long),mexRcv.type,0)==-1)){
                                    TEST_ERROR;
                                }
                                    taxi_list[my_id].exp_so+=1;
                                    taxi_list[my_id].dest=mexRcv.msgc[0];
                                    taxi_list[my_id].car_so=maps[(my_x)*my_mp->width+my_y].val_source;
                                    maps[my_x*my_mp->width+my_y].val_source = -1;
                                    index=-1;
                            }    
                    }  
                    taxi_list[my_id].pos= my_x*my_mp->width+my_y; 
                    taxi_list[my_id].y=my_y;
                    taxi_list[my_id].x=my_x;            
            }   
            /*se ha ricevuto un target vuol dire che può raggiungere la destinazione*/
        }
        gettimeofday(&timer,NULL);
            if(taxi_list[my_id].dest != -1){
                my_x=taxi_list[my_id].x;
                my_y=taxi_list[my_id].y;
                dest_x = maps[taxi_list[my_id].dest].x;
                dest_y = maps[taxi_list[my_id].dest].y;
                index2 = dest_x*my_mp->width+dest_y;
                while(index2!=-1){
                    timeCheck();
                    if(my_x>dest_x){
                            if(semop(maps[(my_x-1)*my_mp->width+my_y].c_sem_id,&sops,1)!=-1){
                                maps[my_x*my_mp->width+my_y].top_cells += 1;
                                taxi_list[my_id].move +=1;
                                maps[my_x*my_mp->width+my_y].num_taxi = 0;
                                sem_relase(maps[my_x*my_mp->width+my_y].c_sem_id, 0);
                                maps[(my_x-1)*my_mp->width+my_y].num_taxi=my_id;
                                my_x -=1;
                                taxi_list[my_id].pos= my_x*my_mp->width+my_y;
                                nanosleep(&tim, NULL);
                                gettimeofday(&timer,NULL);
                            }
                    }
                    if(my_x<dest_x){
                            if(semop(maps[(my_x+1)*my_mp->width+my_y].c_sem_id,&sops,1)!=-1){
                                maps[my_x*my_mp->width+my_y].top_cells += 1;
                                taxi_list[my_id].move +=1;
                                maps[my_x*my_mp->width+my_y].num_taxi = 0;
                                sem_relase(maps[my_x*my_mp->width+my_y].c_sem_id, 0);
                                maps[(my_x+1)*my_mp->width+my_y].num_taxi=my_id;
                                my_x +=1;
                                taxi_list[my_id].pos= my_x*my_mp->width+my_y;
                                nanosleep(&tim, NULL);
                                 gettimeofday(&timer,NULL);
                            }
                    }
                    if(my_y>dest_y){
                            if(semop(maps[(my_x)*my_mp->width+my_y-1].c_sem_id,&sops,1)!=-1){
                                maps[my_x*my_mp->width+my_y].top_cells += 1;
                                taxi_list[my_id].move +=1;
                                maps[my_x*my_mp->width+my_y].num_taxi = 0;
                                sem_relase(maps[my_x*my_mp->width+my_y].c_sem_id, 0);
                                maps[(my_x)*my_mp->width+my_y-1].num_taxi=my_id;
                                my_y -=1;
                                taxi_list[my_id].pos= my_x*my_mp->width+my_y;
                                nanosleep(&tim, NULL);
                                 gettimeofday(&timer,NULL);
                            }
                    }
                    if(my_y<dest_y){
                            if(semop(maps[(my_x)*my_mp->width+my_y+1].c_sem_id,&sops,1)!=-1){
                                maps[my_x*my_mp->width+my_y].top_cells += 1;
                                taxi_list[my_id].move +=1;
                                maps[my_x*my_mp->width+my_y].num_taxi = 0;
                                sem_relase(maps[my_x*my_mp->width+my_y].c_sem_id, 0);
                                maps[(my_x)*my_mp->width+my_y+1].num_taxi=my_id;
                                my_y +=1;
                                taxi_list[my_id].pos= my_x*my_mp->width+my_y;
                                nanosleep(&tim, NULL);
                                gettimeofday(&timer,NULL);
                            }
                    }
                    if(my_y==dest_y && my_x==dest_x){
                        
                        /*messaggio inviato al master*/
                        mexSnd.msgc[0]=my_id;
                        mexSnd.msgc[1]=my_x*my_mp->width+my_y;
                        mexSnd.msgc[2]=1;
                        msgsnd(my_ks->msgq_id,&mexSnd,sizeof(mexSnd)-sizeof(long),0);
                        index2=-1;
                        gettimeofday(&timer,NULL);
                    } 
                    taxi_list[my_id].pos=my_x*my_mp->width+my_y;
                    taxi_list[my_id].y=my_y;
                    taxi_list[my_id].x=my_x; 
                }

            }
            /*possibile problema*/
        wait_zero(my_ks->sem_sync_round, END); 

    }
}

