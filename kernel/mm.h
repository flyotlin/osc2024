#ifndef _DEF_MM
#define _DEF_MM

// #define MM_MAX_ORDER 24
#define MM_MAX_ORDER 16
#define MM_PAGE_SIZE 0x1000 // 4KB
#define MM_START 0x10000000
// #define MM_END 0x12000000
#define MM_END 0x20000000
#define MM_MAX ((1 << MM_MAX_ORDER) + 2)
#define MM_BUDDY -1     // not used by slab allocator

enum page_flag {
    USED,
    AVAILABLE,
};

typedef struct page {
    enum page_flag flag;
    int order;

    /**
     * Used to fast look-up to which slab page belongs.
     * MM_BUDDY (-1) if page is not used by slab allocator.
    */
    int slab_id;
} page_t;

void init_mm(void);

// release all available memory into buddy system
void init_buddy_allocator(void);
void allocate_one_buddy(void);
void *buddy_allocate(int size);
void buddy_free(int buddy, void *addr);
page_t *build_frame(int order);
int find_page(int buddy, int index, int size);

/**
 * start_address + (i-th buddy * buddy_size) + (pfn * page_size)
*/
#define page_address(buddy, page) ((void *) ((long) (MM_START + (buddy * (1 << MM_MAX_ORDER)) + (page * (1 << 12)))))

#define left(index) ((index << 1) + 1)
#define right(index) ((index << 1) + 2)
#define parent(index) ((index - 1) >> 1)
#define sibling(index) ((index % 2) ? index + 1 : index - 1)

#define MM_CHUNK_SIZES 9        // # of different chunk size
// Abstract ref: https://medium.com/jerrynotes/memcached-slab-allocation-1e9da346cc27
typedef struct slab {
    int chunk_size;
    struct chunk *freelist;     // TODO: is there another more efficient data structure to represent chunk?
} slab_t;

typedef struct chunk {
    struct chunk *next;
} chunk_t;

void init_slab_allocator(void);
void *slab_allocate(int size);
void slab_free(int buddy, void *addr);

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

__attribute__((unused)) static page_t ***buddys;    // array of frame_array (page_t **)
__attribute__((unused)) static page_t **frame_array;
__attribute__((unused)) static int total_buddys = 0;
__attribute__((unused)) static int n_buddys = 0;    // number of buddys

#endif
