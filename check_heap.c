#include "umalloc.h"
#include "csbrk.h"

//Place any variables needed here from umalloc.c or csbrk.c as an extern.
extern memory_block_t *free_head;
extern sbrk_block *sbrk_blocks;

/*
 * check_heap -  used to check that the heap is still in a consistent state.
 
 * STUDENT TODO:
 * Required to be completed for checkpoint 1:
 *      - Check that pointers in the free list point to valid free blocks. Blocks should be within the valid heap addresses: look at csbrk.h for some clues.
 *        They should also be allocated as free.
 *      - Check if any memory_blocks (free and allocated) overlap with each other. Hint: Run through the heap sequentially and check that
 *        for some memory_block n, memory_block n+1 has a sensible block_size and is within the valid heap addresses.
 *      - Ensure that each memory_block is aligned. 
 * 
 * Should return 0 if the heap is still consistent, otherwise return a non-zero
 * return code. Asserts are also a useful tool here.
 */
int check_heap() {
    // Example heap check:
    // Check that all blocks in the free list are marked free.
    // If a block is marked allocated, return -1.
    /*
        memory_block_t *cur = free_head;
        while (cur) {
            if (is_allocated(cur)) {
                return -1;
            }
        }
    */

 

    sbrk_block *field = sbrk_blocks;
    while(field!=NULL){
        memory_block_t* block = (memory_block_t*) field->sbrk_start;
        while(block <= (memory_block_t*) field->sbrk_end){
            if (block->block_size_alloc > (field->sbrk_end-field->sbrk_start)){
                return -1;
            }
            block = (memory_block_t*) ((uintptr_t) block + get_size(block));
        }
        field = field->next;
    }

       memory_block_t *current = free_head;
    while (current != NULL){
        if (check_malloc_output(get_payload(current), get_size(current)) == -1 && !is_allocated(current) ) {
            return -1;
        }
        current = current->next;
    }

    field = sbrk_blocks;
    while(field!=NULL){
        memory_block_t* block = (memory_block_t*) field->sbrk_start;
        while(block <= (memory_block_t*) field->sbrk_end){
            if ((uintptr_t) block % ALIGNMENT !=0){
                return -1;
            }
            block = (memory_block_t*) ((uintptr_t) block + get_size(block));
        }
        field = field->next;
    }

    return 0;
}