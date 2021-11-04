#include "source_lib.h"

int randomize_dest(int pos, maps_config* my_mp){
    int x,y,sem,i;
    sem = 0;
    srand(time(0));

    
        x = rand()%my_mp->height;
        y = rand()%my_mp->width;
        sem = x*my_mp->height+y;
        if(sem != pos){
            return sem;
        }else{
            return sem = randomize_dest(pos,  my_mp);
        }
   
}