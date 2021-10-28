#include "master_lib.h"
#include "config.h"
#include <math.h>
#include <stdio.h>



keys_storage* fill_storage_shm (int idm, int idc, int ids, int idq,int idsemr){
    keys_storage* new_s;
    if((new_s=shmat(idm,NULL,0))==((void*)-1)){
        TEST_ERROR;
    }

    new_s->conf_id = idc;
    new_s->maps_id = ids;
    new_s->msgq_id = idq;
    new_s->ks_shm_id = idm;
    new_s->sem_sync_round = idsemr;
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

int* randomize_coordinate_taxi (taxi_data* taxi_list,slot* maps, maps_config* my_mp){
    int i, ok, sem, x, y,j;
    int* my_arr;
   
    if((my_arr = (int*)shmat(my_mp->num_taxi,NULL,0))==(void*) -1 ){
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
                maps[sem].num_taxi = i;
                i++;
            }
        }  

    }
    
    return my_arr;
}
int * randomize_coordinate_source (source_data* list_source, slot* maps, maps_config* my_mp, int source_id){
    int i, ok, sem, x, y,j;


}
void compute_targets(taxi_data* taxi,  int num_taxi, slot* maps){
    int i,j,x,y,k,h,best,dist,num_act;
    int position_so[4];

    num_act=(int)log(num_taxi);
    
    for(i=0; i<num_taxi;i++){
        taxi[i].target = -1;
    }
  

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
}