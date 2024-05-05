#ifndef _DEF_MM
#define _DEF_MM

// #define MM_MAX_ORDER 16
#define MM_MAX_ORDER 6
#define MM_PAGE_SIZE 0x1000 // 4KB
#define MM_START 0x10000000
#define MM_END 0x20000000
#define MM_MAX (1 << MM_MAX_ORDER + 2)

enum page_flag {
    USED,
    AVAILABLE,
    // BUDDY,
};

typedef struct page {
    enum page_flag flag;
    int order;
    // unsigned long pfn;  // TODO: for calculating address
    // unsigned long flags;
    // void *freelist;

    // struct page *parent;
    // struct page *left;
    // struct page *right;
} page_t;


// release all available memory into buddy system
void init_buddy_system(void);
void *allocate(int size);
page_t *build_frame(int order);
int find_page(int index, int size);

#define left(index) ((index << 1) + 1)
#define right(index) ((index << 1) + 2)
#define parent(index) ((index - 1) >> 1)
// #define get_page_address(p) 1000

// TODO: move to memblock.h
// https://zhuanlan.zhihu.com/p/523699107
// https://elixir.bootlin.com/linux/v4.6/source/include/linux/memblock.h#L31
// struct memblock {
//     void *base;
//     void *size;
//     unsigned long type;
// } memblock_t;

// #define MEM_MEMBLOCK        1
// #define RESERVED_MEMBLOCK   2

// TODO: what is zone?

#endif
