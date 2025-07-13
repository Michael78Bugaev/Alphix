#pragma once
#ifndef HEAP_H
#define HEAP_H

#include <stddef.h>
#include <stdint.h>

// memory management constants
#define HEAP_START_ADDRESS    0x1000000  // 16mb - heap start
#define HEAP_INITIAL_SIZE     0x100000   // 1mb initial size
#define HEAP_MAX_SIZE         0x10000000 // 256mb max size
#define HEAP_BLOCK_SIZE       0x1000     // 4kb minimum block size
#define HEAP_MAGIC            0xDEADBEEF // magic number for integrity check

// memory block structure in heap
typedef struct heap_block {
    uint32_t magic;           // magic number for integrity check
    uint32_t size;            // block size (including header)
    uint32_t used;            // 1 if block is used, 0 if free
    struct heap_block *next;  // pointer to next block
    struct heap_block *prev;  // pointer to previous block
} heap_block_t;

// heap management structure
typedef struct {
    uint64_t start_address;   // heap start address
    uint64_t end_address;     // heap end address
    uint64_t max_address;     // heap max address
    heap_block_t *first_block; // pointer to first block
    uint32_t total_blocks;    // total number of blocks
    uint32_t used_blocks;     // number of used blocks
} heap_t;

// initialize memory management system
void heap_init(void);

// main memory allocation and deallocation functions
void *kmalloc(size_t size);
void kmfree(void *ptr);

// helper functions for heap operations
heap_block_t *heap_find_free_block(size_t size);
heap_block_t *heap_split_block(heap_block_t *block, size_t size);
void heap_merge_adjacent_blocks(heap_block_t *block);
void heap_expand_heap(size_t size);

// debug and diagnostic functions
void heap_print_info(void);
void heap_validate_integrity(void);
uint32_t heap_get_total_size(void);
uint32_t heap_get_used_size(void);
uint32_t heap_get_free_size(void);

// convenience macros
#define HEAP_ALIGN(size) (((size) + 7) & ~7)  // align to 8 bytes

#endif // HEAP_H