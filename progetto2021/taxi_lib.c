#include "taxi_lib.h"


void allocate_taxi(int x, int y, int tx,  slot* maps, int width ){
    maps[x*width+y].num_taxi = tx;
    init_sem_to_val(maps[x*width+y].c_sem_id,0 , 0);
}

