#ifndef _SEM_LIB_
#define _SEM_LIB_
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

union semun {
	int              val;    /* valore per SETVAL */
	struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
	unsigned short  *array;  /* Array for GETALL, SETALL */
	struct seminfo  *__buf;  /* Buffer for IPC_INFO (Linux-specific) */
};

/*setta valore set semafori*/
int sem_set_val(int, int, int);

/*reserve del semaforo*/
int sem_reserve(int, int);

/*release del semaforo*/
int sem_release(int, int);

/*inizializzazione valore semaforo*/
int init_sem_to_val(int, int, int);

/*reserve del semaforo e attesa dello zero*/
int wait_zero(int, int);

/*attesa dello zero*/
int check_zero(int, int);

/*incremento di un valore il semaforo*/
int increase_resource(int,int,int);

#endif