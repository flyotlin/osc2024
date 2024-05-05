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
    // page_t *p = frame_array[pfn];
    // uart_putints(p->order);
    uart_puts("pfn: ");
    uart_putints(pfn);
    uart_puts("\n\n\n\n");
    // return NULL;
    return MM_START + (pfn << 12);
    // int i = 0;
    // page_t *cur = frame_array[i];
    // page_t *prev = cur;
    // while (1 << cur->order > size) {
    //     // return order 0 page
    //     if (cur->order == 0) {
    //         prev = cur;
    //         break;
    //     }
    //     // split current page if not exist
    //     if (!frame_array[left(i)] && !frame_array[right(i)]) {
    //         frame_array[i]->flag = USED;
    //         frame_array[left(i)] = build_frame(cur->order - 1);
    //         frame_array[right(i)] = build_frame(cur->order - 1);
    //     }

    //     if (frame_array[left(i)]->flag == AVAILABLE) {

    //     } else if (frame_array[left(i)]->flag == AVAILABLE) {

    //     } else {    // no available page (run out of memory)
    //         return NULL;
    //     }
    // }

    // return prev
}

page_t *build_frame(int order)
{
    // uart_puts("order: ");
    // uart_putints(order);
    // uart_puts("\n");
    page_t *p = (page_t *) kmalloc(sizeof(page_t));
    p->flag = AVAILABLE;
    p->order = order;
    return p;
}

int find_page(int index, int size)
{
    int flag = frame_array[index]->flag;
    int order = frame_array[index]->order;
    // uart_putints(index);
    // uart_puts(" ");
    // uart_hex(order);
    // uart_puts("\n");

    // return order 0 page
    if (order == 0) {
        // page used or page size not sufficient
        if (flag != AVAILABLE || (1 << order) * 0x1000 < size) {
            return MM_MAX;
        }
        return index;
        // return flag == AVAILABLE ? index : MM_MAX;
    }

    if (index == 32) {
        uart_puts("32: ");
        uart_putints(flag);
        uart_puts(" ");
        uart_putints(order);
        uart_puts(" ");
        uart_putints(size);
        uart_puts(" ");
        uart_putints((1 << (order-1)) * 0x1000);
        uart_puts("\n");
    }

    // 目前的 page 可用，且下一層 size 不夠
    if (flag == AVAILABLE && (1 << (order-1)) * 0x1000 < size) {
        return index;
    }

    // if (flag == USED) {
    //     return MM_MAX;
    // }

    // split current page if available
    if (flag == AVAILABLE && !frame_array[left(index)] && !frame_array[right(index)]) {
        if (index == 32) {
            uart_puts("32 got splitted!\n");
        }
        frame_array[index]->flag = USED;
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

    // // return smaller page first (MAXX is not found)
    // return (left_pfn < right_pfn) ? left_pfn : right_pfn;
}