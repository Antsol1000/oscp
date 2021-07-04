/***
* OPERATING SYSTEM WITH CONCURRENCY PROGRAMMING FINAL ASSIGNMENT
* A.2 - "The producer-consumer problem revisited"
*
* ANTONI SOLARSKI 148270
* SI3 - Artificial Intelligence
*
* This program realises the second project from the oscp coursebook.
* If is run without parameters allows to set buffer size, number of producers
* and consumers and time of production (consumption) for each producer (consumer)
* respectively. 
* Also it allows to set the number of items to be produced and consumed.
* If is run with any parameters the above values are set to default ones.
***/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>


// DEFAULT PARAMETERS VALUES
#define DEFAULT_BUFFER_SIZE 3
#define DEFAULT_ITEMS_LIMT 10
#define DEFAULT_PRODUCER_NUM 3
#define DEFAULT_CONSUMER_NUM 2
#define DEFAULT_P_TIME 1
#define DEFAULT_C_TIME 1


// MUTEXES
// mutex for buffer
pthread_mutex_t mutex_buffer;
// mutex for number of produced items
pthread_mutex_t mutex_pro_items;
// mutex for number of consumed items
pthread_mutex_t mutex_con_items;
// mutex for number of possible consumed items
pthread_mutex_t mutex_pcon_items;


// SEMAPHORES
sem_t sem_take; // wait when buffer is empty
sem_t sem_add; // wait when buffer is full


// PARAMETERS
int *buffer; // buffer (stack LIFO)
uint buffer_size; // size of buffer
uint items_limit; // number of items to be produced
uint producer_num; // number of producers
uint consumer_num; // number of consumers


// global variables to control program flow
int cursor = 0; // to control buffer (points to last added item)
uint produced_items = 0; // already produced items
uint consumed_items = 0; // already consumed items
// items that is going to be consumed (consumer is ready to taking from buffer)
uint possible_consumed_items = 0; 


// struct Arg for holding parameters for producers and consumers
struct Arg {
	ushort index; // index of producer (consumer)
	ushort time; // time of production (consumption)
};
typedef struct Arg Arg;


// prints buffer
void print_buff() {
	printf("Buffer (%d", buffer[0]);
	for (int i = 1; i < buffer_size; i++)
		printf(", %d", buffer[i]);
	printf(") size %d", cursor);
}


// prints number of currently produced and consumed items
void print_items() {
	printf("produced items %d, consumed items %d", produced_items, consumed_items);
}


// set params of buffer_size, items_limit, producer_num, consumer_num
void set_params() {
	printf("Enter buffer size: ");
	scanf("%u", &buffer_size);

	printf("Enter items limit: ");
	scanf("%u", &items_limit);

	printf("Enter producers number: ");
	scanf("%u", &producer_num);

	printf("Enter consumers number: ");
	scanf("%u", &consumer_num);
}


// set Arg (index and time) for producers and consumers
void set_times(Arg *producer_params, Arg *consumer_params) {
	for (int i = 0; i < producer_num; i++) {
		(producer_params + i)->index = i;
		printf("Enter time for %hu producer: ", i);
		scanf("%hu", &((producer_params + i)->time));
	}
	for (int i = 0; i < consumer_num; i++) {
		(consumer_params + i)->index = i;
		printf("Enter time for %hu consumer: ", i);
		scanf("%hu", &((consumer_params + i)->time));
	}
}


// set default values of params
void set_default_params() {
	buffer_size = DEFAULT_BUFFER_SIZE;
	items_limit = DEFAULT_ITEMS_LIMT;
	producer_num = DEFAULT_PRODUCER_NUM;
	consumer_num = DEFAULT_CONSUMER_NUM;
}

// set default values of times
void set_dafault_times(Arg *producer_params, Arg *consumer_params) {
	for (int i = 0; i < producer_num; i++) {
		(producer_params + i)->index = i;
		(producer_params + i)->time = DEFAULT_P_TIME;
	}
	for (int i = 0; i < consumer_num; i++) {
		(consumer_params + i)->index = i;
		(consumer_params + i)->time = DEFAULT_C_TIME;
	}
}


// producer program
void *producer(void* params) {
	uint my_item;
	// runs if the limit is not achived
	while (produced_items < items_limit) {

		// produce

		// set index of new item and increment the number of produced items
		pthread_mutex_lock(&mutex_pro_items);
		my_item = produced_items++;
		pthread_mutex_unlock(&mutex_pro_items);
		// print message and produce (sleep)
		printf("Producer %hu starts producing item %hu\n", 
			((Arg *)params)->index, my_item);
		sleep(((Arg *)params)->time);
		printf("Producer %hu end producing item %hu\n", 
			((Arg *)params)->index, my_item);

		// end of produce

		// add

		// wait if buffer is full
		sem_wait(&sem_add);

		// lock the buffer
		pthread_mutex_lock(&mutex_buffer);
		// put item and print buffer
		buffer[cursor++] = my_item;
		printf("Producer %hu puts item %hu in the buffer.\n", 
			((Arg *)params)->index, my_item);
		print_buff();
		printf("; ");
		print_items();
		printf("\n");
		// unlock the buffer
		pthread_mutex_unlock(&mutex_buffer);
		
		// increment taking sem
		sem_post(&sem_take);

		// end of add
	}
}

void *consumer(void* params) {
	uint my_item;
	while (possible_consumed_items < items_limit) {

		// increment the number of items that are going to be consumed
		pthread_mutex_lock(&mutex_pcon_items);
		possible_consumed_items++;
		pthread_mutex_unlock(&mutex_pcon_items);


		// take

		// wait if empty
		sem_wait(&sem_take);

		// lock the buffer
		pthread_mutex_lock(&mutex_buffer);
		// take item and print buffer
		my_item = buffer[--cursor];
		buffer[cursor] = -1;
		printf("Consumer %hu takes item %hu from the buffer.\n", 
			((Arg *)params)->index, my_item);
		print_buff();
		printf("; ");
		print_items();
		printf("\n");
		// unlock the buffer
		pthread_mutex_unlock((&mutex_buffer));
		
		// increment adding sem
		sem_post(&sem_add);

		// end of take

		// consume

		// print message and consume (sleep)
		printf("Consumer %hu start consuming item %hu\n", 
			((Arg *)params)->index, my_item);
		sleep(((Arg *)params)->time);
		printf("Consumer %hu end consuming item %hu\n", 
			((Arg *)params)->index, my_item);
		// increment number of consumed items
		pthread_mutex_lock(&mutex_con_items);
		consumed_items++;
		pthread_mutex_unlock(&mutex_con_items);

		// end of consume
	}
}


int main(int argc, char* argv[]) {

	// if there are parameters set default
	(argc > 1) ? set_default_params() : set_params();

	// create the empty buffer
	buffer = malloc(sizeof(int) * buffer_size);
	for (int i = 0; i < buffer_size; i++)
		buffer[i] = -1;
	cursor = 0;

	// initialize the producers and consumers params
	Arg *producer_params = malloc(sizeof(Arg) * producer_num);
	Arg *consumer_params = malloc(sizeof(Arg) * consumer_num);
	// if params set default
	(argc > 1) ? set_dafault_times(producer_params, consumer_params) :
		set_times(producer_params, consumer_params);

	// create mutexes
	pthread_mutex_init(&mutex_buffer, NULL);
	pthread_mutex_init(&mutex_con_items, NULL);
	pthread_mutex_init(&mutex_pro_items, NULL);
	pthread_mutex_init(&mutex_pcon_items, NULL);

	// create semaphores
	sem_init(&sem_add, 0, buffer_size);
	sem_init(&sem_take, 0, 0);

	// create threads
	pthread_t *producer_threads = malloc(sizeof(pthread_t) * producer_num);
	pthread_t *consumer_threads = malloc(sizeof(pthread_t) * consumer_num);
	for (int i = 0; i < consumer_num; i++) {
		pthread_create(&consumer_threads[i], NULL, &consumer, consumer_params + i);
	}
	for (int i = 0; i < producer_num; i++)
		pthread_create(&producer_threads[i], NULL, &producer, producer_params + i);

	// joind threads
	for (int i = 0; i < producer_num; i++)
		pthread_join(producer_threads[i], NULL);
	for (int i = 0; i < consumer_num; i++)
		pthread_join(consumer_threads[i], NULL);

	// destroy semaphores
	sem_destroy(&sem_take);
	sem_destroy(&sem_add);

	// destroy mutexes
	pthread_mutex_destroy(&mutex_buffer);
	pthread_mutex_destroy(&mutex_con_items);
	pthread_mutex_destroy(&mutex_pro_items);
	pthread_mutex_destroy(&mutex_pcon_items);

	// print summary
	printf("\nSUMMARY\n");
	printf("-----------------------------------\n");
	printf("CURRENT BUFFER ");
	print_buff();
	printf("\n");
	printf("TOTAL PRODUCED ITEMS %d\n", produced_items);
	printf("TOTAL CONSUMED ITEMS %d\n", consumed_items);

	// free the memory
	free(buffer);
	free(producer_params);
	free(consumer_params);
	free(producer_threads);
	free(consumer_threads);

	return 0;
}