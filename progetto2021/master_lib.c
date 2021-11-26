#include "master_lib.h"
#include "config.h"
#include "sem_lib.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>




keys_storage* fill_storage_shm (int idm, int idc, int ids, int idq,int idqso,int idsqsm,int idsqds,int idsqns,int idsqend,int state,int idsemr){
    keys_storage* new_s;
    if((new_s=shmat(idm,NULL,0))==((void*)-1)){
        TEST_ERROR;
    }
    new_s->conf_id = idc;
    new_s->maps_id = ids;
    new_s->msgq_id = idq;
    new_s->ks_shm_id = idm;
    new_s->msgq_id_ts = idqso;
    new_s->msgq_id_sm = idsqsm;
    new_s->msgq_id_st = idsqds;
    new_s->msgq_id_ns = idsqns;
    new_s->msgq_id_end = idsqend;
    new_s->sem_sync_round = idsemr;
    new_s->state = state;
    return new_s;
}

int get_rand_so(int min, int max){
    srand(time(NULL));
    return min==max ? min :(rand()% (max-min)) + min;
}

void randomize_holes( int ho_num, maps_config* my_mp,slot* maps){
    int my_arr[ho_num];
    int i,j,x,y,ok;
    int num_r, num_sem, tmp_index;
    
    srand(time(NULL));
    i = 0;
    while(i<ho_num){
        x = rand()%(my_mp->height);
        y = rand()%(my_mp->width);
        tmp_index = x*my_mp->width+y;
        if(semctl(maps[tmp_index].c_sem_id,0,GETVAL)){
            ok=1;
            for(j = 0; j<i;j++){
                if(my_arr[j]==tmp_index||my_arr[j]==tmp_index+my_mp->width||
                my_arr[j]==tmp_index+my_mp->width+1||
                my_arr[j]==tmp_index+my_mp->width-1||
                my_arr[j]==tmp_index-my_mp->width||
                my_arr[j]==tmp_index-my_mp->width+1||
                my_arr[j]==tmp_index-my_mp->width-1||
                my_arr[j]==tmp_index+1||my_arr[j]==tmp_index-1){
                    ok= 0;
                }
            }
            if(ok==1){
                init_sem_to_val(maps[tmp_index].c_sem_id,0 , 0);
                my_arr[i]=tmp_index;
                maps[tmp_index].val_holes = 1; /*la holes è attiva  */  
                i++;
            }
        }
    }


}

char* integer_to_string_arg(int x){
    char *sup_x;
    sup_x = malloc(3*sizeof(x)+1);
    sprintf(sup_x, "%d",x);
    return sup_x;
}

int* randomize_coordinate_taxi (taxi_data* taxi_list,slot* maps, maps_config* my_mp,int tx_num){
    int i, ok, sem, x, y,j;
    int* my_arr;
    printf("sono dentro \n");
    if((my_arr = (int*)shmat(tx_num,NULL,0))==(void*) -1 ){
        TEST_ERROR
    }
    srand(time(NULL));
    i =1;
    while(i<my_mp->num_taxi+1){
        x = rand()%(my_mp->height);
        y = rand()%(my_mp->width);
        sem = x*my_mp->width+y;
        if(semctl(maps[sem].c_sem_id,0,GETVAL) && maps[sem].val_holes==0){
            ok=1;
            for( j=0; j<i; j++){
                if(my_arr[j]==sem ){
                    ok=0;
                }
            }
            if(ok==1){
                sem_reserve(maps[x*my_mp->width+y].c_sem_id,0);
                my_arr[i]=sem;
                
                maps[sem].num_taxi = i;
                /*do la posizione sulla mappa*/
                taxi_list[i].x = x;
                taxi_list[i].y = y;
                i++;
            }
        }  

    }
    printf("esco \n");
    return my_arr;
}
int * randomize_coordinate_source (source_data* list_source, slot* maps, maps_config* my_mp,int so_num){
    int i, ok, sem, x, y,j;
    int* my_arr_so;
    if((my_arr_so = (int*)shmat(so_num,NULL,0))==(void*) -1 ){
        TEST_ERROR;
    }
    srand(time(NULL));
    i = 0;
    while(i<my_mp->source){
        
        x = rand()%(my_mp->height);
        y = rand()%(my_mp->width);
        sem = x*my_mp->width+y;
        if(maps[sem].val_holes == 0){
            ok = 1;
            for(j =0; j<i; j++){
                if(my_arr_so[j]==sem){
                    ok =0;
                }
            }
        
            if(ok==1){
                my_arr_so[i] = sem;
                list_source[i].x=x;
                list_source[i].y=y;
                maps[sem].val_source = i;
                i++;
            }
        }
    }
    my_arr_so[my_mp->source] = -1; 
    return my_arr_so;
}
void compute_targets(taxi_data* taxi, int num_taxi, slot* maps,int* position_so){
    int i,j,x,y,k,h,best,dist,num_act;

    num_act=(int)log(num_taxi);
    
    for(i=1; i<num_taxi+1;i++){
        taxi[i].target = -1;
        taxi[i].dest = -1;
    } 
    
    for(i=0;position_so[i]!=-1 ;i++){
                best=__INT_MAX__;
                j=__INT_MAX__;
                for(k=1;k<num_taxi+1;k++){
                    if(taxi[k].target==-1&&taxi[k].my_pid!=-1){
                        x=taxi[k].x;
                        y=taxi[k].y;
                        dist=abs(maps[position_so[i]].x-x)+abs(maps[position_so[i]].y-y);
                        if( dist<best){
                            best=dist;
                            j=k;
                        }
                    }
                }
                if(j!=__INT_MAX__){
                    taxi[j].target=position_so[i];
                }
        }
        for(i=0;position_so[i]!=-1 ;i++){
                for(h=0;h<num_act;h++){
                        best=__INT_MAX__;
                        j=__INT_MAX__;
                        for(k=1;k<num_taxi+1;k++){
                            if(taxi[i].target!=-1&&taxi[k].my_pid!=-1){
                                x=taxi[k].x;
                                y=taxi[k].y;
                                dist=abs(maps[position_so[i]].x-x)+abs(maps[position_so[i]].y-y);
                                if(dist<best){
                                    best=dist;
                                    j=k;
                                }
                            }
                        } 
                        if(j!=__INT_MAX__){
                            taxi[j].target=position_so[i];
                        }
                }
        }
}
int calculate_top_taxi(taxi_data *taxi,int max){
    int top_taxi=0;
    int x,out;
    out=0;
    for(x=1; x<max;x++){
        if(top_taxi<taxi[x].move){
            top_taxi = taxi[x].move;
            out = x;
        }
    }
    return out ;
}
int calculate_taxi_succes(taxi_data *taxi,int max){
    int top_taxi=0;
    int x,out;
    out =0;
    for(x=1; x<max;x++){
        if(top_taxi<taxi[x].exp_so){
            top_taxi = taxi[x].exp_so;
            out = x;
        }
    }
    return out ;
}
int calculate_taxi_time(taxi_data *taxi,int max){
    int top_taxi=0;
    int x,out;
    out =0;
    for(x=1; x<max;x++){
        if(top_taxi<taxi[x].time){
            top_taxi = taxi[x].time;
            out = x;
        }
    }
    return out ;
}

void print_metrics( maps_config * my_mp, int* array_id_taxi){
        int i,j;
        taxi_data* taxi_list;
        
        printf("PRINTING METRICS:\n");
        for(j=0; j<=my_mp->width; j++){
                printf("=");               
        }
        for(j=0; j<=my_mp->width; j++){
                printf("=");               
        }
        printf("\n");
}

void create_new_taxi(maps_config*my_mp,slot*maps,int new_id,taxi_data*taxi_list,int key_id_shm,int taxi_list_pos, int*position_taxi,int sem_sync_round,int ex_pos,pid_t*taxi_pid){
        int sem;
        int aspetta = 0;
        int my_y,my_x,ok,pid;

        char* args_tx[6] ={TAXI};
       
                        sem = 0;
                        args_tx[1] = integer_to_string_arg(key_id_shm);
                        args_tx[3] = integer_to_string_arg(taxi_list_pos);       
                        srand(time(NULL));
                            while(!sem){
                                my_x = rand()%my_mp->height;
                                my_y = rand()%my_mp->width;
                                sem=0;        
                                if(semctl(maps[my_x*my_mp->width+my_y].c_sem_id,0,GETVAL) && maps[my_x*my_mp->width+my_y].val_holes==0&&
                                my_x*my_mp->width+my_y!=ex_pos){
                                    sem=1;
                                }      
                            }
                            taxi_list[new_id].pos=my_x*my_mp->width+my_y;
                            taxi_list[new_id].x = my_x;
                            taxi_list[new_id].y = my_y;
                            position_taxi[new_id] = my_x*my_mp->width+my_y;
                            maps[my_x*my_mp->width+my_y].num_taxi = new_id;
                            sem_reserve(maps[my_x*my_mp->width+my_y].c_sem_id,0);
                                switch (taxi_pid[new_id]=fork()){
                                    case -1:
                                        TEST_ERROR
                                    break;
                                    case 0:
                                        args_tx[2]= integer_to_string_arg(new_id); /*id del taxi*/
                                        args_tx[4]=integer_to_string_arg(my_x*my_mp->width+my_y); /*posizione del taxi*/
                                        taxi_list[new_id].my_pid=taxi_pid[new_id];
                                        execve(TAXI,args_tx,NULL);
                                        TEST_ERROR;
                                    break;
                                    default:
                                    break;
                                }                             
                            taxi_list[new_id].my_pid=taxi_pid[new_id];
}

void check_taxi(maps_config*my_mp,slot*maps,taxi_data*taxi_list,int key_id_shm,int taxi_list_pos, int*position_taxi,pid_t* taxi_pid){
        int sem;
        int aspetta = 0;
        int pid;
        int my_y,my_x,ok,i,start;
        char* args_tx[6] ={TAXI};
        for(i=1;i<my_mp->num_taxi+1;i++){
            if(taxi_list[i].my_pid==-1){
                sem = 0;
                        args_tx[1] = integer_to_string_arg(key_id_shm);
                        args_tx[3] = integer_to_string_arg(taxi_list_pos);       
                        srand(time(NULL));
                            while(sem<1){
                                my_x = rand()%my_mp->height;
                                my_y = rand()%my_mp->width;
                                if(semctl(maps[my_x*my_mp->width+my_y].c_sem_id,0,GETVAL)){  
                                    taxi_list[i].x = my_x;
                                    taxi_list[i].y = my_y;
                                    position_taxi[i] = my_x*my_mp->width+my_y;
                                    maps[my_x*my_mp->width+my_y].num_taxi = i;
                                    taxi_list[i].pos =my_x*my_mp->width+my_y;
                                    sem_reserve(maps[my_x*my_mp->width+my_y].c_sem_id,0);
                                        switch (taxi_pid[i]=fork()) {
                                            case -1:
                                            /*il problema è dato qua, quando vengono create nuovi processi*/
                                                printf("errore taxi 1 pid=%d\n",taxi_pid[i]);
                                                exit(EXIT_SUCCESS);
                                            break;
                                            case 0:
                                                args_tx[2]= integer_to_string_arg(i); /*id del taxi*/
                                                args_tx[4]=integer_to_string_arg(my_x*my_mp->width+my_y); /*posizione del taxi*/
                                                taxi_list[i].my_pid=taxi_pid[i];
                                                execve(TAXI,args_tx,NULL);
                                                TEST_ERROR;
                                            break;
                                            default:
                                            break;
                                        } 
                                    sem=1;
                                }
                            }
                            

                                                     
            }
        }

}