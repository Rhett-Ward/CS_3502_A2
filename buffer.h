#ifndef BUFFER_H
#define BUFFER_H

// Required includes for both producer and consumer
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <time.h>

// Constants for shared memory and semaphores
#define BUFFER_SIZE 10
#define SHM_KEY 0x1234
#define SEM_MUTEX "/sem_mutex"
#define SEM_EMPTY "/sem_empty"
#define SEM_FULL "/sem_full"


typedef struct {
	int value;		// The Data
	int producer_id;	// Which producer created this item
} item_t;

typedef struct {
	item_t buffer[BUFFER_SIZE];
	int head;		//Next Wroite Position (Producer)
	int tail;		//Next Read Position (Consumer)
	int count;		//Current number of items;
} shared_buffer_t;

#endif
