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

#define MAX_NUM_ARGUMENTS 4

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size
#define NUM_ITEMS 16 //number of items in each directory

//additional functions
int fileNameCmp(char input[], char fileName[]);
int LBAToOffset(int32_t cluster);
int16_t nextLB(uint32_t cluster, FILE* fp);


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

DIR_Entry directory[NUM_ITEMS]; //current directory and its 16 entries

int16_t BPB_BytesPerSec;
int8_t BPB_SecPerClus;
int16_t BPB_RsvdSecCnt;
int8_t BPB_NumFATS;
int32_t BPB_FATSz32;

int main()
{
	
	FILE* image;
	int rootDirCluster; //variable to hold the root directory's low cluster
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
    	if(strcmp(arguments[0], "exit") == 0) //exits the program
      {
        exit(0);
      }

    	else if(strcmp(arguments[0], "open") == 0) //open a fat32 image
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
            fread(&directory, sizeof(DIR_Entry), NUM_ITEMS, image);
						rootDirCluster = directory[0].firstClusterLow;
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
				printf("%-18s %-7d%-7x\n", "BPB_BytesPerSec:", BPB_BytesPerSec, BPB_BytesPerSec);
			  printf("%-18s %-7d%-7x\n", "BPB_SecPerClus:", BPB_SecPerClus, BPB_SecPerClus);
			  printf("%-18s %-7d%-7x\n", "BPB_RsvdSecCnt:", BPB_RsvdSecCnt, BPB_RsvdSecCnt);
			  printf("%-18s %-7d%-7x\n", "BPB_NumFATS:", BPB_NumFATS, BPB_NumFATS);
			  printf("%-18s %-7d%-7x\n", "BPB_FATSz32:", BPB_FATSz32, BPB_FATSz32);
			}
			
			else if(strcmp(arguments[0], "stat") == 0)
			{
				int i = 0;
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

				if(i>NUM_ITEMS-1)
				{
					printf("Error: File not found\n");
				}
			}
			
			else if(strcmp(arguments[0], "read") == 0)
			{
				int i = 0;
				//go through directory items/files and find the matching file
				for(i=0; i<NUM_ITEMS; i++)
				{
					char filename[12];
					strncpy(filename, &directory[i].name[0], 11);
					filename[12] = '\0';
					
					char input[strlen(arguments[1])];
					strcpy(input, arguments[1]);
							
					if(fileNameCmp(input, filename) == 0)
					{
						break;
					}
				}
				
				if(i>NUM_ITEMS-1)
				{
					printf("Error: File not found\n");
				}
				
				else
				{
					//read from the file at the given position (in bytes) and output number of bytes specified
					//arguments[2] - position
					//arguments[3] - number of bytes to be read
					int nextCluster = directory[i].firstClusterLow;
					int position = atoi(arguments[2]);
					int remainingBytes = atoi(arguments[3]); //remaining number of bytes needed to be read
					
					//each cluster/file is chopped up into certain number of sectors each with a certain number of bytes for each block
					int clusterSize = BPB_BytesPerSec*BPB_SecPerClus;
					unsigned char data[clusterSize]; //buffer to read in data
					
					//if the position user wants to read from is at a position larger than
					//the size of at least one block
					//loop until we get to the right starting block
					if(position > clusterSize)
					{
						while(position > clusterSize)
						{
							position -= clusterSize;
							nextCluster = nextLB(nextCluster, image);
						}
						
						printf("position: %d next cluster: %d offset: %d\n", position, nextCluster, LBAToOffset(nextCluster)+position);
						
						//if the number of bytes user wants read goes beyond the current block
						//read in the rest of the block starting from the position first
						if(remainingBytes > clusterSize-position)
						{
							fseek(image, LBAToOffset(nextCluster)+position, SEEK_SET);
							memset(data, '\0', clusterSize);
							fread(&data[0], clusterSize-position, 1, image);
							for(i=0; i<clusterSize-position; i++)
							{
								printf("%x ", data[i]);
							}
							//reset position to 0 since from now on
							//we'll be reading each cluster from the beginning
							position = 0;
						}
					}
					
					//continuously read in blocks of data as much as possible until we no longer need  to continuously look up
					//the next blocks in FAT
					while(remainingBytes > clusterSize)
					{
						fseek(image, LBAToOffset(nextCluster), SEEK_SET);
						//read-write 1 item of total bytes in one cluster
						fread(&data[0], clusterSize, 1, image);
						
						for(i=0; i<clusterSize; i++)
						{
							printf("%x ", data[i]);
						}
								
						nextCluster = nextLB(nextCluster, image);
						remainingBytes -= clusterSize;
					}
						
					//remainder cluster to write to file
					if(remainingBytes > 0)
					{
						fseek(image, LBAToOffset(nextCluster)+position, SEEK_SET);
						memset(data, '\0', clusterSize);
						fread(&data[0], remainingBytes, 1, image);
						
						for(i=0; i<remainingBytes; i++)
						{
							printf("%x ", data[i]);
						}
					}
					
					printf("\n");
				}
			}
			
			else if(strcmp(arguments[0], "get") == 0)
			{
				int i = 0;
				char fname[12]; //for converting the file name to proper name form to write to
				memset(fname, '\0', 12);
				
				//go through directory items/files and find the matching file
				for(i=0; i<NUM_ITEMS; i++)
				{
					char filename[12];
					strncpy(filename, &directory[i].name[0], 11);
					filename[12] = '\0';
					
					char input[strlen(arguments[1])];
					strcpy(input, arguments[1]);
							
					if(fileNameCmp(input, filename) == 0)
					{
						//convert the file name to proper form
						char* token = strtok(filename, " \t");
						if(strlen(filename) == 11) //in case file name takes up max file name length
						{
							strncpy(fname, filename, strlen(filename)-3); //assuming the extension is 3 letters long or .txt
							fname[strlen(filename)-3] = '.';
							strcpy(&fname[strlen(filename)-2], &filename[(strlen(filename)-3)]); //read in the rest of the extension name in the file name
						}
						
						else
						{
							strncpy(fname, token, strlen(token));
							fname[strlen(token)] = '.';
							token = strtok(NULL, " \t"); //skip all spaces
							if(token)
							{
								strncpy(&fname[strlen(fname)], token, strlen(token));
							}
						}
						
						//turn the all the letters to lowercase to match user input
						int j;
						for(j=0; j<strlen(fname); j++)
						{
							fname[j] = tolower(fname[j]);
						}
						break;
					}
				}
				
				if(i>NUM_ITEMS-1)
				{
					printf("Error: File not found\n");
				}
				
				else
				{
					//create a new file in current working directory
					FILE* outputFile = fopen(fname, "w");
					
					//retrieve the file from the FAT32 image and place it in current working directory
					
					int nextCluster = directory[i].firstClusterLow; //starting cluster number
					int remainingBytes = directory[i].fileSize; //remaining number of bytes needed to be read
					
					//each cluster/file is chopped up into certain number of sectors each with a certain number of bytes for each block
					int clusterSize = BPB_BytesPerSec*BPB_SecPerClus;
					unsigned char data[clusterSize]; //buffer to read in data
					
					//continuously read in blocks of data as much as possible until we no longer need  to continuously look up
					//the next blocks in FAT
					while(remainingBytes > clusterSize)
					{
						fseek(image, LBAToOffset(nextCluster), SEEK_SET);
						//read-write 1 item of total bytes in one cluster
						fread(&data[0], clusterSize, 1, image);
						fwrite(data, clusterSize, 1, outputFile);
							
						nextCluster = nextLB(nextCluster, image);
						remainingBytes -= clusterSize;
					}
					
					//remainder cluster to write to file
					if(remainingBytes > 0)
					{
						fseek(image, LBAToOffset(nextCluster), SEEK_SET);
						memset(data, '\0', clusterSize);
						fread(&data[0], remainingBytes, 1, image);
						fwrite(data, remainingBytes, 1, outputFile);
					}
					
					fclose(outputFile);
				}
			}
			
			else if(strcmp(arguments[0], "ls") == 0) //lists the directory contents
			{
				int i=0;

				//go through the directory and lists its contents
				//given that it is not a hidden, deleted, or a system file
				for(i=0; i<NUM_ITEMS; i++)
				{
					char filename[12];
					strncpy(filename, &directory[i].name[0], 11);
					filename[12] = '\0';

					if( (directory[i].attribute == 0x01 || directory[i].attribute == 0x10 || directory[i].attribute == 0x20) && directory[i].name[0] != (char)0xe5)
					{
						printf("%s\n", filename);
					}
				}
			}
			
      else if(strcmp(arguments[0], "cd") == 0) //change the working directory to the users specified directory
      {
        //changing back to the previous working directory
        if(strcmp(arguments[1], "..") == 0) 
        {   
          int prevOffset;
          int root = 0;
          
          //find the offset of the previous working directory and fseek to it
          //if the working directory is the root directory then the command cannot be executed
          if (directory[0].firstClusterLow == rootDirCluster)
          { 
              root = 1;
              printf("Error: This is the root directory\n");
          }
          
          else if (directory[1].firstClusterLow == 0)
          {
            prevOffset = LBAToOffset(2);
          }
          
          else
          {
            prevOffset = LBAToOffset(directory[1].firstClusterLow);
          }
          
          if(!root)
          {
            fseek(image, prevOffset, SEEK_SET);
            fread(&directory, sizeof(DIR_Entry), NUM_ITEMS, image);
          }
        }
        
        //change to a directory within the working directory
				else{
				  int i = 0;
				  
				  //for each item in the directory, check to see if a file/directory name matches the user's input
		      for(i=0; i<NUM_ITEMS; i++)
					{
						char filename[12];
		        strncpy(filename, &directory[i].name[0], 11);
						filename[12] = '\0';

						char input[strlen(arguments[1])];
						strcpy(input, arguments[1]);
		         
            
					 	if(fileNameCmp(input, filename) == 0)
		        {
		          if(directory[i].attribute == 0x20 || directory[i].attribute == 0x01)
		          {
                //an error when  the user inputs a filename that is not a directory
		            printf("Error: %s is not a directory\n", arguments[1]);
		            break;
		          }
		          else
		          {
                //change to the directory if it is found in the working directory
		            int newDirCluster = directory[i].firstClusterLow;
		            int newDirOffset = LBAToOffset(newDirCluster);
		            fseek(image, newDirOffset, SEEK_SET);
		            fread(&directory, sizeof(DIR_Entry), NUM_ITEMS, image);
		            break;
		          }
		        }
		      }  
					if(i>15)
					{
						printf("Error: Directory not found\n");
					}
				}
			}
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

/*
*finds the starting address of a block of data given the sector number
*cluster - current cluster number that points to a block of data
*returns the value of the address for that block of data
*/
int LBAToOffset(int32_t cluster)
{
	return ((cluster-2) * BPB_BytesPerSec) + (BPB_BytesPerSec*BPB_RsvdSecCnt) + (BPB_NumFATS * BPB_FATSz32 * BPB_BytesPerSec);
}

/*
*given a logical block address, look up in the first FAT
*return the next logical block address of the block in the file
*return -1 if there is no further blocks
*
*cluster - current sector number that points to a block of data
*fp - file system image
*/
int16_t nextLB(uint32_t cluster, FILE* fp)
{
		uint32_t FATAddress = (BPB_BytesPerSec * BPB_RsvdSecCnt) + (cluster * 4);
		int16_t val;
		fseek(fp, FATAddress, SEEK_SET);
		fread(&val, 2, 1, fp);
		return val;
}
