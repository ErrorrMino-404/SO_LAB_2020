#include <unistd.h>
#include "sem_lib.h"

int sem_reserve(int sem_id, int sem_num) {
	struct sembuf sops;
	sops.sem_num = sem_num;
	sops.sem_op = -1;
	sops.sem_flg = 0;
	return semop(sem_id, &sops, 1);
}

int sem_relase(int sem_id, int sem_num) {
	struct sembuf sops;
	sops.sem_num = sem_num;
	sops.sem_op = 1;
	sops.sem_flg = 0;
	return semop(sem_id, &sops, 1);
}

int init_sem_to_val(int sem_id, int pos, int value){
	union semun my_sem;
	my_sem.val=value;
	return semctl(sem_id, pos, SETVAL, my_sem);
}

int wait_zero(int sem_id, int num_s){
	struct sembuf sops;
        sops.sem_num = num_s;
        sops.sem_op = -1;
        sops.sem_flg = 0;
        semop(sem_id, &sops, 1);
        sops.sem_flg = 0;
        sops.sem_op = 0;
        return semop(sem_id, &sops, 1);
}

int check_zero(int sem_id, int num_s){
	struct sembuf sops;
        sops.sem_num = num_s;
        sops.sem_flg = 0;
        sops.sem_op = 0;
        return semop(sem_id, &sops, 1);
}

int increase_resource(int sem_id, int pos, int val){
	struct sembuf sops;
        sops.sem_num = pos;
	sops.sem_flg = 0;
        sops.sem_op = val;
        return semop(sem_id, &sops, 1);
}