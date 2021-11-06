#include "maps.h"


slot* create_maps(int height, int width,int maps_id){
    printf("Creo la Mappa TAXI e RICHIESTE\n");
    int i, j, sem_id;
    slot* maps;
    
    if((maps=(slot*)shmat(maps_id, NULL, 0))== (void*) -1){
        TEST_ERROR;
    }
    
    for(i=0; i<height; i++){
        for(j=0; j<width; j++){
            if((sem_id = semget(IPC_PRIVATE, 1, IPC_CREAT|0666)) == -1){
                TEST_ERROR
            }
            if((init_sem_to_val(sem_id, 0, 1))==-1){
                                TEST_ERROR
            }
                        maps[i*width+j].c_sem_id=sem_id;
                        maps[i*width+j].val_holes = 0;
                        maps[i*width+j].num_taxi = -1;
                        maps[i*width+j].x=i;
                        maps[i*width+j].y=j;
                        maps[i*width+j].val_source = -1;
        }
    }

    return maps;
}
void clean_sem_maps(int height, int width, slot* maps){
    int i, j;
    for(i=0; i<height*width; i++){
        if((semctl(maps[i].c_sem_id, 0, IPC_RMID))==-1){
            TEST_ERROR
        }
    }
}

void print_maps(slot* maps, maps_config* my_mp, int* position_taxi,int* position_so){
    int i, j,u;
    int sem_m;

    /*stampa mappa*/
    printf("MAPPA DI CITTA' \n");
    
    for(j = 0; j <= my_mp->width-1; j++){
        printf("_");
    }
    printf("\n|");
    u = 0;
    for(i=0;i<my_mp->height; i++){
                for(j=0; j<my_mp->width; j++){
                    if((sem_m = semctl(maps[i*my_mp->width+j].c_sem_id,0, GETVAL))==-1){
                        TEST_ERROR;
                    }
                    if(sem_m!=0 && maps[i*my_mp->width+j].val_holes == 0){
                        printf(" ");
                    }else if(sem_m != 0 && maps[i*my_mp->width+j].val_holes!= 0){
                        printf("X");
                        u += 1;
                    }else if(maps[i*my_mp->width+j].num_taxi != -1){
                       
                        printf("1" );

                    }reset();
                        
                        if(j==my_mp->width-1){
                                printf("|\n|");
                        }
                }
    }
    for(j=0; j <= my_mp->width-1;j++){
        printf("_");
    }
    printf("|\n");
    /*stampo movimenti taxi*/
      printf("\n\nSOURCE NELLA MAPPA : \n");
    for(i=0;i < my_mp->source; i++){
        printf("SOURCE = %d POS = %d \n",i,position_so[i]);
    }
    printf("\n");
    printf("TAXI NELLA MAPPA : \n");
    for(i=0;i < my_mp->num_taxi; i++){
        printf("TAXI = %d POS = %d \n",i,position_taxi[i]);
    }
   
    for(j=0; j<=my_mp->width; j++){
                printf("=");
    }
    printf("\n\n");
}


void reset () {
        printf("\033[0m");
}
