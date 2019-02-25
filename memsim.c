/*=======================================================================================
  memsim.c  |    Authors: Griffin Hocker, Eduardo Nodarse
  ---------------------------------------------------------------------------------------
  Project 2 - Simulating page table based off of arguments given
=======================================================================================*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*Creates a boolean variable*/
typedef int bool;
#define true 1
#define false 0

int main(int argc, char *argv[]){

  //Ensures the correct number of arguments are entered
  if(argc != 5){
    printf("ERROR: Need 4 arguments \n");
    return -1;
  }
  
  bool first = true;  //Just used for formatting
  bool debug = false;  //Debug Mode
  
  //Determines if the program is in debug mode
  if(strcmp(argv[4], "debug") == 0){
    debug = true;
    printf("Degbug Mode\n");
  }
  else if(strcmp(argv[4], "quiet") == 0){
    debug = false;
    printf("Quiet Mode\n");
  }
  else{
    printf("ERROR: \'%s\' is not a valid entry\n", argv[4]);
    return -1;
  }
  
  //Determines which Replacement Algorithm should be used
  bool LRU_f = false;
  bool FIFO_f = false;
  bool VMS_f = false;
  
  if(debug)
    printf("Replacement algorithm: ");
    
  if(strcmp(argv[3], "lru") == 0){
    LRU_f = true;
    
    if(debug)
      printf("LRU\n");
  }
  else if(strcmp(argv[3], "fifo") == 0){
    FIFO_f = true;
    
    if(debug)
      printf("FIFO\n");
  }
  else if(strcmp(argv[3], "vms") == 0){
    VMS_f = true;
    
    if(debug)
      printf("VMS\n");
  }
  else{
    printf("ERROR:\'%s\' is not a valid argument\n");
    return -1;
  }
  
  FILE *file = fopen(argv[1], "r");  //Opens the file from argument 1
  
  char string[20];
  int numFrames = atoi(argv[2]);
  
  //If the file does not exist, user must enter correct name of a file in the working dir.
  while (!file){
    //Displays the names entered before prompting new name
    if(first){
      printf("ERROR: No file named %s, please enter a file name\n>>", argv[1]);
      first = false;
    }
    else{
      printf("ERROR: No file named %s, please enter a file name\n>>", string);
    }
    scanf("%s", &string);
    file = fopen(string, "r");
  }
  
  //Ensures a positive number of frames
  while (numFrames <= 0){
    printf("ERROR: %d is an invalid number of frames, needs to be a positive integer\n>>", numFrames);
    scanf("%d", &numFrames); 
  }
  
  //Array of strings to hold the address as a Page Table
  char PageTable[numFrames][10];
  int TableIndex;               //Iterates through the Table
  
  //Initializes an empty table
  for(TableIndex = 0; TableIndex < numFrames; ++TableIndex){
    strcpy(PageTable[TableIndex], "0");
    if(debug)
      printf("%d : %s\n", TableIndex, &PageTable[TableIndex]);   
  }
  
  fclose(file);
  return 0;
}
