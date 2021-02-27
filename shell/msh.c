/*
* Name: Hoang Ho
* ID: 1001654608
* CSE 3320-003
* Due March 1, 2021 by 5:30 pm
*/

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 10     // Mav shell only supports ten arguments

#define MAX_NUM_PIDS 15 //max number of pids to keep track of

int main()
{

  char* cmd_str = (char*) malloc( MAX_COMMAND_SIZE );

  //keeping track of the last 15 processes spawned off the shell
  pid_t pids[MAX_NUM_PIDS];
  int pids_index = 0;

  while( 1 )
  {
    // Print out the msh prompt
    printf ("msh> ");

    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input
    while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) );

    /* Parse input */
    char* arguments[MAX_NUM_ARGUMENTS];

    int   arg_count = 0;                                 
                                                           
    // Pointer to point to the token
    // parsed by strsep
    char* argument_ptr;                                         
                                                           
    char* working_str  = strdup( cmd_str );                

    // we are going to move the working_str pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char* working_root = working_str;

    // Tokenize the input strings with whitespace used as the delimiter
    while( ( (argument_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) && 
              (arg_count<MAX_NUM_ARGUMENTS) )
    {
      arguments[arg_count] = strndup( argument_ptr, MAX_COMMAND_SIZE );
      if( strlen( arguments[arg_count] ) == 0 )
      {
        arguments[arg_count] = NULL;
      }
      arg_count++;
    }

    //execute a process depending on the inputs user has given
    //arguments[0] is the command to execute a process
    //arguments[1...n] is all the command options to the command arguments[0]
    if(strcmp(arguments[0], "exit") == 0 || strcmp(arguments[0], "quit") == 0)
    {
        exit(EXIT_SUCCESS);
    }
    else if(strcmp(arguments[0], "cd")) //command for changing directories
    {
        if(!chdir(arguments[1]))
        {
            printf("Invalid directory\n");
        }
    }
    else if(strcmp(arguments[0], "listpids")) //list out pids of last 15 processes spawned off msh.c
    {
        int i;
        for(i = 0; i < pids_index; ++i)
        {
            printf("%d: %d\n", i, pids[i]);
        }
    }
    else if(arguments[0] != NULL)
    {
        pids[pids_index++] = fork();
        //reset the index keeping track of current position we are in the pids[] array to the beginning
        //if we already have 15 pids in the list
        if (pids_index > 14)
        {
            pids_index = 0;
        }

        int status;

        if(pids[pids_index] == -1) //failed fork
        {
            perror("Fork failed: ");
            exit(EXIT_FAILURE);
        }
        else if(pids[pids_index] == 0) //if we are in the child process
        {
            int ret = execvp(arguments[0], arguments);

            if(ret == -1) //if the execvp failed
            {
                printf("%s: Command not found\n", arguments[0]);
                exit(EXIT_SUCCESS);
            }
        }
        //otherwise we are in the parent process
        waitpid(pids[pids_index], &status, 0); //blocking parent process from doing anything until child process returns
    }

    int arg_index  = 0;
    for( arg_index = 0; arg_index < arg_count; arg_index ++ ) 
    {
      printf("arguments[%d] = %s\n", arg_index, arguments[arg_index] );  
    }

    free( working_root );
  }
  return 0;
}
