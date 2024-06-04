#include "malloc.h"
#include "uart.h"
#include "mm.h"

static char *heap_end = (char *) HEAP_START;
void *kmalloc(unsigned int size)
{
    // size not valid
    if (size <= 0 || size > HEAP_SIZE) {
        uart_puts("size not valid\n");
        return NULL;
    }

    // insufficient heap space
    if ((unsigned long) heap_end + size > HEAP_START + HEAP_SIZE) {
        uart_puts("insufficient heap space\n");
        return NULL;
    }

    char *address = heap_end;
    heap_end += size;
    return (void *) address;
}

void *malloc(int size)
{
    return mm_allocate(size);
}

void free(void *addr)
{
    mm_free(addr);
}
