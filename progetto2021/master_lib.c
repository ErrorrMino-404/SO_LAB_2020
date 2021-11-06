#include "master_lib.h"
#include <stdio.h>




keys_storage* fill_storage_shm (int idm, int idc, int ids, int idq,int idqso,int idsemr,int idp){
    keys_storage* new_s;
    if((new_s=shmat(idm,NULL,0))==((void*)-1)){
        TEST_ERROR;
    }

    new_s->conf_id = idc;
    new_s->maps_id = ids;
    new_s->msgq_id = idq;
    new_s->ks_shm_id = idm;
    new_s->msgq_id_so = idqso;
    new_s->sem_sync_round = idsemr;
    new_s->sem_set_tx = idp;
    return new_s;
}

int get_rand_so(int min, int max){
    srand(time(NULL));
    return min==max ? min :(rand()% (max-min)) + min;
}

int* randomize_holes(int arr_id, int ho_num, maps_config* my_mp,slot* maps){
    int* my_arr;
    int i,j,x,y,ok;
    int num_r, num_sem, tmp_index;
    
    if((my_arr = (int*) shmat(arr_id,NULL,0))==(void*) -1 ){
        TEST_ERROR
    }
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
                my_arr[i]=tmp_index;
                maps[tmp_index].val_holes = 1; /*la holes è attiva  */  
                i++;
            }
        }
    }
        for(i=ho_num; i!=my_mp->holes; i++){ 
                maps[my_arr[rand()%(ho_num)]].val_holes+=1;
        }
    my_arr[ho_num] =-1;
    return my_arr;
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
   
    if((my_arr = (int*)shmat(tx_num,NULL,0))==(void*) -1 ){
        TEST_ERROR
    }
    srand(time(NULL));
    i =0;
    while(i<my_mp->num_taxi){
        x = rand()%(my_mp->height);
        y = rand()%(my_mp->width);
        sem = x*my_mp->width+y;
        if(semctl(maps[sem].c_sem_id,0,GETVAL)&& maps[sem].val_holes!=1){
            ok=1;
            for( j=0; j<i; j++){
                if(my_arr[j]==sem ){
                    ok=0;
                }
            }
            if(ok==1){
                my_arr[i]=sem;
                /*do la posizione sulla mappa*/
                taxi_list[i].x = x;
                taxi_list[i].y = y;
                i++;
            }
        }  

    }
    
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
        if(maps[sem].val_holes == 0 && maps[sem].num_taxi == -1){
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
void compute_targets(taxi_data* taxi,  int num_taxi, slot* maps,int* position_so){
    int i,j,x,y,k,h,best,dist,num_act;

    num_act=(int)log(num_taxi);
    
    for(i=0; i<num_taxi;i++){
        taxi[i].target = -1;
    }
    printf("calcolo del target \n");
    for(i=0;position_so[i]!=-1 ;i++){
                best=__INT_MAX__;
                j=__INT_MAX__;
                for(k=0;k<num_taxi;k++){
                    if(taxi[k].target==-1){
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
                    printf("TAXI = %d SOURCE=%d \n", j,position_so[i]);
                }
        }
        for(i=0;position_so[i]!=-1 ;i++){
                for(h=0;h<num_act;h++){
                        best=__INT_MAX__;
                        j=__INT_MAX__;
                        for(k=0;k<num_taxi;k++){
                            if(taxi[i].target!=-1){
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
        printf("termino target \n");
}


void print_metrics( maps_config * my_mp, int* array_id_taxi){
        int i,j;
        taxi_data* taxi_list;
        
        printf("PRINTING METRICS:\n");
        for(j=0; j<=my_mp->width; j++){
                printf("=");               
        }
        /*for(i=0;i<my_mp->num_taxi;i++){
                printf("TAXI=%d POSIZIONE=%d TARGET_SO=%d \n",i, taxi_list[i].pos,taxi_list[i].target);
        }
       */

        for(j=0; j<=my_mp->width; j++){
                printf("=");               
        }
        printf("\n");
}