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

struct Node
{
  char string[10];
  bool clean;
  struct Node *n;
  struct Node *p;
};

void LRU(FILE *file, char PageTable[][10], int numFrames, bool debug);
void FIFO(FILE *file, char PageTable[][10], int numFrames, bool debug);
void VMS(FILE *file, int numFrames, bool debug);

struct Node *FindLastNode(struct Node *cur);
struct Node *FindPage(struct Node *cur, char string[10]);
struct Node *AddToList(struct Node *add, struct Node *list);
struct Node *RemoveFromList(struct Node *remove, struct Node *list);
void UpdateRW(struct Node *node, int *RW, char rw);

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
      if (strcmp(PageTable[i], "0") == 0)
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
      if (strcmp(PageTable[i], "0") == 0)
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

void VMS(FILE *file, int numFrames, bool debug)
{
  if (numFrames < 4)
  {
    printf("Error: Cannot perform this action with less than 4 frames.\n");
    return;
  }

  //The four lists, and a couple of helper node pointers
  struct Node *A = NULL;
  struct Node *B = NULL;
  struct Node *C = NULL;
  struct Node *D = NULL;
  struct Node *cur = NULL;
  struct Node *temp = NULL;

  char string[10];
  char rw;

  //Keeps track of the number in each of the lists
  int numA;
  int numB;

  int RW[2];
  RW[0] = 0;
  RW[1] = 0;

  int eventCounter = 0;

  int pageHits = 0;

  //highest number of items in one list
  int limit = numFrames / 2;
  int total = numFrames;
  while (fscanf(file, "%s %c", &string, &rw) != EOF)
  {
    //If this address belongs to process 1
    if (string[0] == '3')
    {
      //If List A is empty
      if (A == NULL)
      {
        //Create a new node with the string
        A = (struct Node *)malloc(sizeof(struct Node));
        strcpy(A->string, string);
        A = AddToList(A, A);

        //Update the clean or dirty bit
        UpdateRW(A, RW, rw);

        ++numA;
        --total;
      }
      else
      {
        //Search the elements in A to find the string
        temp = FindPage(A, string);

        //Not in A, search Clean
        if (temp == NULL)
        {
          temp = FindPage(C, string);
        }
        else
        {
          //It was in A
          ++pageHits;
          UpdateRW(A, RW, rw);
        }

        //Search Dirty for element
        if (temp == NULL)
        {
          temp = FindPage(D, string);
        }
        else
        {
          //It was in clean
          //Is A at the limit?
          //No - Add to the end of A
          if (numA < limit)
          {
            cur = RemoveFromList(temp, C);
            A = AddToList(cur, A);
            ++numA;
          }
          else
          {
            //Yes - FIFO A
            cur = RemoveFromList(A, A);
            if (cur->clean)
            {
              C = AddToList(cur, C);
            }
            else
            {
              D = AddToList(cur, D);
            }
            temp = RemoveFromList(temp, C);
            temp = AddToList(temp, A);
          }
        }

        //It was not in Dirty
        if (temp == NULL)
        {
          temp = (struct Node *)malloc(sizeof(struct Node));
          strcpy(temp->string, string);
          UpdateRW(temp, RW, rw);

          //Is A at the limit?
          //No - Add to the end of A
          if (numA < limit)
          {
            A = AddToList(temp, A);
            ++numA;
            --total;
          }
          //Yes - Is table full?
          else
          {
            //No - FIFO A into Clean or Dirty
            if (total > 0)
            {
              cur = RemoveFromList(A, A);
              if (cur->clean)
              {
                C = AddToList(cur, C);
              }
              else
              {
                D = AddToList(cur, D);
              }

              A = AddToList(temp, A);
            }
            //Yes - Check if the clean and dirty are empty
            else
            {
              //If C = null and D = null completely full
              if (C == NULL && D == NULL)
              {
                //FIFO A
                cur = RemoveFromList(A, A);
                free(cur);
                A = AddToList(temp, A);
              }
              else if (C == NULL)
              {
                //Evict first D abd FIFO A
                cur = RemoveFromList(D, D);
                free(cur);

                cur = RemoveFromList(A, A);
                if (cur->clean)
                {
                  C = AddToList(cur, C);
                }
                else
                {
                  D = AddToList(cur, D);
                }

                A = AddToList(temp, A);
              }
              else
              {
                //Evict first C and FIFO A
                cur = RemoveFromList(C, C);
                free(C);

                cur = RemoveFromList(A, A);
                if (cur->clean)
                {
                  C = AddToList(cur, C);
                }
                else
                {
                  D = AddToList(cur, D);
                }
                A = AddToList(temp, A);
              }
            }
          }
        }
        else
        {
          //It was in dirty
          //Is A full?
          if (numA < limit)
          {
            //No- Add to end of A
            A = AddToList(temp, A);
            ++numA;
            --total;
            UpdateRW(temp, RW, rw);
          }
          //Yes - FIFO A and Add temp to end of A
          else
          {
            temp = RemoveFromList(temp, D);
            cur = RemoveFromList(A, A);
            A = AddToList(temp, A);

            if (cur->clean)
            {
              C = AddToList(cur, C);
            }
            else
            {
              D = AddToList(cur, D);
            }
          }
        }
      }
    }
    //This is a B Process
    //Is B empty?
    if (B == NULL)
    {
      //Yes
      B = (struct Node *)malloc(sizeof(struct Node));
      strcpy(B->string, string);
      UpdateRW(B, RW, rw);
      ++numB;
      --total;
    }
    ++eventCounter;
  }

  printf("total memory frames: %d\n", numFrames);
  printf("events in trace: %d\n", eventCounter);
  printf("total disk reads: %d\n", RW[0]);
  printf("total disk writes: %d\n", RW[1]);

  double percentage = (double)pageHits / (double)eventCounter;
  if (debug)
  {
    printf("page hits: %d\n", pageHits);
    printf("hit rate: %f %%", percentage * 100);
  }
}

struct Node *FindLastNode(struct Node *cur)
{
  if (cur == NULL)
  {
    return cur;
  }
  while (cur->n != NULL)
  {
    cur = cur->n;
  }
  return cur;
}

struct Node *FindPage(struct Node *cur, char string[10])
{
  if (cur == NULL)
  {
    return cur;
  }

  while (cur != NULL)
  {
    if (strcmp(cur->string, string) == 0)
    {
      return cur;
    }
    cur = cur->n;
  }

  return cur;
}

struct Node *AddToList(struct Node *add, struct Node *list)
{
  if (list == NULL)
  {
    list = add;
    list->n = NULL;
    list->p = NULL;
  }
  else
  {
    struct Node *cur = FindLastNode(list);
    cur->n = add;
    add->n = NULL;
    add->p = cur;
  }

  return list;
}

struct Node *RemoveFromList(struct Node *remove, struct Node *list)
{
  if (remove->p == NULL && remove->n == NULL)
  {
    list = remove->n;
    return remove;
  }
  else if (remove->p == NULL)
  {
    list = remove->n;
    remove->n = NULL;
    list->p = NULL;
    return remove;
  }
  else if (remove->n == NULL)
  {
    remove->p->n = NULL;
    remove->p = NULL;
    return remove;
  }
  else
  {
    remove->p->n = remove->n;
    remove->n->p = remove->p;
    remove->p = NULL;
    remove->n = NULL;
  }
  return remove;
}

void UpdateRW(struct Node *node, int *RW, char rw)
{
  if (rw == 'R')
  {
    node->clean = true;
    RW[0] += 1;
  }
  else if (rw == 'W')
  {
    node->clean = false;
    RW[1] += 1;
  }
}
/*
//If not found in A and A has room, add new node at end of list
			  if (temp == NULL && numA < limit) {
				  temp = (struct Node*) malloc(sizeof(struct Node));
				  temp->string = string;
				  if (rw == 'R')
					  temp->clean = true;
				  else if (rw == 'W')
					  temp->clean = false;
				  temp->n = NULL;
				  temp->p = cur;
				  cur->n = temp;
				  ++numA;
			  }
			  else if (temp == NULL && numA == limit) {
				  //If not found in A but A has no room
				  if (numB < limit) {
					  //Check clean
					  cur = C;
					  while (cur->next != NULL) {

					  }

					  //Check Dirty

					  A = A->n;
					  if (A->clean) {
						  C = A->p;
						  C->n = NULL;
						  A->p = NULL;
					  }
					  else {
						  D = A->p;
						  D->n = NULL;
						  A->p = NULL;
					  }

					  cur = A;
					  while (cur->n != NULL) {
						  cur = cur->n;
					  }

					  temp = (struct Node*) malloc(sizeof(struct Node));
					  temp->string = string;
					  temp->n = NULL;
					  temp->p = cur;
					  cur->n = temp;
				  }
				  else if (numB == limit) {
					  cur = A;
					  while (cur->next != NULL) {
						  cur = cur->n;
					  }

					  temp = A;
					  A = A->n;
					  A->p = NULL;

					  temp->n = NULL;
					  free(temp);

					  temp = (struct Node*) malloc(sizeof(struct Node));
					  temp->string = string;
					  if (rw == 'R')
						  temp->clean = true;
					  else if (rw == 'W')
						  temp->clean = false;
					  temp->n = false;
					  temp->p = cur;
					  cur->n = temp;

				  }
			  }
*/