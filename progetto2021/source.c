#include "config.h"
#include "source_lib.h"

int main (int argc, char *argv[]){
    slot* maps;
    keys_storage* my_ks;
    maps_config* my_mp;
    int keys_id, my_id;
    taxi_data* my_taxi;

    /*configuro la chiave condivisa*/
    keys_id = atoi(argv[1]);

    if((my_ks=shmat(keys_id,NULL,0))==(void*)-1){
        TEST_ERROR;
    }
    if((my_mp=shmat(my_ks->conf_id,NULL,0))==(void*)-1){
        TEST_ERROR;
    }
    if((maps=shmat(my_ks->maps_id, NULL,0))==(void*)-1){
        TEST_ERROR;
    }

}