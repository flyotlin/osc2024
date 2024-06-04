#include "malloc.h"
#include "uart.h"

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
    /**
     * Allocator used for different size:
     *      >= 2048 --> buddy allocator
     *      else    --> slab allocator
    */
    if (size < (1 << 11)) {
        return slab_allocate(size);
    }
    return buddy_allocate(size);
}

void free(void *addr)
{
    // calculate which page (pfn) address belongs to
    int pfn = ((long) addr - MM_START) >> 12;

    // calculate which buddy address belongs to
    int buddy = ((long) addr - MM_START - pfn * (1 << 12)) / (1 << MM_MAX_ORDER);

    if (buddys[buddy][pfn]->slab_id == MM_BUDDY) {
        buddy_free(buddy, addr);
    } else {
        slab_free(buddy, addr);
    }
}
