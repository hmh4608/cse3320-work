#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

/*
	Hoang Ho - 1001654608
	CSE 3320-003
	02/13/2021
*/

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
    
    //loop decrements countdown each time it iterates and prints out remaining time, sleep(1 second) is used to make sure the next countdown statement does not execute until 1 second has passed 
    int countdown = 10;
    while(countdown >= 0)
    {
    	printf("%d\n", countdown);
    	countdown--;
    	sleep(1); //delay for 1 second
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
