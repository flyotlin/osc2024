#ifndef _DEF_MALLOC
#define _DEF_MALLOC

#define NULL         ((void *)0)
#define HEAP_START   0x2000000      // 0x10 breaks on rpi3b+
#define HEAP_SIZE    0x50000000

// kernel malloc
void *kmalloc(unsigned int);

// allocate memory by buddy system (if order n not found, search in order n+1)
void *malloc(int size);

// release memory back to buddy system (merge until buddy not exist)
void free(void *addr);

#endif
