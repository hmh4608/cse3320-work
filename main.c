#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

/*
	Hoang Ho - 1001654608
	CSE 3320-003
	02/13/2021
*/

static int countdown = 10;

/*
	handles SIGALRM signal - pr
	sig - signal caught
*/
static void alarm_handler(int sig)
{
	printf("%d\n", countdown);
	countdown--;
}

/*
  fork() a child and print a message from the parent and 
  a message from the child
*/
int main()
{
  pid_t pid = fork();

  if( pid == -1 )
  {
    // When fork() returns -1, an error happened.
    perror("fork failed: ");
    exit( EXIT_FAILURE );
  }
  else if ( pid == 0 )
  {
    // When fork() returns 0, we are in the child process.
    
    struct sigaction action;
    memset(&action, '\0', sizeof(action)); //zero out sigaction struct
    
    action.sa_handler = &alarm_handler; //sets the handler to use alarm_handler()
    
    //installing the handler and checking its return value
    if(sigaction(SIGALRM, &action, NULL) < 0)
    {
    	perror("sigaction: ");
    	exit(EXIT_FAILURE);
    }
    
    //loop sets a new alarm for 1 second everytime an alarm finishes and SIGALRM signal is received and calls alarm_handler()
    while(countdown>=0)
    {
    	alarm(1);
    	sleep(1);
    }
    
    fflush(NULL);
    exit( EXIT_SUCCESS );
  }
  else 
  {
    // When fork() returns a positive number, we are in the parent
    // process and the return value is the PID of the newly created
    // child process.
    int status;

    // Force the parent process to wait until the child process 
    // exits
    waitpid(pid, &status, 0 );
    printf("Countdown complete\n");
    fflush( NULL );
  }
  return EXIT_SUCCESS;
}
