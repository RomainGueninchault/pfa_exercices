#ifndef MACRO_TRACKER_H
#define MACRO_TRACKER_H

#ifdef MACRO_REDEFINE

#include <stdlib.h>

void * tracker_malloc(size_t size);
void * tracker_calloc(size_t count, size_t size);
int tracker_posix_memalign(void** ptr, size_t align, size_t size);
void tracker_free(void* ptr);
void * tracker_realloc(void* ptr, size_t size);

#define malloc(size) ({tracker_malloc(size);})
#define calloc(count, size) ({tracker_calloc(count, size);})
#define posix_memalign(ptr, align, size) ({tracker_posix_memalign(ptr, align, size);})
#define free(ptr) ({tracker_free(ptr);})
#define realloc(ptr, size) ({tracker_realloc(ptr, size);})

#endif // MACRO_REDEFINE

#endif // MACRO_TRACKER_H
