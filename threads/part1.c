/*
*	Hoang Ho
*	ID 1001654608
*	CSE 3320-003
*	Due 03/24/2021 5:30 PM
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>

#define MAX 5000000

int total = 0;
int n1,n2; 
char *s1,*s2;
pthread_mutex_t mutex; //to guard the critical areas
int NUM_THREADS = 4;
int partition;

FILE *fp;


/*
*reads the two strings from a file named *.txt
*filename - file where the two strings are being read in from
*returns whether the file read was successful or failed
*/
int readf(char* filename)
{
    if((fp=fopen(filename, "r"))==NULL)
    {
        printf("ERROR: can’t open %s!\n", filename);
        return -1;
    }
    
    s1=(char *)malloc(sizeof(char)*MAX);
    
    if (s1==NULL)
    {
        printf ("ERROR: Out of memory!\n") ;
        return -1;
    }
    
    s2=(char *)malloc(sizeof(char)*MAX);
    
    if (s1==NULL)
    {
        printf ("ERROR: Out of memory\n") ;
        return -1;
    }
    
    /*read s1 s2 from the file*/
    
    s1 = fgets(s1, MAX, fp);
    s2 = fgets(s2, MAX, fp);
    n1 = strlen(s1); /*length of s1*/
    n2 = strlen(s2)-1; /*length of s2*/
    partition = n1/NUM_THREADS; //giving a thread a local range to search for the substring s2
    
    if( s1 == NULL || s2 == NULL || n1 < n2 ) /*when error exit*/
    {
        return -1;
    }
    
    return 0;
}

/*
*calculates the number of substrings s2 in string s1 from the .txt file
*returns the total number of substrings s2 for the local partition of the s1 string of the current thread 
*/
void* num_substring ( void* param )
{
    int i,j,k;
    int count ;
    int* currentThread = (int*)param; //storing which thread it is
    
    int startPos = (*currentThread)*partition;
    
    //move to the first position of the current partition we are trying to find substring s2 in
    char* localStr = s1+startPos;
    
    for (i = 0; i <= (partition-n2); i++)
    {
        count = 0;
        for(j = i ,k = 0; k < n2; j++,k++)
        { /*search for the next string of size of n2*/
            if (*(localStr+j)!=*(s2+k))
            {
                break;
            }
            else
            {
                count++;
            }
            if (count==n2)
            {
            	//lock critical region since we're editing a global variable
            	pthread_mutex_lock(&mutex);
                total++; /*found a substring in this step*/
                pthread_mutex_unlock(&mutex);
            }
         }
    }
}

/*
*creates arrays of pthreads and an array that holds the number pthread created and starts working on the section of s1 each thread works on concurrently
*/
void solve_threaded()
{
	pthread_t threads[NUM_THREADS];
	int thread_num[NUM_THREADS]; //identifies which thread in the sequence we launched them to use as an argument to thread creation later
	int i;
	//create and launch all the threads
	for(i=0; i<NUM_THREADS; ++i)
	{
		thread_num[i] = i;
		if(pthread_create(&(threads[i]), NULL, num_substring, (void*)&thread_num[i]))
		{
			printf("ERROR: Thread creation failed\n");
			exit(EXIT_FAILURE);
		}
	}
	
	//thread completion - waiting for each thread to finish
	for(i=0; i<NUM_THREADS; ++i)
	{
		if(pthread_join(threads[i], NULL))
		{
			printf("ERROR: Thread joining failed\n");
			exit(EXIT_FAILURE);
		}
	}
}
 
int main(int argc, char *argv[])
{
    if( argc < 2 )
    {
      printf("Error: You must pass in the datafile as a commandline parameter\n");
    }

    readf ( argv[1] ) ;
	
    struct timeval start, end;
    float mtime; 
    int secs, usecs;

	pthread_mutex_init(&mutex, NULL);
    gettimeofday(&start, NULL);
    solve_threaded();
    gettimeofday(&end, NULL);
	pthread_mutex_destroy(&mutex);

    secs  = end.tv_sec  - start.tv_sec;
    usecs = end.tv_usec - start.tv_usec;
    mtime = ((secs) * 1000 + usecs/1000.0) + 0.5;

    printf ("The number of substrings is : %d\n" , total) ;
    printf ("Elapsed time is : %f milliseconds\n", mtime );

    if( s1 )
    {
      free( s1 );
    }

    if( s2 )
    {
      free( s2 );
    }

    return 0 ; 
}
