// alphix kernel
// memory management system with kmalloc and kmfree
// mem/heap.c

#include <mem/heap.h>
#include <string.h>
#include <kprintf.h>

// global heap instance
static heap_t kernel_heap;

// initialize the memory management system
void heap_init(void) {
    // clear the heap structure
    memset(&kernel_heap, 0, sizeof(heap_t));
    
    // set up initial heap boundaries
    kernel_heap.start_address = HEAP_START_ADDRESS;
    kernel_heap.end_address = HEAP_START_ADDRESS + HEAP_INITIAL_SIZE;
    kernel_heap.max_address = HEAP_START_ADDRESS + HEAP_MAX_SIZE;
    
    // create the first block that covers the entire initial heap
    heap_block_t *first_block = (heap_block_t *)kernel_heap.start_address;
    first_block->magic = HEAP_MAGIC;
    first_block->size = HEAP_INITIAL_SIZE;
    first_block->used = 0;  // free initially
    first_block->next = NULL;
    first_block->prev = NULL;
    
    kernel_heap.first_block = first_block;
    kernel_heap.total_blocks = 1;
    kernel_heap.used_blocks = 0;
    
    kprintf("heap initialized: start=0x%llx, end=0x%llx\n", 
            kernel_heap.start_address, kernel_heap.end_address);
}

// find a free block that can accommodate the requested size
heap_block_t *heap_find_free_block(size_t size) {
    heap_block_t *current = kernel_heap.first_block;
    
    // add header size to the requested size
    size_t total_size = size + sizeof(heap_block_t);
    total_size = HEAP_ALIGN(total_size);  // align to 8 bytes
    
    while (current != NULL) {
        // check if this block is free and big enough
        if (current->used == 0 && current->size >= total_size) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;  // no suitable block found
}

// split a block if it's much larger than needed
heap_block_t *heap_split_block(heap_block_t *block, size_t size) {
    size_t total_size = size + sizeof(heap_block_t);
    total_size = HEAP_ALIGN(total_size);
    
    // only split if the remaining space is significant (at least 64 bytes)
    if (block->size < total_size + 64) {
        return block;  // don't split if remaining space is too small
    }
    
    // create a new block for the remaining space
    heap_block_t *new_block = (heap_block_t *)((char *)block + total_size);
    new_block->magic = HEAP_MAGIC;
    new_block->size = block->size - total_size;
    new_block->used = 0;  // free
    new_block->next = block->next;
    new_block->prev = block;
    
    // update the original block
    block->size = total_size;
    block->used = 1;  // mark as used
    
    // update the next block's prev pointer
    if (block->next != NULL) {
        block->next->prev = new_block;
    }
    block->next = new_block;
    
    kernel_heap.total_blocks++;
    
    return block;
}

// merge adjacent free blocks to reduce fragmentation
void heap_merge_adjacent_blocks(heap_block_t *block) {
    // try to merge with next block
    if (block->next != NULL && block->next->used == 0) {
        block->size += block->next->size;
        block->next = block->next->next;
        if (block->next != NULL) {
            block->next->prev = block;
        }
        kernel_heap.total_blocks--;
    }
    
    // try to merge with previous block
    if (block->prev != NULL && block->prev->used == 0) {
        block->prev->size += block->size;
        block->prev->next = block->next;
        if (block->next != NULL) {
            block->next->prev = block->prev;
        }
        kernel_heap.total_blocks--;
    }
}

// expand the heap when we run out of memory
void heap_expand_heap(size_t size) {
    size_t expand_size = HEAP_ALIGN(size);
    if (expand_size < HEAP_BLOCK_SIZE) {
        expand_size = HEAP_BLOCK_SIZE;
    }
    
    // check if we can expand
    if (kernel_heap.end_address + expand_size > kernel_heap.max_address) {
        kprintf("heap expansion failed: would exceed max size\n");
        return;
    }
    
    // create a new free block at the end
    heap_block_t *new_block = (heap_block_t *)kernel_heap.end_address;
    new_block->magic = HEAP_MAGIC;
    new_block->size = expand_size;
    new_block->used = 0;  // free
    new_block->next = NULL;
    
    // find the last block and link it
    heap_block_t *last_block = kernel_heap.first_block;
    while (last_block->next != NULL) {
        last_block = last_block->next;
    }
    
    last_block->next = new_block;
    new_block->prev = last_block;
    
    kernel_heap.end_address += expand_size;
    kernel_heap.total_blocks++;
    
    kprintf("heap expanded by %zu bytes, new end: 0x%llx\n", 
            expand_size, kernel_heap.end_address);
}

// main memory allocation function
void *kmalloc(size_t size) {
    if (size == 0) {
        return NULL;
    }
    
    // find a suitable free block
    heap_block_t *block = heap_find_free_block(size);
    
    if (block == NULL) {
        // no suitable block found, try to expand the heap
        heap_expand_heap(size);
        block = heap_find_free_block(size);
        
        if (block == NULL) {
            kprintf("kmalloc failed: out of memory (requested %zu bytes)\n", size);
            return NULL;
        }
    }
    
    // split the block if it's much larger than needed
    block = heap_split_block(block, size);
    
    kernel_heap.used_blocks++;
    
    // return pointer to the data area (after the header)
    return (void *)((char *)block + sizeof(heap_block_t));
}

// main memory deallocation function
void kmfree(void *ptr) {
    if (ptr == NULL) {
        return;
    }
    
    // get the block header
    heap_block_t *block = (heap_block_t *)((char *)ptr - sizeof(heap_block_t));
    
    // validate the block
    if (block->magic != HEAP_MAGIC) {
        kprintf("kmfree: invalid block magic number\n");
        return;
    }
    
    if (block->used == 0) {
        kprintf("kmfree: double free detected\n");
        return;
    }
    
    // mark the block as free
    block->used = 0;
    kernel_heap.used_blocks--;
    
    // try to merge with adjacent free blocks
    heap_merge_adjacent_blocks(block);
}

// print heap information for debugging
void heap_print_info(void) {
    kprintf("heap info:\n");
    kprintf("  start: 0x%llx\n", kernel_heap.start_address);
    kprintf("  end: 0x%llx\n", kernel_heap.end_address);
    kprintf("  max: 0x%llx\n", kernel_heap.max_address);
    kprintf("  total blocks: %u\n", kernel_heap.total_blocks);
    kprintf("  used blocks: %u\n", kernel_heap.used_blocks);
    kprintf("  total size: %u bytes\n", heap_get_total_size());
    kprintf("  used size: %u bytes\n", heap_get_used_size());
    kprintf("  free size: %u bytes\n", heap_get_free_size());
}

// validate heap integrity by checking all blocks
void heap_validate_integrity(void) {
    heap_block_t *current = kernel_heap.first_block;
    uint32_t block_count = 0;
    
    while (current != NULL) {
        if (current->magic != HEAP_MAGIC) {
            kprintf("heap corruption detected at block %u\n", block_count);
            return;
        }
        block_count++;
        current = current->next;
    }
    
    if (block_count != kernel_heap.total_blocks) {
        kprintf("heap block count mismatch: expected %u, found %u\n", 
                kernel_heap.total_blocks, block_count);
    }
}

// get total heap size
uint32_t heap_get_total_size(void) {
    return (uint32_t)(kernel_heap.end_address - kernel_heap.start_address);
}

// get used heap size
uint32_t heap_get_used_size(void) {
    uint32_t used_size = 0;
    heap_block_t *current = kernel_heap.first_block;
    
    while (current != NULL) {
        if (current->used) {
            used_size += current->size;
        }
        current = current->next;
    }
    
    return used_size;
}

// get free heap size
uint32_t heap_get_free_size(void) {
    return heap_get_total_size() - heap_get_used_size();
}

// standard c library wrappers
void *malloc(size_t size) {
    return kmalloc(size);
}

void mfree(void *ptr) {
    kmfree(ptr);
}
