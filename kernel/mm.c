#include "malloc.h"
#include "mm.h"
#include "uart.h"

static page_t **frame_array;
void init_buddy_system()
{
    frame_array = (page_t **) kmalloc(sizeof(page_t *) * ((1 << (MM_MAX_ORDER + 1))));
    frame_array[0] = (page_t *) kmalloc(sizeof(page_t));
    frame_array[0]->flag = AVAILABLE;
    frame_array[0]->order = MM_MAX_ORDER;
}

void *allocate(int size)
{
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
