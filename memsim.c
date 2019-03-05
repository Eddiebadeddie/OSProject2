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

struct CleanDirtyBits
{
  bool p1;
  bool p2;
  bool clean;
  bool dirty;
};

void LRU(FILE *file, char PageTable[][10], int numFrames, bool debug);
void FIFO(FILE *file, char PageTable[][10], int numFrames, bool debug);
void VMS(FILE *file, char PageTable[][100], int numFrames, bool debug);

int main(int argc, char *argv[])
{

  //Ensures the correct number of arguments are entered
  if (argc != 5)
  {
    printf("ERROR: Need 4 arguments \n");
    return -1;
  }

  bool first = true;  //Just used for formatting
  bool debug = false; //Debug Mode

  //Determines if the program is in debug mode
  if (strcmp(argv[4], "debug") == 0)
  {
    debug = true;
    printf("Degbug Mode\n");
  }
  else if (strcmp(argv[4], "quiet") == 0)
  {
    debug = false;
  }
  else
  {
    printf("ERROR: \'%s\' is not a valid entry\n", argv[4]);
    return -1;
  }

  //Determines which Replacement Algorithm should be used
  bool LRU_f = false;
  bool FIFO_f = false;
  bool VMS_f = false;

  if (debug)
    printf("Replacement algorithm: ");

  if (strcmp(argv[3], "lru") == 0)
  {
    LRU_f = true;

    if (debug)
      printf("LRU\n");
  }
  else if (strcmp(argv[3], "fifo") == 0)
  {
    FIFO_f = true;

    if (debug)
      printf("FIFO\n");
  }
  else if (strcmp(argv[3], "vms") == 0)
  {
    VMS_f = true;

    if (debug)
      printf("VMS\n");
  }
  else
  {
    printf("ERROR:\'%s\' is not a valid argument\n");
    return -1;
  }

  FILE *file = fopen(argv[1], "r"); //Opens the file from argument 1

  char string[20];
  int numFrames = atoi(argv[2]);

  //If the file does not exist, user must enter correct name of a file in the working dir.
  while (!file)
  {
    //Displays the names entered before prompting new name
    if (first)
    {
      printf("ERROR: No file named %s, please enter a file name\n>>", argv[1]);
      first = false;
    }
    else
    {
      printf("ERROR: No file named %s, please enter a file name\n>>", string);
    }
    scanf("%s", &string);
    file = fopen(string, "r");
  }

  //Ensures a positive number of frames
  while (numFrames <= 0)
  {
    printf("ERROR: %d is an invalid number of frames, needs to be a positive integer\n>>", numFrames);
    scanf("%d", &numFrames);
  }

  //Array of strings to hold the address as a Page Table
  char PageTable[numFrames][10];
  int i; //Iterates through the Table

  //Initializes an empty table
  for (i = 0; i < numFrames; ++i)
  {
    strcpy(PageTable[i], "0");
    if (debug)
      printf("%d : %s\n", i, &PageTable[i]);
  }

  if (LRU_f)
  {
    LRU(file, PageTable, numFrames, debug);
  }

  if (FIFO_f)
  {
    FIFO(file, PageTable, numFrames, debug);
  }

  fclose(file);
  return 0;
}

/*=======================================================================================
  LRU  |    Authors: Eduardo Nodarse
  ---------------------------------------------------------------------------------------
    Takes in the file, the page table, the number of frames and if debug mode is active
  and will perform the Least Recently Used (LRU) replacement algorithm on the page table.
=======================================================================================*/
void LRU(FILE *file, char PageTable[][10], int numFrames, bool debug)
{
  char string[20]; //string buffer to hold the string
  char rw;         //holds the read or write operation on the address

  int i;
  int eventCounter = 0; //Counts the number of addresses accessed
  int pageHit = 0;

  //Keeps track of which element of the page table was accessed at which moment.
  int event[numFrames];

  int RW[2]; //Keeps track of the number of R and W operations
  RW[0] = 0; //Read
  RW[1] = 0; //Write

  /*
    Goes through the file and reads in the addresses from the trace file.
  */
  while (fscanf(file, "%s %c", &string, &rw) != EOF)
  {
    if (debug)
      printf("Performing %c on %s\n", rw, string);

    /*
      If the value of the page table at the given index is the initialized value, then
      store the string in the page table.
    */
    bool present = false;
    int index;

    for (i = 0; i < numFrames; ++i)
    {
      if (strcmp(PageTable[i], string) == "0")
      {
        present = true;
        index = i;
        break;
      }
    }

    if (present)
    {
      event[i] = eventCounter;
      ++eventCounter;
      ++pageHit;
      continue;
    }

    if (strcmp(PageTable[eventCounter % numFrames], "0") == 0)
    {
      strcpy(PageTable[eventCounter % numFrames], string);
      event[eventCounter % numFrames] = eventCounter;

      if (debug)
        printf("PageTable[%d] = %s at %d\n", eventCounter % numFrames, string, eventCounter);
    }
    else
    {
      //The table has already been populated
      if (debug)
      {
        printf("Table is full\n");
      }

      present = false; //Determines if the string is already in memory

      /*
        Goes through all of the entries in the page table
      */
      for (i = 0; i < numFrames; ++i)
      {
        //If the entry in the page table is the same as the string, then page hit
        if (strcmp(PageTable[i], string) == 0)
        {
          if (debug)
          {
            printf("Page hit\n");
            ++pageHit;
          }
          present = true;
          event[i] = eventCounter; //Update the event
          if (debug)
            printf("event[%d] = %d\n", i, event[i]);

          break;
        }
      }

      //If the string is not an entry in the page table
      if (present == false)
      {
        if (debug)
          printf("%s is not present\n", string);
        int min = 0; //Initial index, used for comparison in search for LRU

        /*
          Finds LRU in events
        */
        for (i = 1; i < numFrames; ++i)
        {
          //When it finds the LRU, replaces min
          if (event[i] < event[min])
          {
            min = i;
          }
        }
        if (debug)
          printf("PageTable[%d] was %s ", min, PageTable[min]);
        strcpy(PageTable[min], string); //Copies string in the place of LRU

        if (debug)
          printf("is now %s at %d\n", PageTable[min], eventCounter);
        event[min] = eventCounter; //Updates event at the min

        if (debug)
          printf("event[%d] = %d\n", min, event[min]);
      }
    }

    if (rw == 'R' || rw == 'r')
    {
      RW[0] += 1; //Increments if this was a read operation
    }
    else if (rw == 'W' || rw == 'w')
    {
      RW[1] += 1; //Increments if this was a write operation
    }

    ++eventCounter;
  }

  printf("total memory frames: %d\n", numFrames);
  printf("events in trace: %d\n", eventCounter);
  printf("total disk reads: %d\n", RW[0]);
  printf("total disk writes: %d\n", RW[1]);

  double percentage = (double)pageHit / (double)eventCounter;
  //if (debug)
  //{
  printf("page hits: %d\n", pageHit);
  printf("hit rate: %f %%", percentage * 100);
  //}
}

void FIFO(FILE *file, char PageTable[][10], int numFrames, bool debug)
{
  char string[20]; //string buffer to hold the string
  char rw;         //holds the read or write operation on the address

  int i;
  int eventCounter = 0; //Counts the number of addresses accessed
  int pageHit = 0;      //Counts the number of page hits

  //Keeps track of which element of the page table was accessed at which moment.
  int event[numFrames];

  int RW[2]; //Keeps track of the number of R and W operations
  RW[0] = 0; //Read
  RW[1] = 0; //Write

  /*
    Goes through the file and reads in the addresses from the trace file.
  */
  while (fscanf(file, "%s %c", &string, &rw) != EOF)
  {
    if (debug)
      printf("Performing %c on %s\n", rw, string);

    /*
      If the value of the page table at the given index is the initialized value, then
      store the string in the page table.
    */

    bool present = false;
    int index;

    for (i = 0; i < numFrames; ++i)
    {
      if (strcmp(PageTable[i], string) == "0")
      {
        present = true;
        index = i;
        break;
      }
    }

    if (present)
    {
      event[i] = eventCounter;
      ++eventCounter;
      ++pageHit;
      continue;
    }

    if (strcmp(PageTable[eventCounter % numFrames], "0") == 0)
    {
      strcpy(PageTable[eventCounter % numFrames], string);
      event[eventCounter % numFrames] = eventCounter;

      if (debug)
        printf("PageTable[%d] = %s at %d\n", eventCounter % numFrames, string, eventCounter);
    }
    else
    {
      //The table has already been populated
      if (debug)
      {
        printf("Table is full\n");
      }

      bool present = false; //Determines if the string is already in memory

      /*
        Goes through all of the entries in the page table
      */
      for (i = 0; i < numFrames; ++i)
      {
        //If the entry in the page table is the same as the string, then page hit
        if (strcmp(PageTable[i], string) == 0)
        {
          if (debug)
          {
            printf("Page hit\n");
            ++pageHit;
          }
          present = true;
          if (debug)
            printf("event[%d] = %d\n", i, event[i]);

          break;
        }
      }

      //If the string is not an entry in the page table
      if (present == false)
      {
        if (debug)
          printf("%s is not present\n", string);
        int min = 0;

        for (i = 1; i < numFrames; ++i)
        {
          if (event[i] < event[min])
          {
            min = i;
          }
        }
        if (debug)
          printf("PageTable[%d] was %s ", min, PageTable[min]);
        strcpy(PageTable[min], string);

        if (debug)
          printf("is now %s at %d\n", PageTable[min], eventCounter);
        event[min] = eventCounter; //Updates event at the min

        if (debug)
          printf("event[%d] = %d\n", min, event[min]);
      }
    }

    if (rw == 'R' || rw == 'r')
    {
      RW[0] += 1; //Increments if this was a read operation
    }
    else if (rw == 'W' || rw == 'w')
    {
      RW[1] += 1; //Increments if this was a write operation
    }

    ++eventCounter;
  }

  printf("total memory frames: %d\n", numFrames);
  printf("events in trace: %d\n", eventCounter);
  printf("total disk reads: %d\n", RW[0]);
  printf("total disk writes: %d\n", RW[1]);

  double percentage = (double)pageHit / (double)eventCounter;
  //if (debug)
  //{
  printf("page hits: %d\n", pageHit);
  printf("hit rate: %f %%", percentage * 100);
  //}
}

void VMS(FILE *file, char PageTable[][100], int numFrames, bool debug)
{
  if (numFrames < 4)
  {
    printf("Error: Cannot perform this action with less than 4 frames.\n");
    return;
  }

  //Keeps track of where each process begins and ends
  int limit = numFrames / 2;

  int A = 0;
  int B = 0;

  int RW[2];
  RW[0] = 0;
  RW[1] = 0;

  int i;

  char string[20];
  char rw;

  int eventCounter = 0;
  int event[numFrames];
  struct CleanDirtyBits list[numFrames];

  int pageHit;
  /*
    Goes through the file and reads in the addresses from the trace file.
  */
  while (fscanf(file, "%s %c", &string, &rw) != EOF)
  {
    if (debug)
      printf("Performing %c on %s\n", rw, string);

    bool present = false;
    int index;

    for (i = 0; i < numFrames; ++i)
    {
      if (strcmp(PageTable[i], string) == "0")
      {
        present = true;
        index = i;
        break;
      }
    }

    if (present)
    {
      event[i] = eventCounter;
      ++eventCounter;
      ++pageHit;
      continue;
    }

    if (strcmp(PageTable[eventCounter % numFrames], "0") == 0)
    {
      strcpy(PageTable[eventCounter % numFrames], string);

      if (string[0] == '3')
      {
        list[eventCounter % numFrames].p1 = true;
        list[eventCounter % numFrames].p2 = false;
        if (A == limit)
        {
          int min = event[0];
          int index = 0;
          for (i = 1; i < numFrames; ++i)
          {
            if (!list[index].p1)
            {
              min = event[i];
              index = i;
            }
            if (event[i] < min && list[i].p1)
            {
              min = event[i];
              index = i;
            }
          }

          list[index].p1 = false;
          list[index].p2 = false;
        }
        else
        {
          ++A;
        }
      }
      else
      {
        list[eventCounter % numFrames].p1 = false;
        list[eventCounter % numFrames].p2 = true;
        if (B == limit)
        {
          int min = event[0];
          int index = 0;
          for (i = 1; i < numFrames; ++i)
          {
            if (!list[index].p1)
            {
              min = event[i];
              index = i;
            }
            if (event[i] < min && list[i].p1)
            {
              min = event[i];
              index = i;
            }
          }

          list[index].p1 = false;
          list[index].p2 = false;
          if (rw == 'R' || rw == 'r')
          {
            list[index].clean = true;
            list[index].dirty = false;
          }
          else if (rw == 'W' | rw = 'w')
          {
            list[index].clean = false;
            list[index].dirty = true;
          }
        }
        else
        {
          ++B;
        }
      }
      event[eventCounter % numFrames] = eventCounter;
    }
    else
    {
      printf("Table is full\n");

      bool present;

      for (i = 0; i < numFrames; ++i)
      {
        if (strcmp(PageTable[eventCounter % numFrames], string) == 0)
        {
          present = true;
          ++pageHit;
          break;
        }
      }

      if (!present)
      {
        int min = event[0];
        int index = 0;
        bool clean = false;
        bool dirty = false;

        for (i = 0; i < numFrames; ++i)
        {
          if (list[i].clean)
          {
            if (min > event[i])
            {
              min = event[i];
              index = i;
              clean = true;
            }
          }
        }

        if (clean)
        {
          if (PageTable[index][0] == '3')
          {
            list[index].p1 = true;
            list[index].p2 = false;
          }
          else
          {
            list[index].p1 = false;
            list[index].p2 = true;
          }
          list[eventCounter % numFrames].p1 = false;
          list[eventCounter % numFrames].p2 = false;
          event[index] = eventCounter;
        }
      }
    }

    if (rw == 'R' || rw == 'r')
    {
      RW[0] += 1; //Increments if this was a read operation
      list[eventCounter % numFrames].clean = true;
      list[eventCounter % numFrames].dirty = false;
    }
    else if (rw == 'W' || rw == 'w')
    {
      RW[1] += 1; //Increments if this was a write operation
      list[eventCounter % numFrames].clean = false;
      list[eventCounter % numFrames].dirty = true;
    }

    ++eventCounter;
  }

  printf("total memory frames: %d\n", numFrames);
  printf("events in trace: %d\n", eventCounter);
  printf("total disk reads: %d\n", RW[0]);
  printf("total disk writes: %d\n", RW[1]);

  double percentage = (double)pageHit / (double)eventCounter;
  if (debug)
  {
    printf("page hits: %d\n", pageHit);
    printf("hit rate: %f %%", percentage * 100);
  }
}