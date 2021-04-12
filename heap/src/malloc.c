/*
*Hoang Ho - 1001654608
*Joanna Huynh - 1001702615
*CSE 3320-003 Heap Assignment
*Due: April 12, 2021 5:30 PM
*/


#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

#define ALIGN4(s)         (((((s) - 1) >> 2) << 2) + 4)
#define BLOCK_DATA(b)      ((b) + 1)
#define BLOCK_HEADER(ptr)   ((struct _block *)(ptr) - 1)


static int atexit_registered = 0;
static int num_mallocs       = 0;
static int num_frees         = 0;
static int num_reuses        = 0;
static int num_grows         = 0;
static int num_blocks        = 0;
static int num_requested     = 0;
static int max_heap          = 0;

/*
 *  \brief printStatistics
 *
 *  \param none
 *
 *  Prints the heap statistics upon process exit.  Registered
 *  via atexit()
 *
 *  \return none
 */
void printStatistics( void )
{
  printf("\nheap management statistics\n");
  printf("mallocs:\t%d\n", num_mallocs );
  printf("frees:\t\t%d\n", num_frees );
  printf("reuses:\t\t%d\n", num_reuses );
  printf("grows:\t\t%d\n", num_grows );
  printf("blocks:\t\t%d\n", num_blocks );
  printf("requested:\t%d\n", num_requested );
  printf("max heap:\t%d\n", max_heap );
}

struct _block 
{
   size_t  size;         /* Size of the allocated _block of memory in bytes */
   struct _block *prev;  /* Pointer to the previous _block of allcated memory   */
   struct _block *next;  /* Pointer to the next _block of allcated memory   */
   bool   free;          /* Is this _block free?                     */
   char   padding[3];
};


struct _block *heapList = NULL; /* Free list to track the _blocks available */
struct _block *last_alloc = NULL; //keeping track of the last block reuse for the next fit algorithm

/*
 * \brief findFreeBlock
 *
 * \param last pointer to the linked list of free _blocks
 * \param size size of the _block needed in bytes 
 *
 * \return a _block that fits the request or NULL if no free _block matches
 *
 * \TODO Implement Next Fit
 * \TODO Implement Best Fit
 * \TODO Implement Worst Fit
 */
struct _block *findFreeBlock(struct _block **last, size_t size) 
{
   struct _block *curr = heapList;

#if defined FIT && FIT == 0
   /* First fit */
   //chooses the first block that can fit the size requested
   
   while (curr && !(curr->free && curr->size >= size)) 
   {
      *last = curr;
      curr  = curr->next;
   }
#endif

#if defined BEST && BEST == 0
   /*best fit goes through the entire heap list and chooses
    *the block with the block with the least remaining space after
    *using the requested size*/
   struct _block *chosen = NULL;
   size_t difference = INT_MAX;
   
   while(curr)
   {
      //makes sure block is free and large enough to allocate
      if(curr->free && size <= (curr->size))
      {
         /*sees if difference is smaller than the previous difference
          *to find the best fit*/
         if( ((curr->size) - size) < difference )
         {
            difference = (curr->size) - size;
            chosen = curr;
         }
      }
      *last = curr;
      curr = curr->next;
   }
   curr = chosen;
#endif

#if defined WORST && WORST == 0
   /*worst fit goes through the entire heap list and chooses
    *the block with the block with the most remaining space after
    *using the requested size*/
    
   struct _block *chosen = NULL;
   //using 0 here instsead of INT_MIN since size_t is unsigned
   size_t difference = 0; 
   
   while(curr)
   {
      //makes sure block is free and large enough to allocate
      if(curr->free && size <= (curr->size))
      {
         //sees if difference is bigger than the previous difference
         //to find the worst fit
         if( ((curr->size) - size) > difference)
         {
            difference = (curr->size) - size;
            chosen = curr;
         }
      }
      *last = curr;
      curr = curr->next;
   }
   curr = chosen;
#endif

#if defined NEXT && NEXT == 0
   //next fit optimizes first fit to keep track of previous block allocated/reused
   
   //makes sure the heap list is not empty before setting the variable
   if(last_alloc != NULL)
   {
      curr = last_alloc;
   }
   
   /*try to go all the way to the end of the heap list
    *this is to take into account last_alloc being
    *somewhere in the list*/
   while (curr && !(curr->free && curr->size >= size)) 
   {
      *last = curr;
      curr = curr->next;
   }
   
   /*if we go through the list until current reaches NULL
    *and the last reuse we did is not at the end of the list
    *go through the heap list from the top again making sure
    *we check all the blocks*/
   if(curr == NULL && last_alloc != NULL)
   {
      curr = heapList;
      while (curr && !(curr->free && curr->size >= size)) 
      {
         *last = curr;
         curr = curr->next;
      }
   }
   last_alloc = curr;
   
#endif

   return curr;
}

/*
 * \brief growheap
 *
 * Given a requested size of memory, use sbrk() to dynamically 
 * increase the data segment of the calling process.  Updates
 * the free list with the newly allocated memory.
 *
 * \param last tail of the free _block list
 * \param size size in bytes to request from the OS
 *
 * \return returns the newly allocated _block of NULL if failed
 */
struct _block *growHeap(struct _block *last, size_t size) 
{
   /* Request more space from OS */
   struct _block *curr = (struct _block *)sbrk(0);
   struct _block *prev = (struct _block *)sbrk(sizeof(struct _block) + size);

   assert(curr == prev);

   /* OS allocation failed */
   if (curr == (struct _block *)-1) 
   {
      return NULL;
   }

   /* Update heapList if not set / there is no head */
   if (heapList == NULL) 
   {
      heapList = curr;
   }

   /* Attach new _block to prev _block */
   if (last) 
   {
      last->next = curr;
   }

   /* Update _block metadata */
   curr->size = size;
   curr->next = NULL;
   curr->free = false;
   
   //tracking number of growHeaps and current maximum heap size and number of blocks in the heap
   max_heap += size;
   num_blocks++;
   num_grows++;
   
   return curr;
}

/*
 * \brief malloc
 *
 * finds a free _block of heap memory for the calling process.
 * if there is no free _block that satisfies the request then grows the 
 * heap and returns a new _block
 *
 * \param size size of the requested memory in bytes
 *
 * \return returns the requested memory allocation to the calling process 
 * or NULL if failed
 */
void *malloc(size_t size) 
{
   num_requested = num_requested + size; //tracking total requested blocks

   if( atexit_registered == 0 )
   {
      atexit_registered = 1;
      atexit( printStatistics );
   }

   /* Align to multiple of 4 */
   size = ALIGN4(size);

   /* Handle 0 size */
   if (size == 0) 
   {
      return NULL;
   }

   /* Look for free _block */
   struct _block *last = heapList;
   struct _block *next = findFreeBlock(&last, size);

   /* Could not find free _block, so grow heap */
   if (next == NULL) 
   {
      next = growHeap(last, size);
   }
   else
   {
      num_reuses++; //tracking number of times an existing block is used without growing heap
   }

   /* Could not find free _block or grow heap, so just return NULL */
   if (next == NULL)
   {
      return NULL;
   }
   
   /* Mark _block as in use */
   next->free = false;
   num_mallocs++;

   /* Return data address associated with _block */
   return BLOCK_DATA(next);
}

/*
 * \brief realloc
 *
 * reallocate provided memory to a specified size
 *
 * \param ptr address to the memory whose size is to be changed
 * \param size size of the requested memory in bytes
 *
 * \return returns the requested memory allocation to the calling process 
 * or NULL if failed
 */
void *realloc(void *ptr, size_t size)
{
	struct _block *new_block = ( struct _block * ) malloc ( size );
	memcpy ( new_block, ptr, size );
	free( ptr );
	return new_block;
}

/*
 * \brief calloc
 *
 * finds a free _block of heap memory for the calling process.
 * if there is no free _block that satisfies the request then grows the 
 * heap and returns a new _block
 *
 * \param num the number of blocks each of size to request memory
 * \param size size of the requested memory in bytes
 *
 * \return returns the requested memory allocation to the calling process 
 * or NULL if failed
 */
void *calloc(size_t num, size_t size)
{
	size_t total_size = num * size;
	struct _block *new_block = ( struct _block * ) malloc ( total_size );
	memset( new_block, 0, total_size);
	return BLOCK_DATA(new_block);
}

/*
 * \brief free
 *
 * frees the memory _block pointed to by pointer. if the _block is adjacent
 * to another _block then coalesces (combines) them
 *
 * \param ptr the heap memory to free
 *
 * \return none
 */
void free(void *ptr) 
{
   if (ptr == NULL) 
   {
      return;
   }

   /* Make _block as free */
   struct _block *curr = BLOCK_HEADER(ptr);
   assert(curr->free == 0);
   curr->free = true;
   num_frees++;
}

/* vim: set expandtab sts=3 sw=3 ts=6 ft=cpp: --------------------------------*/
