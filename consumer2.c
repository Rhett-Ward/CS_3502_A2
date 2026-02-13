#include "buffer.h"

//Global variables for cleanup
shared_buffer_t* buffer = NULL;
sem_t* mutex = NULL;
sem_t* empty = NULL;
sem_t* full = NULL;
int shm_id = -1;

void cleanup(){
	// detach shared memory
	if (buffer != NULL) {
		shmdt(buffer);
	}

	// Close Semaphores
	if (mutex != SEM_FAILED) sem_close(mutex);
	if (empty != SEM_FAILED) sem_close(empty);
	if (full != SEM_FAILED) sem_close(full);
}

void signal_handler(int sig){
	printf("\nConsumer: Caught signal %d, cleaning up...\n", sig);
	cleanup();
	exit(0);
}

int main(int argc, char* argv[]) {
	if(argc != 3) {
		fprintf(stderr, "Usage: %s <consumer_id> <num_items>\n", argv[0]);
		exit(1);
	}

	int consumer_id = atoi(argv[1]);
	int num_items = atoi(argv[2]);

	// Set up signal handlers
	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);

	//seed random number generator
	srand(time(NULL) + consumer_id * 100);

	//To Do: Attach to shared memory
	shm_id = shmget(SHM_KEY, sizeof(shared_buffer_t), 0666);
	if (shm_id == -1) {
		perror("shmget failure");
		return(-1);
	}
	buffer = (shared_buffer_t*) shmat(shm_id, NULL, 0);
	if(buffer == ((void*) -1)){
			perror("shmat failure");
			return -1;
	}

	// To Do: Open Semaphore (Dont use O_CREAT - Producer Creates Them)
	mutex = sem_open("/sem_mutex", 0);
	empty = sem_open("/sem_empty", 0);
	full = sem_open("/sem_full", 0);
	if (mutex == SEM_FAILED || empty == SEM_FAILED || full == SEM_FAILED){
		perror("sem_open failure");
	cleanup();
	return(-1);
	}

	printf("Consumer %d: Starting to consumer %d items\n", consumer_id, num_items);

	//To Do: Main consumption loop
	for (int i = 0; i < num_items; i++){
		//To Do: wait for full slot
		sem_wait(full);

		// To Do: Enter Critical Section
		sem_wait(mutex);

		// To Do Remove Items From Buffer
		int producer_id = buffer->buffer[buffer->tail].producer_id;
		int value = buffer->buffer[buffer->tail].value;
		printf("Consumer %d: Consumed Value: %d from Producer: %d\n", consumer_id, value, producer_id);
		buffer-> tail = (buffer->tail + 1) % BUFFER_SIZE;
		buffer->count--;

		// To Do: Exit Critical Section
		sem_post(mutex);

		// To Do: Signal Empty Slot
		sem_post(empty);

		// Simulate consumption time
		usleep(rand() % 100000);
	}

	printf("Consumer %d: Finished consuming %d items\n", consumer_id, num_items);
	cleanup();
	return 0;
}
