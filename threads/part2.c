/*
*	Hoang Ho
*	ID 1001654608
*	CSE 3320-003
*	Due 03/28/2021 11:59 PM
*/

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

#define MAX 5000000
#define NONSHARED 1
#define QUEUE_SIZE 5

sem_t consumerSem, producerSem;        
int prodQueuePos, consQueuePos = 0; //position of the next oldest or available slot for a character to be read in or write to
char queue[QUEUE_SIZE];

void* consume(void* arg) 
{
  	printf("Consumer created\n");

  	while(1)
  	{
    	//read in more characters
    	sem_wait(&producerSem);
		
		if(consQueuePos > QUEUE_SIZE)
		{
			consQueuePos = 0;
		}
		//sequentially read from the queue and prints the character in the same order they are written
		printf("%c", queue[consQueuePos]);
		consQueuePos++;	
		
    	//tell producer to write more characters into the queue
    	sem_post(&consumerSem);

    	// Sleep a little bit so we can read the output on the screen
    	sleep(2);
  }
}

void* produce(void* arg) 
{
  	printf( "Producer created\n" );
  	
  	char* filename = (char*)arg;
	FILE* txtFile;
	
	if((txtFile=fopen(filename, "r"))==NULL)
    {
        printf("ERROR: can’t open %s!\n", filename);
        exit(EXIT_FAILURE);
    }

	char currentChar;

  	while(1)
  	{
    	//wait until the consumer is ready to read in more characters
    	sem_wait(&consumerSem);
    	if(prodQueuePos > QUEUE_SIZE)
    	{
    		prodQueuePos = 0;
		}
    	
    	if((currentChar = fgetc(txtFile)) != EOF)
    	{
    		queue[prodQueuePos] = currentChar;
		}
		
		//update the position within the queue for next character to read in
    	prodQueuePos++;
    	sem_post(&producerSem);

    	// Sleep a little bit so we can read the output on the screen
    	sleep(1);
  	}
  	
  	fclose(txtFile);
}

int main( int argc, char *argv[] ) 
{
  	if(argc < 2)
    {
      printf("Error: You must pass in the datafile as a commandline parameter\n");
    }
  
  	time_t t;

  	srand((unsigned int)time(&t));

  	pthread_t producer;  
  	pthread_t consumer;

  	sem_init(&consumerSem, NONSHARED, 1); //set as 1 since consumer is ready to read from the queue at the beginning
  	sem_init(&producerSem, NONSHARED, 0);

  	pthread_create(&producer, NULL, produce, (void*)&argv[1]);
  	pthread_create(&consumer, NULL, consume, NULL);

  	pthread_join(producer, NULL);
  	pthread_join(consumer, NULL);
}
