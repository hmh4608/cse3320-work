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

int main()
{

  char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );

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
    char *arguments[MAX_NUM_ARGUMENTS];

    int   arg_count = 0;                                 
                                                           
    // Pointer to point to the token
    // parsed by strsep
    char *argument_ptr;                                         
                                                           
    char *working_str  = strdup( cmd_str );                

    // we are going to move the working_str pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *working_root = working_str;

    // Tokenize the input strings with whitespace used as the delimiter
    while( ( (argument_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) && 
              (token_count<MAX_NUM_ARGUMENTS) )
    {
      token[token_count] = strndup( argument_ptr, MAX_COMMAND_SIZE );
      if( strlen( arguments[arg_count] ) == 0 )
      {
        arguments[arg_count] = NULL;
      }
      arg_count++;
    }

    if(arguments[0] == "exit" || arguments[0] == "quit")
    {
        exit(EXIT_SUCCESS);
    }
    else if(arguments[0] != NULL)
    {
        pid_t pid = fork();
        int status;

        if (pid == -1) //failed fork
        {
            perror("fork failed: ");
            exit(EXIT_FAILURE);
        }
        else if (pid == 0) //if we are in the child process
        {
            int ret = execvp(arguments[0], arguments);

            if (ret = -1) //if the execvp failed
            {
                printf("%s: Command not found\n", arguments[0]);
                exit(EXIT_SUCCESS);
            }
        }
        //otherwise we are in the parent process
        waitpid(pid, &status, 0); //blocking parent process from doing anything until child process returns
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
