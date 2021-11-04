CFLAGS = -std=c89 -pedantic

all: master taxi source

master: init.conf taxi.c master.c source.c maps.c maps.h config.h config.c  sem_lib.h sem_lib.c master_lib.h master_lib.c taxi_lib.h taxi_lib.c source_lib.h source_lib.c 
	gcc $(CFLAGS)  master.c config.c maps.c sem_lib.c master_lib.c -lm  -o master 
taxi: init.conf master.c taxi.c source.c master_lib.c master_lib.h maps.c maps.h config.h config.c sem_lib.h sem_lib.c taxi_lib.h taxi_lib.c source_lib.h source_lib.c 
	gcc $(CFLAGS) taxi.c master_lib.c config.c maps.c sem_lib.c taxi_lib.c -lm -o taxi
source: init.conf master.c taxi.c source.c master_lib.c master_lib.h maps.c maps.h config.h config.c sem_lib.h sem_lib.c taxi_lib.h taxi_lib.c source_lib.h source_lib.c 
	gcc $(CFLAGS) source.c source_lib.c config.c maps.c sem_lib.c taxi_lib.c  -lm -o source
run:  all
	./master