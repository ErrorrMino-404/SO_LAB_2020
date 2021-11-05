#include "source_lib.h"

int randomize_dest(int pos, maps_config* my_mp){
    int x,y,sem,i;
    sem = 0;
    i = 0;
    srand(time(0));
    while(i<1){
        x = rand()%(my_mp->height);
        y = rand()%(my_mp->width);
        sem = x*my_mp->width+y;      
        if(sem != pos){
            i++;
        }  
    }
    return sem;
   
}