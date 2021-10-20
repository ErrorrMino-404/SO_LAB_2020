#include "master_lib.h"

#include <stdlib.h>
#include <sys/sem.h>



keys_storage* fill_storage_shm (int idm, int idc, int ids, int idq, int idp){
    keys_storage* new_s;
    if((new_s=shmat(idm,NULL,0))==((void*)-1)){
        TEST_ERROR;
    }

    new_s->conf_id = idc;
    new_s->maps_id = ids;
    new_s->msgq_id = idq;
    new_s->ks_shm_id = idm;
    
    new_s->sem_set_pl = idp;
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
    printf("sono entrato\n");
    if((my_arr = (int*) shmat(arr_id,NULL,0))==(void*) -1 ){
        printf("errore nel randomize_source my arr \n");
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
                printf("val di tmp_index %d \n", tmp_index);
                my_arr[i]=tmp_index;
                maps[my_arr[i]].val_holes = 1; /*la source è attiva  */              
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

void randomize_coordinate_taxi (taxi_data* taxi_list,slot* maps,int my_pos, int width,int height){
    int i, ok, sem, x, y;
    srand(time(NULL));
  
    while(!ok){
        x=rand()%height;
        y=rand()%width;
        sem=maps[x*width+y].c_sem_id;
        if((semctl(sem,0, GETVAL))){
            ok = 1;
        }
    }
    taxi_list[my_pos].x = x;
    taxi_list[my_pos].y = y;
}