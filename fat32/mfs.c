/*
	Hoang Ho - 1001654608
	Joanna Huynh - 1001702615
	CSE 3320-003
	Programming Assignment 4 - FAT32 File System
	Due Tuesday May 4, 2021 by 5:30 PM
	Compile: gcc mfs.c -o mfs
*/

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <stdint.h>
#include <ctype.h>

#define MAX_NUM_ARGUMENTS 3

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size
#define NUM_ITEMS 16 //number of items in each directory

//additional functions
int fileNameCmp(char input[], char fileName[]);


//file and directory representation
typedef struct __attribute__((__packed__)) DirectoryEntry
{
	char name[11];
	uint8_t attribute;
	uint8_t unused1[8];
	uint16_t firstClusterHigh;
	uint8_t unused2[4];
	uint16_t firstClusterLow;
	uint32_t fileSize;
}DIR_Entry;

DIR_Entry directory[16]; //current directory and its 16 entries

int16_t BPB_BytesPerSec;
int8_t BPB_SecPerClus;
int16_t BPB_RsvdSecCnt;
int8_t BPB_NumFATS;
int32_t BPB_FATSz32;

int main()
{
	
	FILE* image;
	FILE* rootDir; //pointer to the root directory
	int isOpen = 0; //to check if file system image is already opened or closed
  char* cmd_str = (char*) malloc( MAX_COMMAND_SIZE );

  while(1)
  {
    //Print out the mfs prompt
    printf ("mfs> ");

    //Read the command from the commandline.  The
    //maximum command that will be read is MAX_COMMAND_SIZE
    //This while command will wait here until the user
    //inputs something since fgets returns NULL when there
    //is no input
    while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) );

    /*Parse input*/
    char *arguments[MAX_NUM_ARGUMENTS];

    int   arg_count = 0;                                 
                                                           
    //Pointer to point to the token
    //parsed by strsep
    char *arg_ptr;                                         
                                                           
    char *working_str  = strdup( cmd_str );                

    //we are going to move the working_str pointer so
    //keep track of its original value so we can deallocate
    //the correct amount at the end
    char *working_root = working_str;

    //Tokenize the input stringswith whitespace used as the delimiter
    while ( ( (arg_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) && 
              (arg_count<MAX_NUM_ARGUMENTS))
    {
      arguments[arg_count] = strndup( arg_ptr, MAX_COMMAND_SIZE );
      if( strlen( arguments[arg_count] ) == 0 )
      {
        arguments[arg_count] = NULL;
      }
      arg_count++;
    }

    
    //FAT32 functionality
    if(arguments[0] != NULL)
    {
    	if(strcmp(arguments[0], "open") == 0) //open a fat32 image
    	{
    		
    		if(isOpen) //check if a file system is already open
    		{
    			printf("Error: File system image already open.\n");
    		}
    		else
    		{
    			//open the file system image
    			image = fopen(arguments[1], "r");
    			
		  		if(image == NULL)
		  		{
		  			printf("Error: File system image not found.\n"); 
					}
					else
					{
						isOpen = 1;
						
						//read in reserved sectors
						//move to where the start of count of bytes per sector is stored from the beginning
						fseek(image, 11, SEEK_SET);
						fread(&BPB_BytesPerSec, 2, 1, image);
						
						//Sectors per Cluster
						fseek(image, 13, SEEK_SET);
						fread(&BPB_SecPerClus,  1, 1, image);
						
						//Reserved Sector Count
						fseek(image, 14, SEEK_SET);
						fread(&BPB_RsvdSecCnt, 2, 1, image);
						
						//Number of FATS
						fseek(image, 16, SEEK_SET);
						fread(&BPB_NumFATS, 1, 1, image);
						
						//FAT32 Size
						fseek(image, 36, SEEK_SET);
						fread(&BPB_FATSz32, 4, 1, image);
						
						//go to the root directory and read in its contents
						//each directory only has at most 16 items (NUM_ITEMS)
						fseek(image, ((BPB_NumFATS*BPB_FATSz32*BPB_BytesPerSec)+(BPB_RsvdSecCnt*BPB_BytesPerSec)), SEEK_SET);
						rootDir = image;
						fread(&directory, sizeof(DIR_Entry), NUM_ITEMS, image);
					}
    		}
    	}
    	else if(strcmp(arguments[0], "close") == 0) //close fat32 image
			{
				//check if a file system not currently open
				if(isOpen)
				{
					fclose(image);
					isOpen = 0;
				}
				else
				{
					printf("Error: File system not open\n");
				}
			}
    	else if(!isOpen) //check if a file system is open before processing any other commands after a close
    	{
				printf("Error: File system image must be opened first\n");
    	}

    	else if(strcmp(arguments[0], "info") == 0)
    	{
    		//print out information about file system in hexadecimal and base 10
				printf("BPB_BytesPerSec: %5d%5x\n", BPB_BytesPerSec, BPB_BytesPerSec);
		  	printf("BPB_SecPerClus: %5d%5x\n", BPB_SecPerClus, BPB_SecPerClus);
		  	printf("BPB_RsvdSecCnt: %5d%5x\n", BPB_RsvdSecCnt, BPB_RsvdSecCnt);
		  	printf("BPB_NumFATS: %5d%5x\n", BPB_NumFATS, BPB_NumFATS);
		  	printf("BPB_FATSz32: %5d%5x\n", BPB_FATSz32, BPB_FATSz32);
			}
			else if(strcmp(arguments[0], "stat") == 0)
			{
						int i=0;

						//go through directory items/files and
						//print out the data about the file/directory the user inputted
						for(i=0; i<NUM_ITEMS; i++)
						{
							char filename[12];
							strncpy(filename, &directory[i].name[0], 11);
							filename[12] = '\0';
							
							char input[strlen(arguments[1])];
							strcpy(input, arguments[1]);
							
							if(fileNameCmp(input, filename) == 0)
							{
								printf("Attribute: %d\n", directory[i].attribute);
								printf("File Size: %d\n", directory[i].fileSize);
								printf("First Cluster Low: %d\n", directory[i].firstClusterLow);
								printf("First Cluster High: %d\n", directory[i].firstClusterHigh);
								break;
							}
						}

						if(i>15)
						{
							printf("Error: File not found\n");
						}
			}
		}
			
    
    //Now print the tokenized input as a debug check
    int arg_index  = 0;
    for( arg_index = 0; arg_index < arg_count; arg_index ++ ) 
    {
      printf("arguments[%d] = %s\n", arg_index, arguments[arg_index] );  
    }

    free( working_root );

  }
  return 0;
}

/*
*compares the user inputted file/directory name to current file/directory
*
*input - user inputted file/directory name
*fileName - file/directory name currently checking to see if it matches user input
*
*returns 0 if the file/directory name matches user input
*returns -1 if it doesn't match 
*/
int fileNameCmp(char input[], char fileName[])
{
	char expandedName[12];
	memset(expandedName, ' ', 12);

	//get file name before extension
	char* token = strtok(input, ".");
	strncpy(expandedName, token, strlen(token));
	
	//place the extension name onto the expanded name for the input
	token = strtok(NULL,".");
	if(token)
	{
		strncpy(&expandedName[11-strlen(token)], token, strlen(token));
	}
	expandedName[11] = '\0';
	
	int i;
	for(i=0; i<11; i++)
	{
		expandedName[i] = toupper(expandedName[i]);
	}
	
	if(strncmp(expandedName, fileName, 11) == 0)
	{
		return 0; //they matched
	}
	
	return -1; //did not match
}
