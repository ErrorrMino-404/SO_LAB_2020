#include "maps.h"
#include "config.h"
#include <stdio.h>
#include <sys/sem.h>
#include <sys/shm.h>

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
                TEST_ERROR;
            }
            if((init_sem_to_val(sem_id, 0, 1))==-1){
                                TEST_ERROR;
            }
                        maps[i*width+j].c_sem_id=sem_id;
                        maps[i*width+j].val_holes = 0;
                        maps[i*width+j].x=i;
                        maps[i*width+j].y=j;
        }
    }

    return maps;
}

void print_maps(slot* maps, maps_config* my_mp){
    int i, j, y, part, u;
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
                    }else if(maps[i*my_mp->width+j].num_taxi+1 < 10){
                        printf("1");

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

}

void color(char* my_color){
        printf("\033%s", my_color);
}

void reset () {
        printf("\033[0m");
}
