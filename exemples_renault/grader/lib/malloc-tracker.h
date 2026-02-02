#ifndef MALLOC_TRACKER
#define MALLOC_TRACKER

#include <setjmp.h>

#include "grader2.h"

/**
 * Memory tracker usage
 *
 * Tracks calls to malloc, calloc, realloc and susped in order to detect
 * - double free
 * - invalid free (free of address that has not been allocated)
 * - invalid realloc
 * - memory leaks
 *
 * The code to check need to be enclosed between the initializer and the
 * finalizer to handle internal data structures properly. Tracking can be
 * suspended and resumed:
 *
 * INITIALIZE_MEMORY_TRACKER()
 * ...   // tracked block of code
 * SUSPEND_MEMORY_TRACKER()
 * ..    // untracked block of code
 * RESUME_MEMORY_TRACKER()
 * ..    // tracked block of code
 * FINALIZE_MEMORY_TRACKER()
 *
 * It is also possible to use several memory tracking blocks within the same
 * test case ALTHOUGH THIS IS NOT RECOMMENDED. The blocks shall be given
 * distinct identifiers:
 *
 * INITIALIZE_MEMORY_TRACKER(1)
 * ...   // tracked block of code
 * FINALIZE_MEMORY_TRACKER(1)
 * ...   // untrack block of code
 * INITIALIZE_MEMORY_TRACKER(2)
 * ...   // tracked block
 * FINALIZE_MEMORY_TRACKER(2)
 *
 * Notice that using two blocks is different from suspending/resuming within
 * the same block. Indeed, the the former case, all memory tracking is lost
 * when the first tracker is finalized, whereas, in the later case, this
 * information is still there when tracking is resumed.
 *
 * Finally, at any point WITHIN a tracking block, we can get memory usage
 * statistics and check for absence of faults and memory leaks:
 *
 * INITIALIZE_MEMORY_TRACKER()
 * ...
 * struct a_stats begin = GET_A_STATS()
 * ...
 * struct a_stats end = GET_A_STATS()
 * ...
 * // check that 4 bytes have been allocated between begin and end
 * ASSERT_UINT(4, end.allocated_size - begin.allocated_size, "...");
 *
 * // check absence of faults and memory leaks, in-between begin and end
 * ASSERT_MEMORY_OK(begin, end);
 * ...
 * FINALIZE_MEMORY_TRACKER()
 * 
 * The memory tracker implements two modes of tracking:
 * - macro redefiniton of allocators / deallocators will track calls only in 
 * files which include this header file.
 * - global redefinition of symbols will track every call to an allocator or a 
 * deallocator in the code.
 * The default behavior is global redefinition. Macro redefinition can be 
 * selected by defining symbol MACRO_REDEFINE at compilation time (flag 
 * -DMACRO_REDEFINE)
*/

// Used to finalize on memory faults
extern jmp_buf mem_tracker_env;

// Return number of memory faults
unsigned long get_fault();

// Initialize data structures for memory tracking
// And starts tracking of all calls to: malloc, calloc, realloc and free
#define INITIALIZE_MEMORY_TRACKER(n)                 \
    do {                                             \
        initialize_memory_tracking();                \
        if (setjmp(mem_tracker_env))                 \
            goto finalize##n;                        \
    }                                                \
    while (0)

// Free data structures for memory tracking
// And stops tracking of all calls to: malloc, calloc, realloc and free
#define FINALIZE_MEMORY_TRACKER(n)                           \
    do {                                                     \
finalize##n:                                                 \
        finalize_memory_tracking();                          \
        ASSERT_UINT(0, get_fault(), "memory fault(s) ");     \
    }                                                        \
    while (0)

// Suspend tracking of calls to malloc, calloc, realloc and free
#define SUSPEND_MEMORY_TRACKER() ({suspend_memory_tracking();})

// Resume tracking of calls to malloc, calloc, realloc and free
#define RESUME_MEMORY_TRACKER() ({resume_memory_tracking();})

// Should be called in-between INITIALIZE_MEMORY_TRACKER and
// FINALIZE_MEMORY_TRACKER to get statistics on memory usage
#define GET_A_STATS() ({get_a_stats();})

// Should be called in-between INITIALIZE_MEMORY_TRACKER and
// FINALIZE_MEMORY_TRACKER to assert that memory states
// __begin, __end do not exhibit faults or memory leaks
#define ASSERT_MEMORY_OK(__begin, __end)                                       \
    do {                                                                       \
        long allocated = __end.allocated_size - __begin.allocated_size;        \
        long freed = __end.freed_size - __begin.freed_size;                    \
        long leaks = allocated - freed;                                        \
        long faults = __end.fault - __begin.fault;                             \
        if (leaks) {                                                           \
            MSG(COLOR(FAILED, "Memory leak"));                                 \
            report_memory_leaks();                                             \
        }                                                                      \
        if (faults)                                                            \
            MSG(COLOR(FAILED, "Invalid free or realloc")" on %ld pointer%s",   \
                    faults, faults == 1 ? "" : "s");                           \
        if (leaks || faults) {                                                 \
            FAILED("Called by:\n\t%s", last_call_buffer);                      \
            return;                                                            \
        } else                                                                 \
            PASSED();                                                          \
    } while (0)

void report_memory_leaks();
void initialize_memory_tracking();
void finalize_memory_tracking();
void suspend_memory_tracking();
void resume_memory_tracking();

#include "macro-tracker.h"

#endif // MALLOC_TRACKER
