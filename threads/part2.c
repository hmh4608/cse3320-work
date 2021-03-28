/*
*	Hoang Ho
*	ID 1001654608
*	CSE 3320-003
*	Due 03/28/2021 11:59 PM
*
*	Compilation: gcc part2.c -o part2 -lpthread
*/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>

#define NONSHARED 1
#define QUEUE_SIZE 5

sem_t consumerSem, producerSem;        
int prodQueuePos, consQueuePos = 0; //position of the next oldest or available slot for a character to be read in or write to
char queue[QUEUE_SIZE];
int done = 0; //marked by the producer to tell the consumer all characters have been read in, 

/*
*consumer thread reads from the queue[5] one by one as it is written by the producer to the queue
*/
void* consume(void* arg) 
{
  	printf("Consumer created\n");

  	while(1)
  	{
	    	if(!done) //only wait for producer to write more to the queue if there is still more to be written from the txt file
	    	{
	    		//read in more characters once producer has posted/signaled that it has written more
	    		sem_wait(&producerSem);
	    	}
			
		if(consQueuePos > QUEUE_SIZE)
		{
			consQueuePos = 0;	
		}
			
		//finished printing all of the characters in the loop and no more is being written, so leave the function
		if(prodQueuePos == consQueuePos && done) 
		{			
			break;
		}
		else
		{
			//sequentially read from the queue and prints the character in the same order they are written
			printf("%c\n", queue[consQueuePos]);
			consQueuePos++;
		}
			
	    	//tell producer to write more characters into the queue
	    	sem_post(&consumerSem);
  	}
}

/*
*producer thread writes to the queue[5] one by one
*arg - provided file name (should be message.txt)
*/
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
		    	//update the position within the queue for next character to read in
		    	prodQueuePos++;
		}
		//tell the consumer it can now read from the queue when something is written to it
		sem_post(&producerSem);
				
		//tell the consumer that the producer has read in all characters from the message, so print the rest that has not been printed and stop
		if(currentChar == EOF)
		{
			done = 1;
			break;
		}
	}
	
  	fclose(txtFile);
}

int main( int argc, char *argv[] )
{
	if(argc < 2)
    	{
      		printf("Error: You must pass in the datafile as a commandline parameter\n");
    	}

  	pthread_t producer;  
  	pthread_t consumer;

	//semaphores for the producer and consumer to communicate with each other
  	sem_init(&consumerSem, NONSHARED, 1); //set as 1 since consumer is ready to read from the queue at the beginning
  	sem_init(&producerSem, NONSHARED, 0);

	//create producer and consumer threads
  	pthread_create(&producer, NULL, produce, (void*)argv[1]);
  	pthread_create(&consumer, NULL, consume, NULL);

  	pthread_join(producer, NULL);
  	pthread_join(consumer, NULL);
}
