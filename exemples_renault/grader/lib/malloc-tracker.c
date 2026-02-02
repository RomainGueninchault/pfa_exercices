// This define is mandatory to enable RTLD_NEXT
#define _GNU_SOURCE
#include "grader2.h"

#include <dlfcn.h>
#include <stdlib.h>
#include <unistd.h>

static void* (*real_malloc)(size_t size) = 0;
static void* (*real_calloc)(size_t count, size_t size) = 0;
static void* (*real_realloc)(void* ptr, size_t size) = 0;
static int  (*real_posix_memalign)(void **ptr, size_t align, size_t size) = 0;
static void  (*real_free)(void* ptr) = 0;

static int tracker_enabled;

jmp_buf mem_tracker_env;

// Block list management

/**
 * Allocated/deallocated adresses are stored in a linked list (struct a_list).  
 * For each allocated memory, we store its size ams status.
 * 
 * In order to avoid memory corruption as mush as possible, cells (struct 
 * a_list) are allocated in blocks that take one page of memory. The last
 * sizeof(void*) bytes of the blocks are used to chain the block together.
 * 
 * The data structure is implemented by struct a_block. Free cells (struct 
 * a_list) are in free_list, used ones are in used_list, and block_list points
 * to the head block.
*/

enum a_status {
    IN_USE,
    FREED,
};

struct a_list {
    void *ptr;            // Allocated pointer
    size_t size;          // Allocated size
    enum a_status status; // Status of allocated pointer
    struct a_list *next;  // Next in allocated chain
};

struct a_block {
    struct a_list *used_list, *free_list;
    struct a_list *block_list;
    struct a_stats stats;
} malloc_superblock;

static long BLOCK_SIZE; // it's not a constant but should be used like if it was one

static void compute_block_size()
{
    BLOCK_SIZE = (sysconf(_SC_PAGESIZE) - sizeof(void *)) / sizeof(struct a_list); // -1
}

#define next_block_ptr(b) ((struct a_list **)((char *)b + sizeof(struct a_list) * BLOCK_SIZE))

static void load_func(void **fun_hdl, const char *name)
{
    void *ptr = *fun_hdl = dlsym(RTLD_NEXT, name);
    if (!ptr) {
        fprintf(stderr,
                "Can not grab symbol %s: %s\n", name, dlerror());
        exit(1);
    }
}

#define INITIALIZE(name, ...)                       \
    do {                                            \
        if (!real_##name)                           \
            load_func((void**)&real_##name, #name); \
    } while(0)

static struct a_list *allocate_block()
{
    if (BLOCK_SIZE == 0) {
        perror("Memory tracker has not been initialized");
        exit(EXIT_FAILURE);
    }

    struct a_list *new_block = real_malloc(sizeof(*new_block) * BLOCK_SIZE + sizeof(void *));

    int i = BLOCK_SIZE - 1;
    while (i --)
        new_block[i].next = new_block + i + 1;

    malloc_superblock.free_list = new_block;

    *next_block_ptr(new_block) = malloc_superblock.block_list;
    malloc_superblock.block_list = new_block;

    return malloc_superblock.free_list;
}

static struct a_list* assign(struct a_list *info, size_t size, enum a_status status, void *ptr)
{
    info->size = size;
    info->status = status;
    info->ptr = ptr;
    return info;
}

static struct a_list* locate_ptr(void *ptr)
{
    struct a_list* node = malloc_superblock.used_list;

    while(node && node->ptr != ptr)
        node = node->next;

    return node;
}

static void register_ptr(size_t size, void *ptr)
{
    struct a_list * node = locate_ptr(ptr);
    if (node != NULL) {
        if (node->status == IN_USE) {
            perror("Cannot register pointer that is already in use");
            exit(EXIT_FAILURE);
        }
        node = assign(node, size, IN_USE, ptr);
    }
    else {
        node = malloc_superblock.free_list;
        if (node == NULL && !(node = allocate_block())) {
            perror("Can't allocate new block");
            exit(EXIT_FAILURE);
        }
        malloc_superblock.free_list = node->next;      // update free_list
        node->next = malloc_superblock.used_list;      // update used_list
        malloc_superblock.used_list = assign(node, size, IN_USE, ptr); // update cell
    }
}

static void free_all_blocks()
{
    for (struct a_list * b = malloc_superblock.block_list; b != NULL;) {
        struct a_list * to_free = b;
        b = *next_block_ptr(b);
        real_free(to_free);
    }
    malloc_superblock.block_list = NULL;
}

// Allocation / deallocation functions

/**
 * Our own implementation of allocation and deallocation functions. These 
 * functions track memory leaks, double free, free NULL, free of non allocated
 * address.
 * 
 * Memory leaks are detected using macro ASSERT_MEMORY_OK (see malloc-tracker.h)
 * but invalid frees are detected within functions tracker_free() and 
 * tracker_realloc(). In order to fail the corresponding test, MEMORY_FAILURE() 
 * is used to perform a longjmp to the FINALISE_MEMORY_TRACKER() instruction in 
 * the test. The jump is set by INITIALIZE_MEMORY_TRACJER()
 * (cf. memory-tracker.h)
*/

#define MEMORY_FAILURE()                  \
    do {                                  \
        longjmp(mem_tracker_env, 1);      \
    }                                     \
    while(0)

void * tracker_malloc(size_t size)
{
    INITIALIZE(malloc, size);
    void * ptr = real_malloc(size);
    if (tracker_enabled) {
        malloc_superblock.stats.allocated ++;
        malloc_superblock.stats.allocated_size += size;
        if (ptr)
            register_ptr(size, ptr);
    }
    return ptr;
}

void * tracker_calloc(size_t count, size_t size)
{
    INITIALIZE(calloc, count, size);
    void * ptr = real_calloc(count, size);
    if (tracker_enabled) {
        malloc_superblock.stats.allocated ++;
        malloc_superblock.stats.allocated_size += size;
        if (ptr)
            register_ptr(count*size, ptr);
    }
    return ptr;
}

int tracker_posix_memalign(void** ptr, size_t align, size_t size)
{
    INITIALIZE(posix_memalign, ptr, align, size);
    int res = real_posix_memalign(ptr, align, size);
    if (tracker_enabled) {
        malloc_superblock.stats.allocated ++;
        if (!res) {
            malloc_superblock.stats.allocated_size += size;
            register_ptr(size, *ptr);
        }
    }
    return res;
}

void tracker_free(void* ptr)
{
    INITIALIZE(free, ptr);
    if (!tracker_enabled) {
        real_free(ptr);
        return;
    }

    if (ptr == NULL) {
        malloc_superblock.stats.fault ++;
        MSG(COLOR(FAILED, "Cannot free %p")": NULL pointer\n", ptr);
        MEMORY_FAILURE();
    }
    else {
        struct a_list * node = locate_ptr(ptr);
        if (node == NULL) {
            malloc_superblock.stats.fault ++;
            MSG(COLOR(FAILED, "Cannot free %p")": pointer has not been heap-allocated\n", ptr);
            MEMORY_FAILURE();
        }
        else if (node->status == FREED) {
            malloc_superblock.stats.fault ++;
            MSG(COLOR(FAILED, "Cannot free %p")": pointer has already been freed\n", ptr);
            MEMORY_FAILURE();
        }
        else {
            malloc_superblock.stats.freed_size += node->size;
            malloc_superblock.stats.freed ++;
            node = assign(node, 0, FREED, node->ptr); // update status
            real_free(ptr);
        }
    }
}

void * tracker_realloc(void* ptr, size_t size)
{
    INITIALIZE(realloc, ptr, size);
    if (!tracker_enabled)
        return real_realloc(ptr, size);

    if (!ptr) {
        void * ptr = real_realloc(NULL, size);
        malloc_superblock.stats.reallocated ++;
        malloc_superblock.stats.allocated_size += size;
        if (ptr)
            register_ptr(size, ptr);
        return ptr;
    }

    struct a_list * node = locate_ptr(ptr);
    if (node == NULL) {
        malloc_superblock.stats.fault ++;
        MSG(COLOR(FAILED, "Cannot realloc %p")": pointer is not allocated\n", ptr);
        MEMORY_FAILURE();
        return NULL;   // never executed due to MEMORY_FAILURE()
    }

    malloc_superblock.stats.allocated_size -= node->size;

    void * new_ptr = real_realloc(ptr, size);

    malloc_superblock.stats.reallocated ++;
    malloc_superblock.stats.allocated_size += size;

    if (new_ptr != ptr) {
        node = assign(node, 0, FREED, ptr);
        if (new_ptr != NULL)
            register_ptr(size, new_ptr);
    }
    else
        node = assign(node, size, IN_USE, new_ptr);

    return new_ptr;
}

#ifndef MACRO_REDEFINE

void * malloc(size_t size)
{
    return tracker_malloc(size);
}

void * calloc(size_t count, size_t size)
{
    return tracker_calloc(count, size);
}

int posix_memalign(void** ptr, size_t align, size_t size)
{
    return tracker_posix_memalign(ptr, align, size);
}

void free(void* ptr)
{
    tracker_free(ptr);
}

void *realloc(void* ptr, size_t size)
{
    return tracker_realloc(ptr, size);
}

#endif // MACRO_REDEFINE

/** Public API
 * This functions should be accessed using the corresponding macros in
 * malloc-tracker.h
 */ 

struct a_stats get_a_stats()
{
    return malloc_superblock.stats;
}

unsigned long get_fault()
{
    return malloc_superblock.stats.fault;
}

void enable_tracker()
{
    tracker_enabled = 1;
}

void disable_tracker()
{
    tracker_enabled = 0;
}

size_t pointer_info(void *ptr)
{
    struct a_list *node = locate_ptr(ptr);
    if (node)
        return node->size;
    return 0;
}

void report_memory_leaks()
{
    for (struct a_list * node = malloc_superblock.used_list; node != NULL; node = node->next) {
        if (node->status == IN_USE) {
            MSG(COLOR(FAILED, "lost")": %zu byte(s) at %p", node-> size, node->ptr);
        }
    }
}

void initialize_memory_tracking()
{
#ifdef MACRO_REDEFINE
    printf("********* Running in macro redefine mode *********\n");
#else
    printf("********* Running in global redefine mode *********\n");
#endif // MACRO_REDEFINE
    compute_block_size();
    enable_tracker();
}

void finalize_memory_tracking()
{
    disable_tracker();
    free_all_blocks();
}

void suspend_memory_tracking()
{
    disable_tracker();
}

void resume_memory_tracking()
{
    enable_tracker();
}
