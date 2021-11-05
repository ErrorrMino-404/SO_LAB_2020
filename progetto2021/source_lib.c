#include "source_lib.h"
#include "master_lib.h"

int randomize_dest(int pos, maps_config* my_mp,source_data* source,slot* maps){
    int x,y,sem,i,j;
    sem = 0;
    i = 0;
    srand(time(0));
    while(i<1){
        x = (rand()+1)%(my_mp->height);
        y = (rand()+1)%(my_mp->width);
        sem = x*my_mp->width+y; 
        i++;
        for(j=0; j<my_mp->source; j++){
            if(sem == source[j].destin && semctl(maps[sem].c_sem_id,0,GETVAL)){
                i = 0;
            }
        }     
        
    }
    return sem;
   
}