#include "buffer.h"

// Global variables for cleanup
shared_buffer_t* buffer = NULL;
sem_t* mutex = NULL;
sem_t* empty = NULL;
sem_t* full = NULL;
int shm_id = -1;

void cleanup() {
	//detach shared memory
	if (buffer != NULL) {
	shmdt(buffer);
	}

	// Close semaphores (dont unlink - other processes may be using)
	if (mutex != SEM_FAILED) sem_close(mutex);
	if (empty != SEM_FAILED) sem_close(empty);
	if (full != SEM_FAILED) sem_close(full);
}

void signal_handler(int sig){
	printf("\n Producer: Caught signal %d, cleaning up...\n", sig);
	cleanup();
	exit(0);
}

int main(int argc, char* argv[]){
	if (argc != 3) {
		fprintf(stderr, "Usage: %s <producer_id> <num_items>\n", argv[0]);
		exit(1);
	}

	int producer_id = atoi(argv[1]);
	int num_items = atoi(argv[2]);

	// set up signal handlers
	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);

	// seed random number Generator
	srand(time(NULL) + producer_id);

	//To DO : Attach to shared memory
	shm_id = shmget(SHM_KEY, sizeof(shared_buffer_t), IPC_CREAT | 0666);
	if (shm_id  == -1){
		return -1;
	}
	buffer = (shared_buffer_t*) shmat(shm_id, NULL, 0);
	if (buffer == ((void*) -1)){
		return (-1);
	}
	// TO DO: Open semaphores
	mutex = sem_open("/sem_mutex", O_CREAT, 0644, 1);
	empty = sem_open("/sem_empty", O_CREAT, 0644, BUFFER_SIZE);
	full = sem_open("/sem_full", O_CREAT, 0644, 0);

	if (mutex == SEM_FAILED || empty == SEM_FAILED || full == SEM_FAILED){
		perror("sem_open failure");
	cleanup();
	return(-1);
	}

	printf("Producer %d: Starting to produce %d items\n", producer_id, num_items);

	// To Do: main production loop
	for (int i = 0; i < num_items; i++) {
		//create item
		item_t item;
		item.value = producer_id * 1000 + i;
		item.producer_id = producer_id;

		// To Do: wait  for empty slot
		sem_wait(empty);

		// To Do: Enter Critical Section
		sem_wait(mutex);

		// TO Do: add item to buffer
		buffer -> buffer[buffer->head] = item;
		buffer -> head = (buffer->head + 1) % BUFFER_SIZE;
		buffer->count++;

		printf("Producer %d: Produced value %d\n", producer_id, item.value);

		// To Do: Exit Critical Section
		sem_post(mutex);

		// To Do: Signal Item Available
		sem_post(full);

		// Simulate Production time
		usleep(rand() % 100000);
	}

	printf("Producer %d: Finished producing %d items\n", producer_id, num_items);
	cleanup();
	return 0;
}

