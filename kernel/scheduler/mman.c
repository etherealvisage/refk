#include "klib/avl.h"
#include "klib/kmem.h"
#include "klib/kutil.h"

#include "mman.h"

// memory management data structures
avl_tree_t memory_roots;
avl_tree_t root_refcount;
avl_tree_t page_refcount;

static void increment_page(uint64_t page);
static void decrement_page(uint64_t page);
static void import_root(uint64_t root);

void mman_init(uint64_t bootproc_cr3) {
    d_printf("mman_init()\n");
    avl_initialize(&memory_roots, avl_ptrcmp, 0);
    avl_initialize(&root_refcount, avl_ptrcmp, 0);
    avl_initialize(&page_refcount, avl_ptrcmp, 0);

    d_printf("importing boot cr3\n");
    import_root(bootproc_cr3);
    d_printf("importing scheduler cr3\n");
    import_root(kmem_current());
    d_printf("all roots imported\n");
}

static void increment_page(uint64_t page) {
    uint64_t current = (uint64_t)avl_search(&page_refcount, (void *)page);
    avl_insert(&page_refcount, (void *)page, (void *)(current + 1));
}

static void decrement_page(uint64_t page) {
    uint64_t current = (uint64_t)avl_search(&page_refcount, (void *)page);

    // can't decrement a page already fully-decremented
    if(current == 0) return;
    // case: at least one reference remaining
    else if(current > 1) {
        avl_insert(&page_refcount, (void *)page, (void *)(current - 1));
    }
    // case: time to release
    else {
        avl_remove(&page_refcount, (void *)page);
        kmem_unuse(page);
    }
}

static void import_helper(uint64_t root, int level) {
    for(uint64_t i = 0; i < 512; i ++) {
        if(level == 0 && i == 384) break;

        // read entry
        uint64_t entry = phy_read64(root + i*8);

        // if entry not present, continue
        if((entry & 1) == 0) continue;

        uint64_t page = entry & ~KMEM_FLAG_MASK;
        increment_page(page);

        if(level < 3) import_helper(page, level+1);
    }
}

static void import_root(uint64_t root) {
    // root page is in use
    increment_page(root);
    // mark everything else as in use
    import_helper(root, 0);
}
