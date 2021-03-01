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

#define MAX_NUM_TRACK 15 //max number of pids or history of commands to keep track of

//function prototypes
void printPIDs(pid_t* pids, int pids_pos, int count);
void printHistory(char history[][MAX_COMMAND_SIZE], int history_pos, int count);

int main()
{

  char* cmd_str = (char*)malloc(MAX_COMMAND_SIZE);
  
  //keeping track of the last 15 processes spawned off the shell
  pid_t pids[MAX_NUM_TRACK];
  int pids_pos = 0; //holds the oldest pid or next available slot in the pids list
  int pids_count = 0; //used to see if pids has already been filled up once before

  //keeping track of the last 15 commands
  char history[MAX_NUM_TRACK][MAX_COMMAND_SIZE];
  int history_pos = 0; //holds the oldest command or next available slot in the history
  int history_count = 0; //used to see if history has already been filled up once before

  while(1)
  {
    // Print out the msh prompt
    printf ("msh> ");

    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input
    while( !fgets(cmd_str, MAX_COMMAND_SIZE, stdin) );

    //Parse input
    char* arguments[MAX_NUM_ARGUMENTS];

    int   arg_count = 0;                                 
                                                           
    // Pointer to point to the token
    // parsed by strsep
    char* argument_ptr;                                         
                                                           
    char* working_str  = strdup(cmd_str);

    //if the user types !n (n is a number between 1 - 15)
    //shell re-runs the nth command
    if(working_str[0] == '!')
    {
        int n = atoi(&working_str[1]);

        if(n > history_count || n < 1)
        {
            printf("Command not in history.\n");
            strcpy(working_str, "history\n");
        }
        else
        {
            //since what is displayed to the user isn't the actual index
            //on the array, we have to offset based on
            //the current oldest command
            int index = history_pos+n-1;

            if(index > MAX_NUM_TRACK-1)
            {
                index -= MAX_NUM_TRACK;
            }
            strcpy(working_str, history[index]);
        }
    }

    //reset the current position of the oldest command entered to 0
    //if we have reached the max stored index of history
    if(history_pos > MAX_NUM_TRACK-1)
    {
        history_pos = 0;
    }
    //check if the user did not only press enter
    if(working_str[0] != '\n')
    {
        //set all values in the string to NULL in case a shorter string overwrites it later
        memset(history[history_pos], 0, MAX_COMMAND_SIZE);

        //remove any \n at the end of the working_str if stored by fgets by replacing with a space
        //since fgets will not keep any characters in positions >= MAX_COMMAND_SIZE
        //strcspn will return the number of characters until the first occurrence of \n
        //if no \n are present, the length of the string is returned
        int pos = strcspn(working_str, "\n");
        if(pos != 0)
        {
            working_str[pos] = ' ';
        }
        strcpy(history[history_pos], working_str);
        history_pos++;
        if(history_count < MAX_NUM_TRACK)
        {
            history_count++;
        }
    }
 
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
    if(arguments[0] != NULL)
    {
        if(strcmp(arguments[0], "exit") == 0 || strcmp(arguments[0], "quit") == 0)
        {
            exit(EXIT_SUCCESS);
        }
        else if(strcmp(arguments[0], "cd") == 0) //command for changing directories
        {
            if(chdir(arguments[1]) == -1)
            {
                printf("Invalid directory\n");
            }
        }
        else if(strcmp(arguments[0], "listpids") == 0) //list out pids of last 15 processes spawned off msh.c
        {
            printPIDs(pids, pids_pos, pids_count);
        }
        else if(strcmp(arguments[0], "history") == 0) //list out last 15 commands entered
        {
            printHistory(history, history_pos, history_count);
        }
        else
        {
            //reset the index keeping track of current position we are in the pids[] array to the beginning
            //if we already have 15 pids in the list
            if(pids_pos > MAX_NUM_TRACK-1)
            {
                pids_pos = 0;
            }

            pids[pids_pos] = fork();
            int status;

            if(pids[pids_pos] == -1) //failed fork
            {
                perror("Fork failed: ");
                exit(EXIT_FAILURE);
            }
            else if(pids[pids_pos] == 0) //if we are in the child process
            {
                //execvp uses the file name of the command and vector/list of command options as the parameters
                int ret = execvp(arguments[0], arguments);

                if(ret == -1) //if the execvp failed
                {
                    printf("%s: Command not found\n", arguments[0]);
                    exit(EXIT_FAILURE);
                }
            }
            //otherwise we are in the parent process
            waitpid(pids[pids_pos], &status, 0); //blocking parent process from doing anything until child process returns
            pids_pos++;
            if(pids_count < MAX_NUM_TRACK)
            {
                pids_count++;
            }
        }
    }

    free(working_root);
  }
  return 0;
}

/*
* prints out the list of PIDs of the last 15 processes spawned by msh shell
* 
* pids - array of the last 15 processes
* pids_pos - position of the oldest command in pids or
*            if pids has not been filled up at least once, it is the next available slot in pids
* count - used to see if all MAX_NUM_TRACK entries of pids has been filled up
*/
void printPIDs(pid_t* pids, int pids_pos, int count)
{
    if(count < MAX_NUM_TRACK)
    {
        pids_pos = 0; //in case we have not filled up pids at least once
                      //since pids_pos can also be the next available or empty slot in pids
    }
    
    int i;
    for(i=0; i<count; ++i)
    {
        if(pids_pos > MAX_NUM_TRACK-1)
        {
            pids_pos = 0;
        }
        printf("%d: %d\n", i+1, pids[pids_pos]);
        pids_pos++;
    }
}

/*
* prints out the list of the last 15 commands used
* 
* history - array of previous commands used
* history_pos - the position of the oldest command in history or
                if history has not been filled up at least once, it is the next available slot in history
* count - used to see if all MAX_NUM_TRACK entries of history has been filled up
*/
void printHistory(char history[][MAX_COMMAND_SIZE], int history_pos, int count)
{
    if(count < MAX_NUM_TRACK)
    {
        history_pos = 0; //in the case that we have not filled up history at least once
                         //since history_pos can be the next available slot in history
    }

    int i;
    for(i=0; i<count; ++i)
    {
        if(history_pos > MAX_NUM_TRACK-1)
        {
            history_pos = 0;
        }
        printf("%d: %s\n", i+1, history[history_pos]);
        history_pos++;
    }
}
