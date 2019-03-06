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

void LRU(FILE *file, char PageTable[][10], int numFrames);
void FIFO(FILE *file, char PageTable[][10], int numFrames);
void VMS(FILE *file, int numFrames);

struct Node *FindLastNode(struct Node *cur);
struct Node *FindPage(struct Node *cur, char string[10]);
struct Node *AddToList(struct Node *add, struct Node *list);
struct Node *RemoveFromList(struct Node *remove, struct Node *list);
void UpdateRW(struct Node *node, int *RW, char rw);

bool debug;

int main(int argc, char *argv[])
{

  //Ensures the correct number of arguments are entered
  if (argc != 5)
  {
    printf("ERROR: Need 4 arguments \n");
    return -1;
  }

  bool first = true;  //Just used for formatting
  debug = false; //Debug Mode

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
    LRU(file, PageTable, numFrames);
  }

  if (FIFO_f)
  {
    FIFO(file, PageTable, numFrames);
  }

  if(VMS_f){
    VMS(file, numFrames);
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
void LRU(FILE *file, char PageTable[][10], int numFrames)
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
      if (strcmp(PageTable[i], string) == 0)
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

void FIFO(FILE *file, char PageTable[][10], int numFrames)
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
      if (strcmp(PageTable[i], string) == 0)
      {
        present = true;
        index = i;
        break;
      }
    }

    if (present)
    {
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

void VMS(FILE *file, int numFrames)
{
  if (numFrames % 2 != 0)
  {
    printf("Error: Cannot perform this action with an odd number of frames.\n");
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
    if(debug){
      printf("\nEvent %d: %c on \'%s\'\n", eventCounter, rw, string);
    }
    //If this address belongs to process 1
    if (string[0] == '3')
    {
      if(debug){
        printf("This belongs to Process A\n");
      }
      //If List A is empty
      if (A == NULL)
      {
        if(debug){
          printf("A is empty\n");
        }
        //Create a new node with the string
        A = (struct Node *)malloc(sizeof(struct Node));
        
        if(debug){
          printf("Created a new node\n");
        }

        strcpy(A->string, string);
        if(debug){
          printf("Returned from Adding to the list\n");
        }

        //Update the clean or dirty bit
        UpdateRW(A, RW, rw);

        ++numA;
        --total;
        ++eventCounter;
        continue;
      }
      else
      {
        //Search the elements in A to find the string
        temp = FindPage(A, string);
        
        if(debug)
          printf("Was it in A? >");
        //Not in A, search Clean
        if (temp == NULL)
        {
          if(debug){
            printf("No, checking in Clean now\n");
          }
          temp = FindPage(C, string);
        }
        else
        {
          if(debug)
            printf("HIT\n");
          //It was in A
          ++pageHits;
          UpdateRW(A, RW, rw);
          ++eventCounter;
          continue;
        }

        //Search Dirty for element
        if(debug)
          printf("Is it in clean? >");
        if (temp == NULL)
        {
          if(debug)
            printf("No, checking Dirty now\n");
          temp = FindPage(D, string);
        }
        else
        {
          printf("Yes\n");
          //It was in clean
          //Is A at the limit?
          //No - Add to the end of A
          if (numA < limit)
          {
            if(debug){
              printf("%d / %d in A\n", numA, limit);
            }

            if(debug)
              printf("Removing it from Clean\n");
            cur = RemoveFromList(temp, C);

            if(debug)
              printf("Adding it to A\n");
            A = AddToList(cur, A);
            ++numA;
            UpdateRW(cur, RW, rw);
          }
          else
          {
            //Yes - FIFO A
            if(debug)
              printf("A is full\nRemoving The first from A");
            cur = RemoveFromList(A, A);

            if(debug)
              printf("Adding the removed page to ");
            if (cur->clean)
            {
              if(debug)
                printf("CLEAN\n");
              C = AddToList(cur, C);
            }
            else
            {
              if(debug)
                printf("DIRTY\n");
              D = AddToList(cur, D);
            }

            temp = RemoveFromList(temp, C);
            temp = AddToList(temp, A);
            UpdateRW(temp, RW, rw);
          }

          ++eventCounter;
          continue;
        }

        if(debug)
          printf("Was it in Dirty? >");
        //It was not in Dirty
        if (temp == NULL)
        {
          if(debug){
            printf("No\n");
          }
          temp = (struct Node *)malloc(sizeof(struct Node));

          if(debug)
            printf("Created a new node\n");

          strcpy(temp->string, string);
          UpdateRW(temp, RW, rw);

          //Is A at the limit?
          //No - Add to the end of A
          if(debug)
            printf("%d / %d in A\n", numA, limit);
          if (numA < limit)
          {            
            A = AddToList(temp, A);
            ++numA;
            --total;
          }
          //Yes - Is table full?
          else
          {
            if(debug)
              printf("Is the table full? >");
            //No - FIFO A into Clean or Dirty
            if (total > 0)
            {
              if(debug)
                printf("No, performing FIFO on A");
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
              if(debug)
                printf("Yes\n");
              //If C = null and D = null completely full
              if (C == NULL && D == NULL)
              {
                if(debug)
                  printf("CLEAN and DIRTY are empty, regular FIFO\n");

                //FIFO A
                cur = RemoveFromList(A, A);
                free(cur);
                A = AddToList(temp, A);
              }
              else if (C == NULL)
              {
                if(debug)
                  printf("Evicting DIRTY and FIFO on A\n");
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
                if(debug)
                  printf("Evicting from C and FIFO on A\n");
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
          if(debug)
            printf("Yes\n%d / %d in A\n", numA, limit);
          if (numA < limit)
          {
            //No- Add to end of A
            A = AddToList(temp, A);
            ++numA;
            --total;
            UpdateRW(temp, RW, rw);
            if(debug)
              printf("Added to A\n");
          }
          //Yes - FIFO A and Add temp to end of A
          else
          {
            if(debug)
              printf("Removing from Dirty and FIFO A\n");

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

            UpdateRW(temp, RW, rw);
          }
        }
      }
      ++eventCounter;
      continue;
    }
    //This is a B Process
    //Is B empty?
    else{
       if(debug)
          printf("This process belongs to B\n");

      if (B == NULL)
      {
        //Yes
        if(debug)
          printf("B is empty\n");
        B = (struct Node *)malloc(sizeof(struct Node));
        strcpy(B->string, string);
        UpdateRW(B, RW, rw);
        ++numB;
        --total;
        ++eventCounter;
        continue;
      }
      else{
        if(debug)
          printf("Searching in B\n");
        temp = FindPage(B, string);

        //Is the page in the list?
        if(temp == NULL){
          //No- Check for it in Clean
          if(debug)
            printf("Searching in Clean:\n");
          temp = FindPage(C, string);
        }
        else{
          //Yes - It is in the list
          if(debug)
            printf("HIT\n");
          ++pageHits;
          UpdateRW(temp, RW, rw);
          ++eventCounter;
          continue;
        }

        //Is it in Clean?
        if(temp == NULL){
          //No - Check Dirty
          if(debug)
            printf("Checking Dirty\n");
          temp = FindPage(D, string);
        }
        else{
          //It is in Clean
          //Is B full?
          if(debug)
            printf("Found in Clean\n");
          if(numB < limit){
            //No
            if(debug)
              printf("B is not full\n");
            temp = RemoveFromList(temp, C);
            B = AddToList(temp, B);
            UpdateRW(temp, RW, rw);
            ++eventCounter;
            continue;
          }
          else{
            //Yes
            if(debug)
              printf("B is full\n");
            temp = RemoveFromList(temp, C);
            cur = RemoveFromList(B, B);
            B = AddToList(temp, B);

            if(cur->clean){
              C = AddToList(cur, C);
            }
            else{
              D = AddToList(cur, D);
            }
            UpdateRW(temp, RW, rw);
            ++eventCounter;
            continue;
          }
        }
          //Is it in Dirty?
        if(temp == NULL){
          //No
          if(debug)
            printf("Not found in any lists\n");
          temp = (struct Node*) malloc (sizeof(struct Node));
          strcpy(temp->string, string);
          UpdateRW(temp, RW, rw);
          --total;

          //Is B full?
          if(numB < limit){
            //No - add at the end of the list
            if(debug)
              printf("B is not full, adding to the end of the list\n");
            B = AddToList(temp, B);
            ++eventCounter;
            ++numB;
            continue;
          }
          else{
            //B is full- Is there room in the table?
            if(debug)
              printf("B is full\n");
            if(total > 0){
              //Yes
              if(debug)
                printf("Table has space\n");
              cur = RemoveFromList(B, B);
              B = AddToList(temp,B);
              printf("Placing B into ");
              if(cur->clean){
                printf("CLEAN\n");
                C = AddToList(cur, C);
              }
              else{
                D = AddToList(cur, D);
                printf("CLEAN\n");
              }
            }
            else{
              //There is no room on the table
              //If C and D are empty, FIFO B
              if(C == NULL && D == NULL){
                if(debug)
                  printf("FIFO on B\n");
                cur = RemoveFromList(B, B);
                free (cur);
                B = AddToList(temp, B);
                ++eventCounter;
                continue;
              }
              else if( C==NULL){
                //Take from Dirty and FIFO B
                if(debug)
                  printf("Evicting from Dirty and FIFO on B\n");
                cur = RemoveFromList(D, D);
                free(cur);

                cur = RemoveFromList(B,B);
                B = AddToList(temp, B);

                if(cur->clean){
                  C = AddToList(cur, C);
                }
                else{
                  D = AddToList(cur, D);
                }

                ++eventCounter;
                continue;
              }
              else{
                //Take from Clean
                if(debug)
                  printf("Evicting from C and FIFO on B\n");
                cur = RemoveFromList(C, C);
                free(cur);

                cur = RemoveFromList(B, B);
                B = AddToList(temp, B);
                if(cur->clean){
                  C = AddToList(cur, C);
                }
                else{
                  D = AddToList(cur, D);
                }
                ++eventCounter;
                continue;
              }
            }
          }
        }
        else {
          if (debug)
            printf("Yes\n");
          //It was in Dirty
          //Is B full?
          if(numB < limit){
            //No
            if(debug)
            temp = RemoveFromList(temp, D);
            B = AddToList(temp, B);
            ++numB;
            ++eventCounter;
            continue;
          }
          else{
            //B is full
            temp = RemoveFromList(temp, D);
            cur = RemoveFromList(B, B);

            B = AddToList(temp, B);

            if(cur->clean){
              C = AddToList(cur, C);
            }
            else{
              D = AddToList(cur, D);
            }
            ++eventCounter;
            continue;
          
          }
        }
      }
    }
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
  if(debug){
    printf("\nFIND_LAST_NODE::Entered\n");
  }
  if (cur == NULL)
  {
    if(debug){
      printf("\tList is empty\n");
    }
    return cur;
  }

  if(debug)
    printf("\tList is not empty\n\t");
  
  while (cur->n != NULL)
  {
    if(debug)
      printf("SEARCHING--");
    cur = cur->n;
  }
  if(debug)
    printf("FOUND!\n");
  return cur;
}

struct Node *FindPage(struct Node *list, char string[10])
{
  if(debug)
    printf("\nFIND_PAGE::Entered\n");

  struct Node* cur = list;

  if (cur == NULL)
  {
    printf("\tThis list is currently empty\n");
    return cur;
  }

  if(debug)
    printf("\t");
  while (cur != NULL)
  {
    if(debug)
      printf("Searching--");
    if (strcmp(cur->string, string) == 0)
    {
      if(debug)
        printf("FOUND!\n");
      return cur;
    }
    cur = cur->n;
  }

  if(debug)
    printf("Couldn't find the page\n");
  return cur;
}

struct Node *AddToList(struct Node *add, struct Node *list)
{
  if(debug){
    printf("\nADD_TO_LIST::\'%s\' to list\n", add->string);
  }
  if (list == NULL)
  {
    if(debug){
      printf("\tList is empty\n");
    }
    list = add;
    list->n = NULL;
    list->p = NULL;
  }
  else
  {
    struct Node *cur = FindLastNode(list);
    
    if(debug)
      printf("\nADD_TO_LIST:: Returned from FindLastNode()\n");

    cur->n = add;
    add->n = NULL;
    add->p = cur;
  }
  return list;
}

struct Node *RemoveFromList(struct Node *remove, struct Node *list)
{
  if(debug)
    printf("\nREMOVE_FROM_LIST::Entered\n");
  if (remove->p == NULL && remove->n == NULL)
  {
    if(debug)
      printf("\tThis list is now empty\n");
    list = remove->n;
    return remove;
  }
  else if (remove->p == NULL)
  {
    if(debug)
      printf("\tRemoving the first element of this list\n");
    list = remove->n;
    remove->n = NULL;
    list->p = NULL;
    return remove;
  }
  else if (remove->n == NULL)
  {
    if(debug)
      printf("\tRemoving the last element of this list\n");
    remove->p->n = NULL;
    remove->p = NULL;
    return remove;
  }
  else
  {
    if(debug)
      printf("\tRemoving an element from the list\n");
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