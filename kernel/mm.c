#include "malloc.h"
#include "mm.h"
#include "uart.h"

static page_t ***buddys;    // array of frame_array (page_t **)
static page_t **frame_array;
static int total_buddys = 0;
static int n_buddys = 0;    // number of buddys
void init_mm()
{
    init_buddy_allocator();
    init_slab_allocator();
}

void init_buddy_allocator()
{
    // calculate number of buddy systems
    total_buddys = (MM_END - MM_START) / (1 << MM_MAX_ORDER);

    // allocate memory for all buddys
    buddys = (page_t ***) kmalloc(sizeof(page_t **) * total_buddys);
    allocate_one_buddy();
    frame_array = buddys[0];
}

/**
 * Allocate a new buddy contiguously, and a root page frame
*/
void allocate_one_buddy()
{
    if (n_buddys >= total_buddys) {
        return;
    }

    // allocate space for one buddy (2^(MM_MAX_ORDER+1)-1 page frames)
    buddys[n_buddys] = (page_t **) kmalloc(sizeof(page_t *) * ((1 << (MM_MAX_ORDER + 1)) - 1));
    buddys[n_buddys][0] = build_frame(MM_MAX_ORDER);
    n_buddys += 1;
}

void *mm_allocate(int size)
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

void *buddy_allocate(int size)
{
    int pfn, buddy;
    for (buddy = 0; buddy < total_buddys; buddy++) {
        if (buddys[buddy] == NULL) {
            allocate_one_buddy();
        }
        pfn = find_page(buddy, 0, size);

        if (pfn != MM_MAX) {
            break;
        }
    }

    buddys[buddy][pfn]->flag = USED;
    return page_address(buddy, pfn);
}

page_t *build_frame(int order)
{
    page_t *p = (page_t *) kmalloc(sizeof(page_t));
    p->flag = AVAILABLE;
    p->order = order;
    p->slab_id = MM_BUDDY;
    return p;
}

/**
 * Find page of `size` in `buddy` system.
 * Starting from `index`-th node.
*/
int find_page(int buddy, int index, int size)
{
    int flag = buddys[buddy][index]->flag;
    int order = buddys[buddy][index]->order;

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
    if (flag == AVAILABLE && !buddys[buddy][left(index)] && !buddys[buddy][right(index)]) {
        if (index == 32) {
            uart_puts("32 got splitted!\n");
        }
        buddys[buddy][index]->flag = USED;
        // TODO: build left/right if NULL, else just update flag
        buddys[buddy][left(index)] = build_frame(order-1);
        buddys[buddy][right(index)] = build_frame(order-1);
    }

// 不能兩邊都找，否則第一次 traverse 就會把整顆樹都切到 order 1
    // find left and right
    uart_putints(index);
    uart_puts(" left: ");
    uart_putints(left(index));
    uart_puts("\n");
    int left_pfn = find_page(buddy, left(index), size);
    if (left_pfn != MM_MAX) {
        return left_pfn;
    }
    uart_putints(index);
    uart_puts(" right: ");
    uart_putints(right(index));
    uart_puts("\n");
    int right_pfn = find_page(buddy, right(index), size);
    return right_pfn;
}

void mm_free(void *addr)
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

void buddy_free(int buddy, void *addr)
{
    // buddy system free
    int me = ((long) addr - MM_START) >> 12;
    int sibling = sibling(me);

    while (1) {
        // TODO: 還要 check me/sibling 是否存在吧？ -> 但向下 split 時一定會產生 sibling
        if (buddys[buddy][me]->flag == USED && buddys[buddy][sibling]->flag == AVAILABLE) {
            // merge
            buddys[buddy][me]->flag = AVAILABLE;
            // update me and sibling
            me = parent(me);
            sibling = sibling(me);
            buddys[buddy][me]->flag = AVAILABLE;
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

        // TODO: assumes first buddy has at least 9 pages
        pfn = find_page(0, 0, 1 << 12);
        buddys[0][pfn]->flag = USED;
        buddys[0][pfn]->slab_id = i;
        uart_puts("slab pfn: ");
        uart_putints(pfn);
        uart_puts("\n\n\n\n");

        // split page into chunks
        int chunks_count = (1 << 12) / slab_chunk_sizes[i];
        for (int k = 0; k < chunks_count; k++) {
            chunk = (chunk_t *) ((long) (MM_START + (pfn << 12) + (slab_chunk_sizes[i] * k)));
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

void slab_free(int buddy, void *addr)
{
    // Find page by addr, then Find slab by slab_id on page
    int pfn = ((long) addr - MM_START) >> 12;
    page_t *p = buddys[buddy][pfn];
    slab_t *slab = slabs[p->slab_id];

    // insert chunk to freelist
    ((chunk_t *) addr)->next = slab->freelist;
    slab->freelist = addr;
}
