#include "malloc.h"
#include "mm.h"
#include "uart.h"

void init_mm()
{
    init_buddy_allocator();
    init_slab_allocator();
}

static page_t **frame_array;
void init_buddy_allocator()
{
    // TODO: create multiple buddy systems (use up all available memory)
    frame_array = (page_t **) kmalloc(sizeof(page_t *) * ((1 << (MM_MAX_ORDER + 1))));
    frame_array[0] = build_frame(MM_MAX_ORDER);
}

void *allocate(int size)
{
    // TODO: not smaller than 4096, but 2048 instead (2048 ~ 4096 -> allocate a page)
    if (size < (1 << 11)) {     // slab allocator
        return slab_allocate(size);
    }
    // buddy allocator
    int pfn = find_page(0, size);
    frame_array[pfn]->flag = USED;
    uart_puts("pfn: ");
    uart_putints(pfn);
    uart_puts("\n\n\n\n");
    return MM_START + (pfn << 12);
}

page_t *build_frame(int order)
{
    page_t *p = (page_t *) kmalloc(sizeof(page_t));
    p->flag = AVAILABLE;
    p->order = order;
    p->slab_id = MM_BUDDY;
    return p;
}

int find_page(int index, int size)
{
    int flag = frame_array[index]->flag;
    int order = frame_array[index]->order;

    // return order 0 page
    if (order == 0) {
        // page used or page size not sufficient
        if (flag != AVAILABLE || (1 << order) * 0x1000 < size) {
            return MM_MAX;
        }
        return index;
    }

    // 目前的 page 可用，且下一層 size 不夠
    if (flag == AVAILABLE && (1 << (order-1)) * 0x1000 < size) {
        return index;
    }

    // split current page if available
    if (flag == AVAILABLE && !frame_array[left(index)] && !frame_array[right(index)]) {
        if (index == 32) {
            uart_puts("32 got splitted!\n");
        }
        frame_array[index]->flag = USED;
        // TODO: build left/right if NULL, else just update flag
        frame_array[left(index)] = build_frame(order-1);
        frame_array[right(index)] = build_frame(order-1);
    }

// 不能兩邊都找，否則第一次 traverse 就會把整顆樹都切到 order 1
    // find left and right
    uart_putints(index);
    uart_puts(" left: ");
    uart_putints(left(index));
    uart_puts("\n");
    int left_pfn = find_page(left(index), size);
    if (left_pfn != MM_MAX) {
        return left_pfn;
    }
    uart_putints(index);
    uart_puts(" right: ");
    uart_putints(right(index));
    uart_puts("\n");
    int right_pfn = find_page(right(index), size);
    return right_pfn;
}

void free(void *addr)
{
    int me = ((long) addr - MM_START) >> 12;
    int sibling = sibling(me);
    int parent;

    while (1) {
        // TODO: 還要 check me/sibling 是否存在吧？ -> 但向下 split 時一定會產生 sibling
        if (frame_array[me]->flag == USED && frame_array[sibling]->flag == AVAILABLE) {
            // merge
            frame_array[me]->flag = AVAILABLE;
            // update me and sibling
            me = parent(me);
            sibling = sibling(me);
            frame_array[me]->flag = AVAILABLE;
        } else {
            break;
        }
    }
}

static slab_t *slabs[MM_CHUNK_SIZES];
static int slab_chunk_sizes[] = {8, 16, 32, 64, 128, 256, 512, 1024, 2048};
// NOTE: pages given to slab won't be released back to buddy again
void init_slab_allocator(void)
{
    // allocate slabs with chunk size [8, 16, 32, 64, 128, 256, 512, 1024, 2048]
    int pfn;

    chunk_t *chunk;
    chunk_t *cur;
    for (int i = 0; i < MM_CHUNK_SIZES; i++) {
        chunk = NULL;
        cur = NULL;
        slabs[i] = (slab_t *) kmalloc(sizeof(slab_t));

        // get one page
        pfn = find_page(0, 1 << 12);
        frame_array[pfn]->flag = USED;
        frame_array[pfn]->slab_id = i;
        uart_puts("slab pfn: ");
        uart_putints(pfn);
        uart_puts("\n\n\n\n");

        // split page into chunks
        int chunks_count = (1 << 12) / slab_chunk_sizes[i];
        for (int k = 0; k < chunks_count; k++) {
            chunk = MM_START + (pfn << 12) + (slab_chunk_sizes[i] * k);
            chunk->next = NULL;

            if (cur == NULL) {
                cur = chunk;
                slabs[i]->freelist = cur;
                slabs[i]->chunk_size = slab_chunk_sizes[i]; // TODO: not required
            } else {
                cur->next = chunk;
                cur = chunk;
            }
        }
    }
}

void *slab_allocate(int size)
{
    // find closest chunk size
    int i;
    for (i = MM_CHUNK_SIZES - 1; i >= 0; i--) {
        if (slab_chunk_sizes[i] < size) {
            break;
        }
    }
    i += 1;

    uart_puts("allocate chunk from slab-");
    uart_putints(slab_chunk_sizes[i]);
    uart_puts("\n");

    uart_putlong((long) slabs[i]->freelist);
    uart_puts("\n");
    // request page from buddy if no free chunk
    chunk_t *chunk = slabs[i]->freelist;
    slabs[i]->freelist = chunk->next;

    // return free chunk's address
    return chunk;
}

void slab_free(void *addr)
{
    // Find page by addr, then Find slab by slab_id on page
    int pfn = ((long) addr - MM_START) >> 12;
    page_t *p = frame_array[pfn];
    slab_t *slab = slabs[p->slab_id];

    // insert chunk to freelist
    ((chunk_t *) addr)->next = slab->freelist;
    slab->freelist = addr;
}