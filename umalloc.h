#include <stdlib.h>
#include <stdbool.h>

#define ALIGNMENT 16 /* The alignment of all payloads returned by umalloc */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~(ALIGNMENT-1))

/*
 * memory_block_t - Represents a block of memory managed by the heap. The 
 * struct can be left as is, or modified for your design.
 * In the current design bit0 is the allocated bit
 * bits 1-3 are unused.
 * and the remaining 60 bit represent the size.
 */
typedef struct memory_block_struct {
    size_t block_size_alloc;
    struct memory_block_struct *next;
} memory_block_t;

// Helper Functions, this may be editted if you change the signature in umalloc.c

/*
*  STUDENT TODO:
*      Write 1-2 sentences for each function explaining what it does. Don't just repeat the name of the function back to us.
*/

//checks the final bit to see if block is allocated
bool is_allocated(memory_block_t *block);

//indicates that block is allocated by assigning 1 to final bit
void allocate(memory_block_t *block);

//indicates block is deallocated by assigining 0 to last bit
void deallocate(memory_block_t *block);
//checks first 3 bits and returns size of block
size_t get_size(memory_block_t *block);
//returns memory block pointer of next block
memory_block_t *get_next(memory_block_t *block);
//puts a new block of memory at address - indicates free or allocated with last bit
void put_block(memory_block_t *block, size_t size, bool alloc);
//returns a pointer to pay load using 16bit increments
void *get_payload(memory_block_t *block);
//retunrs pointer to block by subtracting 16bits from pay load
memory_block_t *get_block(void *payload);
//iterate thru free blocks in linkedlist to find available memory address
memory_block_t *find(size_t size);
//if no free block exists of appropriate size use sbrk to add more memory
memory_block_t *extend(size_t size);
//divides a free block into allocated and free - returns pointer to allocated
memory_block_t *split(memory_block_t *block, size_t size);
//merges free blocks to optimize space 
memory_block_t *coalesce(memory_block_t *block);


// Portion that may not be edited
int uinit();
void *umalloc(size_t size);
void ufree(void *ptr);