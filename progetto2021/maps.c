#include "maps.h"
#include "config.h"
#include <stdio.h>


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
                        maps[i*width+j].num_taxi = 0;
                        maps[i*width+j].attr = 0;
                        maps[i*width+j].x=i;
                        maps[i*width+j].y=j;
                        maps[i*width+j].val_source = -1;
                        maps[i*width+j].top_cells = 0;
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
void merge (int* a, int left, int center, int right){
    int i,j,k;
    i = left;
    j = center+1;
    k = 0;
    int b[right-left+1];

    while(i<=center&&j<=right){
        if(a[i]<=a[j]){
            b[k]=a[i];
            i ++;
        }else {
            b[k]=a[j];
            j ++;
        }
        k++;
    }
    while(i<=center){
        b[k] = a[i];
        i++;
        k++;
    }
    while(j<=right){
        b[k] = a[j];
        j++;
        k++;
    }

    for(k=left;k<=right;k++){
        a[k] = b[k-left];
    }

}
void mergesort(int* top_cells, int left,  int right){
    if(left < right){
        int center = (left+right)/2;
        mergesort(top_cells, left,center);
        mergesort(top_cells,center+1, right);
        merge(top_cells,left,center,right);
    }
}
void print_maps(slot* maps,maps_config* my_mp,int* position_so,int top_taxi,int taxi_succes,int succ,int aborti,int inve){
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
                    if((sem_m = semctl(maps[i*my_mp->width+j].c_sem_id,0, GETVAL))==-1){
                        TEST_ERROR;
                    }
                    if( maps[i*my_mp->width+j].val_holes == 0 && maps[i*my_mp->width+j].val_source == -1){
                        printf(" ");
                    }else if(sem_m != 0 && maps[i*my_mp->width+j].val_holes!= 0){
                        printf("X");
                        u += 1;
                    }/*posizone delle source*/
                    else if(maps[i*my_mp->width+j].val_source!= -1){
                        for(x=0;x<2; x++){
                            if (position_so[x] == i*my_mp->width+j ) {
                                printf("1");
                            }
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
    
    for(x=0;x<=my_mp->width*my_mp->height;x++){
        mov_cell[x] = maps[x].top_cells;
    }
    mergesort(mov_cell,0,my_mp->width*my_mp->height);

    j=0;
    for(x=my_mp->height*my_mp->width;x>160;x--){
        printf("%d) CELLA = %d  ",j,mov_cell[x]);
        x--;
        j++;
        printf("%d) CELLA = %d \n",j,mov_cell[x]);
        j++;

    }
    printf("\nTAXI CON MAGGIORE MOVIMENTI => %d\n",top_taxi);
    printf("\nTAXI CON MAGGIORE SOURCE RACCOLTE => %d\n",taxi_succes);


    for(j=0; j<=my_mp->width; j++){
                printf("=");
    }
    printf("\n\n");
}


void reset () {
        printf("\033[0m");
}
