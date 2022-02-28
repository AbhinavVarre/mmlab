#include "umalloc.h"
#include "csbrk.h"
#include "ansicolors.h"
#include <stdio.h>
#include <assert.h>

const char author[] = ANSI_BOLD ANSI_COLOR_RED "Abhinav Varre av36753" ANSI_RESET;

/*
 * The following helpers can be used to interact with the memory_block_t
 * struct, they can be adjusted as necessary.
 */

// A sample pointer to the start of the free list.
memory_block_t *free_head;

/*
 * is_allocated - returns true if a block is marked as allocated.
 */
bool is_allocated(memory_block_t *block)
{
    assert(block != NULL);
    return block->block_size_alloc & 0x1;
}

/*
 * allocate - marks a block as allocated.
 */
void allocate(memory_block_t *block)
{
    assert(block != NULL);
    block->block_size_alloc |= 0x1;
}


/*
 * deallocate - marks a block as unallocated.
 */
void deallocate(memory_block_t *block)
{
    assert(block != NULL);
    block->block_size_alloc &= ~0x1;
}

/*
 * get_size - gets the size of the block.
 */
size_t get_size(memory_block_t *block)
{
    assert(block != NULL);
    return block->block_size_alloc & ~(ALIGNMENT - 1);
}
size_t round_up(size_t size){
    return ((size + (ALIGNMENT-1)) & (-1*ALIGNMENT));
}


/*
 * get_next - gets the next block.
 */
memory_block_t *get_next(memory_block_t *block)
{
    assert(block != NULL);
    return block->next;
}

/*
 * put_block - puts a block struct into memory at the specified address.
 * Initializes the size and allocated fields, along with NUlling out the next 
 * field.
 */
void put_block(memory_block_t *block, size_t size, bool alloc)
{
    assert(block != NULL);
    assert(size % ALIGNMENT == 0);
    assert(alloc >> 1 == 0);
    block->block_size_alloc = size | alloc;
    block->next = NULL;
}

/*
 * get_payload - gets the payload of the block.
 */
void *get_payload(memory_block_t *block)
{
    assert(block != NULL);
    return (void *)(block + 1);
}

/*
 * get_block - given a payload, returns the block.
 */
memory_block_t *get_block(void *payload)
{
    assert(payload != NULL);
    return ((memory_block_t *)payload) - 1;
}

/*
 *  STUDENT TODO:
 *      Describe how you select which free block to allocate. What placement strategy are you using?
 * look through memory until the next block is null - this means that this block is empty. return this block
 * 
 * find - finds a free block that can satisfy the umalloc request.
 */
memory_block_t *find(size_t size)
{
    //? STUDENT TODO
    memory_block_t *tempHeadRef = free_head;
    while (get_size(tempHeadRef) < size)
    {
        tempHeadRef = get_next(tempHeadRef);
        if (tempHeadRef == NULL)
        {
            return NULL;
        }
    }
    return tempHeadRef;
}

/*
 * extend - extends the heap if more memory is required.
 */
memory_block_t *extend(size_t size)
{
    memory_block_t *tempHeadRef = free_head;
    while (get_size(tempHeadRef) < size)
    {
        if (tempHeadRef->next == NULL)
        {
            memory_block_t *extra_block = (memory_block_t *)csbrk(PAGESIZE * 5);
            put_block(extra_block, PAGESIZE * 5 - ALIGNMENT, false);
            tempHeadRef->next = extra_block;
            return tempHeadRef->next;
        }
        tempHeadRef = get_next(tempHeadRef);
    }
    return NULL;
    return NULL;
}

/*
 *  STUDENT TODO:
 *      Describe how you chose to split allocated blocks. Always? Sometimes? Never? Which end?
 *     The split is based on the least significant end of the blocks in order of memory addresses. This 
 *      way the starting address of the block remains unchanged. This means that the memory addresses can stay the same.
*/

/*
 * split - splits a given block in parts, one allocated, one free.
 */
memory_block_t *split(memory_block_t *block, size_t size)
{
    if (block->block_size_alloc < size)
    {
        return NULL;
    }
    else
    {
        put_block(block, block->block_size_alloc - size, false);
        memory_block_t *new_alloced_block = (memory_block_t *)((uintptr_t)block + get_size(block));
        put_block(new_alloced_block, size, true);
        return new_alloced_block;
    }
}

memory_block_t *splice(memory_block_t *block) {
    memory_block_t* temp_head_reference = free_head;
    if (temp_head_reference->next == NULL){
        return temp_head_reference;
    }
    while (temp_head_reference->next != block){
        if (temp_head_reference->next == NULL){
            return temp_head_reference;
        }
        temp_head_reference = get_next(temp_head_reference);
    }
    temp_head_reference->next = block->next;
    return temp_head_reference;
}

/*
 * coalesce - coalesces a free memory block with neighbors.
 */
memory_block_t *coalesce(memory_block_t *block)
{
    memory_block_t *next = (memory_block_t *)((uintptr_t)block + get_size(block));
    if (is_allocated(next))
    {
        splice(next);
        put_block(block, get_size(block) + get_size(block), false);
        free_head = block;
        return block;
    }
    return NULL;
}



/*
 * uinit - Used initialize metadata required to manage the heap
 * along with allocating initial memory.
 */
int uinit()
{
    memory_block_t* new_alloced_block = (memory_block_t*) csbrk(PAGESIZE*5);
    put_block(new_alloced_block, PAGESIZE*5-ALIGNMENT, false);
    free_head = new_alloced_block;
    return 0;
}

/*
 * umalloc -  allocates size bytes and returns a pointer to the allocated memory.
 */
void *umalloc(size_t size)
{
    size = round_up(size);
    extend(size);
    memory_block_t* addr = find(size);
    if (addr != NULL){
        return split(addr, size);
    }
    return NULL;
}

/*
 *  STUDENT TODO:
 *      Describe your free block insertion policy.
 *      rearrange the existing memory blocks (coalesce and splice)
 *      and then deallocate the memory. 
 * 
 *       
 * 
*/

/*
 * ufree -  frees the memory space pointed to by ptr, which must have been called
 * by a previous call to malloc.
 */
void ufree(void *ptr)
{
    memory_block_t* mem_to_free = ptr;
    splice(mem_to_free);
    if (coalesce(ptr) ==NULL){
        deallocate(mem_to_free);
        mem_to_free->next = free_head;
        free_head = mem_to_free;
    }   
}