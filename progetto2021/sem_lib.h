#ifndef _SEM_LIB_
#define _SEM_LIB_
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

union semun {
	int              val;
	struct semid_ds *buf;  
	unsigned short  *array;  
	struct seminfo  *__buf;  
};
int sem_set_val(int, int, int);

int sem_reserve(int, int);

int sem_relase(int, int);

int init_sem_to_val(int, int, int);

int wait_zero(int, int);

int check_zero(int, int);

int increase_resource(int,int,int);

#endif