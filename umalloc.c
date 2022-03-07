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
bool is_allocated(memory_block_t *block) {
    assert(block != NULL);
    return block->block_size_alloc & 0x1;
}

/*
 * allocate - marks a block as allocated.
 */
void allocate(memory_block_t *block) {
    assert(block != NULL);
    block->block_size_alloc |= 0x1;
}


/*
 * deallocate - marks a block as unallocated.
 */
void deallocate(memory_block_t *block) {
    assert(block != NULL);
    block->block_size_alloc &= ~0x1;
}

/*
 * get_size - gets the size of the block.
 */
size_t get_size(memory_block_t *block) {
    assert(block != NULL);
    return block->block_size_alloc & ~(ALIGNMENT-1);
}

/*
 * get_next - gets the next block.
 */
memory_block_t *get_next(memory_block_t *block) {
    assert(block != NULL);
    return block->next;
}

/*
 * put_block - puts a block struct into memory at the specified address.
 * Initializes the size and allocated fields, along with NUlling out the next 
 * field.
 */
void put_block(memory_block_t *block, size_t size, bool alloc) {
    assert(block != NULL);
    assert(size % ALIGNMENT == 0);
    assert(alloc >> 1 == 0);
    block->block_size_alloc = size | alloc;
    block->next = NULL;
}

/*
 * get_payload - gets the payload of the block.
 */
void *get_payload(memory_block_t *block) {
    assert(block != NULL);
    return (void*)(block + 1);
}

/*
 * get_block - given a payload, returns the block.
 */
memory_block_t *get_block(void *payload) {
    assert(payload != NULL);
    return ((memory_block_t *)payload) - 1;
}


//puts a free block in memory address order into the list of free blocks
memory_block_t *insertBlock(memory_block_t *free_address) {
    if (free_head==NULL){
        free_head = free_address;
        return free_address;
    }
    memory_block_t* head_reference = free_head;
    if ((uint64_t)free_address< (uint64_t)head_reference){
        free_address->next = free_head;
        free_head = free_address;
        return free_address;
    }
    while(head_reference!=NULL){
        if (head_reference->next == NULL){
            head_reference->next=free_address;
            return free_address;
        }
        if ((uint64_t)free_address < (uint64_t)head_reference->next){
            free_address->next = head_reference->next;
            head_reference->next=free_address;
            return free_address;
        }
        head_reference = get_next(head_reference);
    }
    return free_address;
}

/*
 *  STUDENT TODO:
 *      Describe how you select which free block to allocate. What placement strategy are you using?
 *      look through memory until the next block is null - this means that this block is empty. return this block
 */

/*
 * find - finds a free block that can satisfy the umalloc request.
 */
memory_block_t *find(size_t size) {
    memory_block_t* head_reference = free_head;
    if (free_head==NULL){
        return extend(size);
    }
    while (get_size(head_reference) < size){
        head_reference = get_next(head_reference);
        if (head_reference == NULL){
            return extend(size);
        }
    }
    return head_reference;
}

/*
 * extend - extends the heap if more memory is required.
 */
memory_block_t *extend(size_t size) {
    memory_block_t* alloced_block = (memory_block_t*) csbrk(size);
    put_block(alloced_block,size, true);
    return alloced_block;
}

/*
 *  STUDENT TODO:
 *     Describe how you chose to split allocated blocks. Always? Sometimes? Never? Which end?
 *     The split is based on the least significant end of the blocks in order of memory addresses. This 
 *     way the starting address of the block remains unchanged. This means that the memory addresses can stay the same.
*/

/*
 * split - splits a given block in parts, one allocated, one free.
 */
memory_block_t *split(memory_block_t *block, size_t size) {
    if (get_size(block)<size){
        return NULL;
    }
    memory_block_t* nextBlock = block->next; 
    put_block(block, get_size(block)-size, false);
    block->next = nextBlock;
    //go to the least significant end and add block
    memory_block_t* allocated_block = (memory_block_t*) ((uint64_t) block + get_size(block));
    put_block(allocated_block, size, true);
    return allocated_block;
}

//gets the last block in the free list
memory_block_t *get_prev(memory_block_t *block){
    memory_block_t* head_reference = free_head;
    if (head_reference->next == NULL){
        return NULL;
    }
    while (head_reference->next != block){
        if (head_reference->next == NULL){
            return NULL;
        }
        head_reference = get_next(head_reference);
    }
    return head_reference;
}

//takes the given block out of the free list
memory_block_t *splice( memory_block_t *block) {
    if (free_head==NULL){
        return block;
    }
    if (block == free_head){
        free_head = NULL;
        block->next = NULL;
        return block;
    }
    memory_block_t *prev = get_prev(block);
    if (prev!=NULL){
        prev->next = block->next;
    }
    block->next = NULL;
    return block;
}


/*
 * coalesce - coalesces a free memory with free blocks.
 */
memory_block_t *coalesce(memory_block_t *block) {
    memory_block_t* neighbor = (memory_block_t*) ((uintptr_t) block + get_size(block));
    if (neighbor==block->next && !is_allocated(neighbor)){
        memory_block_t* neighborNext = neighbor->next;
        put_block(block,get_size(block) + get_size (neighbor), false);
        block->next = neighborNext;
    }
    else{
        insertBlock(block);
    }
    return block;
}

/*
 * uinit - Used initialize metadata required to manage the heap
 * along with allocating initial memory.
 */
int uinit() {
    free_head = (memory_block_t*) csbrk(PAGESIZE);
    if (free_head == NULL){
        return -1;
    }
    put_block(free_head, PAGESIZE, false);
    return 0;
}

/*
 * umalloc -  allocates size bytes and returns a pointer to the allocated memory.
 */
void *umalloc(size_t size) {
    size = ALIGN(size+ALIGNMENT);
    memory_block_t* free_address = find(size);
    if (free_address != NULL){
        if ((get_size(free_address)-size) > 16)
            return get_payload(split(free_address, size));
        else{ //cannot split
            allocate(free_address);
            splice(free_address);
            return (get_payload(free_address));
        }
    }
    return NULL;
}

/*
 *  STUDENT TODO:
 *      Describe your free block insertion policy.
 *      rearrange the existing memory blocks (coalesce and splice)
 *      and then deallocate the memory. 
*/

/*
 * ufree -  frees the memory space pointed to by ptr, which must have been called
 * by a previous call to malloc.
 */
void ufree(void *ptr) {
    memory_block_t* to_free = get_block((memory_block_t*) ptr);
    deallocate(to_free);
    coalesce(to_free);
}