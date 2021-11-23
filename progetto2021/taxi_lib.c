#include "taxi_lib.h"


void allocate_taxi(int x, int y, int tx,  slot* maps, int width ){
    maps[x*width+y].num_taxi = tx;
    init_sem_to_val(maps[x*width+y].c_sem_id,0 , 0);
}

int calculate_target(taxi_data taxi,int* arr,slot*maps){
    int i, j, best, x, y, dist;
    x = taxi.x;
    y = taxi.y;
    best = __INT_MAX__;
    for (i = 0; arr[i] != -1; i++) {
        if (maps[arr[i]].val_source != 0) {
        dist = abs(maps[arr[i]].x - x) + abs(maps[arr[i]].y - y);
        if ( dist < best) {
            best = dist;
            j = i;
        }
        }
  }
 return best==__INT_MAX__ ? -1 : j; 
}