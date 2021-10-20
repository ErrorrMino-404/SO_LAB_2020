CFLAGS = -std=c89 -pedantic

all: master taxi

master: init.conf taxi.c master.c  maps.c maps.h config.h config.c  sem_lib.h sem_lib.c master_lib.h master_lib.c taxi_lib.h taxi_lib.c
	gcc $(CFLAGS) master.c config.c maps.c sem_lib.c master_lib.c -o master
taxi: init.conf master.c taxi.c master_lib.c master_lib.h maps.c maps.h config.h config.c sem_lib.h sem_lib.c taxi_lib.h taxi_lib.c
	gcc $(CFLAGS) taxi.c config.c maps.c sem_lib.c master_lib.c taxi_lib.c -o taxi
run:  all
	./master