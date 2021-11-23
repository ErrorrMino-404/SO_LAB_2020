#include "config.h"

maps_config *init_maps_config(int shm_id){
   
    FILE *my_file;
    maps_config *my_maps;
    char* line = NULL;
    ssize_t length = 0;
    
    
    my_file = fopen("init.conf","r");
    if((my_maps = (maps_config*)shmat(shm_id,NULL,0)) == (void *) -1){
        TEST_ERROR;
    }
    line = NULL;

    if(my_file == NULL){
        
        TEST_ERROR;
    }
    

    while(getline(&line, &length, my_file)!=EOF){ 
                if(strstr(line, "SO_BASE")!=NULL){
                        my_maps->width=find_value(line);
                }else if(strstr(line, "SO_ALTEZZA")!=NULL){
                        my_maps->height=find_value(line);
                }else if(strstr(line, "SO_HOLES")!=NULL){
                        my_maps->holes = find_value(line);
                }else if(strstr(line,"SO_SOURCES")!=NULL){
                        my_maps->source = find_value(line);
                }else if(strstr(line,"SO_TAXI")!= NULL){
                        my_maps->num_taxi = find_value(line);
                }else if(strstr(line,"SO_CAP_MIN")!= NULL){
                        my_maps->min_taxi_cell = find_value(line);
                }else if(strstr(line,"SO_CAP_MAX")!= NULL){
                        my_maps->max_taxi_cell = find_value(line);
                }else if(strstr(line,"SO_TIMENSEC_MIN")!= NULL){
                        my_maps->timensec_min = find_value(line);
                }else if(strstr(line,"SO_TIMENSEC_MAX")!= NULL){
                        my_maps->timensec_max = find_value(line);
                }else if (strstr(line,"SO_TOP_CELLS")!= NULL){
                        my_maps->top_cells = find_value(line);
                }else if (strstr(line,"SO_DURANTION")!= NULL){
                        my_maps->durantion = find_value(line);
                }else{
                       TEST_ERROR;
                }
        }

    if((fclose(my_file))!= 0){
        TEST_ERROR;
    }
        printf("ho terminato in config.c\n");

    return my_maps;

}

int find_value(char *line){
    strtok(line,"=");
    return atoi(strtok(NULL, "="));
}