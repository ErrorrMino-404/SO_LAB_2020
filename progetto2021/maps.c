#include "maps.h"
#include "config.h"
#include <stdio.h>

int random_val(int min,int max){
    int val;
    srand(time(NULL));
    val = min==max? min : (rand() % (max - min)) + min;
    return val;
}

slot* create_maps(int height, int width,int maps_id,int tmp_min,int tmp_max,int min_cell,int max_cell){
    printf("Creo la Mappa TAXI e RICHIESTE\n");
    int i, j, sem_id;
    int val;
    slot* maps;
    if((maps=(slot*)shmat(maps_id, NULL, 0))== (void*) -1){
        TEST_ERROR;
    }
    for(i=0; i<height; i++){
        for(j=0; j<width; j++){
            val=random_val(min_cell,max_cell);
            if((sem_id = semget(IPC_PRIVATE,val,IPC_CREAT|0666)) == -1){
                TEST_ERROR
            }
            if((init_sem_to_val(sem_id, 0,val))==-1){
                                TEST_ERROR
            }
                        maps[i*width+j].c_sem_id=sem_id;
                        maps[i*width+j].val_holes = 0;
                        maps[i*width+j].num_taxi = 0;
                        maps[i*width+j].attr = 0;
                        maps[i*width+j].x=i;
                        maps[i*width+j].y=j;
                        maps[i*width+j].val_source = -1;
                        maps[i*width+j].top_cells = 0;
                        maps[i*width+j].tmp_attr = random_val(tmp_min,tmp_max);
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

void print_maps(slot* maps,maps_config* my_mp,int* position_so,int top_taxi,int taxi_succes,int succ,int aborti,int inve,int val_move,int val_succ,int taxi_time,int val_time){
    int i, j,u,x,y;
    int sem_m;
    int mov_cell[my_mp->height*my_mp->width];
    
    /*stampa mappa*/
    printf("MAPPA DI CITTA' \n");
    
    for(j = 0; j <= my_mp->width-1; j++){
        printf("_");
    }
    printf("\n|");
    u = 0;
    for(i=0;i<my_mp->height; i++){
                for(j=0; j<my_mp->width; j++){
                    u = 0;
                    if((sem_m = semctl(maps[i*my_mp->width+j].c_sem_id,0, GETVAL))==-1){
                        TEST_ERROR;
                    }
                    if( maps[i*my_mp->width+j].val_holes == 0 && maps[i*my_mp->width+j].val_source == -1){
                        printf(" ");
                    }else if(maps[i*my_mp->width+j].val_holes!= 0){
                        printf("X");
                       
                    }/*posizone delle source*/
                    else if(maps[i*my_mp->width+j].val_source!= -1){
                        for(x=0;x<2; x++){
                            if (position_so[x] == i*my_mp->width+j ) {
                                printf("1");
                                u = 1;
                            }
                        } 
                        if(u!=1){
                            printf(" ");
                        } 
                        
                    }

                    reset();
                        
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
    printf("SUCCESSI: %d\n", succ);
    printf("INEVASI:  %d\n", inve);
    printf("ABORTITI: %d\n",aborti);
    printf("\nSOURCE NELLA MAPPA : \n");
    for(i=0;i < 2; i++){
        printf("SOURCE = %d POS = %d \n",i,position_so[i]);
    }
    printf("\n");
    printf("\nTOP %d CELLS : \n", my_mp->top_cells);
    /*applico un algoritmo di ordinamento per velocizzare l'ordinamento*/
    int max;
    max=0;
    j=0;
    int tmp;
    for(i=0;i<my_mp->height*my_mp->width;i++){
        mov_cell[i] = i;
    }
        for(i=0;i<my_mp->height*my_mp->width-1;i++){
            max=i;
            for(j=i+1; j<my_mp->height*my_mp->width;j++){
                if(maps[mov_cell[j]].top_cells>maps[mov_cell[max]].top_cells){
                    max=j;
                }
            }
            tmp=mov_cell[i];
            mov_cell[i]=mov_cell[max];
            mov_cell[max]=tmp;
        }
    for(i=0;i<my_mp->top_cells;i++){
        printf("%d) CELLS= %d   ", i, mov_cell[i]);
        i++;
        printf("%d) CELLS= %d\n", i, mov_cell[i]);

    }
    printf("\nTAXI CON MAGGIORE MOVIMENTI => %d         MOVIMENTI=>%d \n",top_taxi,val_move);
    printf("\nTAXI CON MAGGIORE SOURCE RACCOLTE => %d   RISORSE=>%d \n",taxi_succes,val_succ);
    printf("\nTAXI CON IL VIAGGIO LUNGO=> %d            SECONDI=>%f \n",taxi_time,(float)val_time/1000000000);
    

    for(j=0; j<=my_mp->width; j++){
                printf("=");
    }
    printf("\n\n");
}


void reset () {
        printf("\033[0m");
}
