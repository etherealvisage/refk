#include "klib/avl.h"
#include "klib/kmem.h"
#include "klib/kutil.h"

#include "id.h"
#include "mman.h"

// memory management data structures
avl_tree_t root_map;
avl_tree_t root_refcount;
avl_tree_t page_refcount;

uint64_t this_root_id;

static void increment_page(uint64_t page);
static void decrement_page(uint64_t page);
static uint64_t import_root(uint64_t root);
static void remove_helper(uint64_t root, int level);

void mman_init(uint64_t bootproc_cr3) {
    d_printf("mman_init()\n");
    kmem_setup();

    avl_initialize(&root_map, avl_ptrcmp, 0);
    avl_initialize(&root_refcount, avl_ptrcmp, 0);
    avl_initialize(&page_refcount, avl_ptrcmp, 0);

    this_root_id = import_root(kmem_current());
    mman_increment_root(this_root_id);

    // release memory from boot process
    uint64_t bootproc_id = import_root(bootproc_cr3);
    mman_increment_root(bootproc_id);
    mman_decrement_root(bootproc_id);
}

static uint64_t paging_addr_create(uint64_t root_address, uint64_t vaddr,
    uint8_t level) {

    uint8_t ok;
    uint64_t addr = kmem_paging_addr(root_address, vaddr, level, &ok);

    if(!ok) {
        // need to allocate some intermediate entries
        uint64_t pa = -1;
        for(int i = 0; i <= level; i ++) {
            uint64_t a = kmem_paging_addr(root_address, vaddr, i, &ok);
            if(!ok) {
                uint64_t ntable = kmem_getpage();
                for(int i = 0; i < 512; i ++) phy_write64(ntable + i*8, 0);
                phy_write64(pa, ntable | 0x7);
                increment_page(ntable);
                a = kmem_paging_addr(root_address, vaddr, i, &ok);
            }
            pa = a;
        }

        addr = kmem_paging_addr(root_address, vaddr, level, &ok);
    }

    return addr;
}

int mman_anonymous(uint64_t root_id, uint64_t address, uint64_t size) {
    uint64_t root_address =
        (uint64_t)avl_search(&root_map, (void *)root_id);
    if(root_address == 0) return -1;

    if(address & 0xfff) return -1;
    if(size & 0xfff) return -1;

    while(size > 0) {
        uint64_t eaddr = paging_addr_create(root_address, address, 3);
        uint64_t paddr = kmem_getpage();
        phy_write64(eaddr, paddr | KMEM_MAP_DATA);
        increment_page(paddr);
        address += 0x1000;
        size -= 0x1000;
    }

    return 0;
}

int mman_physical(uint64_t root_id, uint64_t address, uint64_t paddress,
    uint64_t size) {

    uint64_t root_address =
        (uint64_t)avl_search(&root_map, (void *)root_id);
    if(root_address == 0) return -1;

    if(address & 0xfff) return -1;
    if(paddress & 0xfff) return -1;
    if(size & 0xfff) return -1;

    while(size > 0) {
        uint64_t eaddr = paging_addr_create(root_address, address, 3);
        phy_write64(eaddr, paddress | KMEM_MAP_DATA);

        address += 0x1000;
        paddress += 0x1000;
        size -= 0x1000;
    }

    return 0;
}

int mman_check_all_mapped(uint64_t root_id, uint64_t address, uint64_t size) {
    uint64_t root_address =
        (uint64_t)avl_search(&root_map, (void *)root_id);
    if(root_address == 0) return -1;

    if(address & 0xfff) return -1;
    if(size & 0xfff) return -1;

    while(size > 0) {
        uint8_t ok = 0;
        uint64_t eaddr = kmem_paging_addr(root_address, address, 3, &ok);
        if(!ok) {
            d_printf("OK failed\n");
            return 0;
        }
        if(!(phy_read64(eaddr) & 0x1)) {
            d_printf("address 0x%x not present!\n", address);
            return 0;
        }

        address += 0x1000;
        size -= 0x1000;
    }

    return 1;
}

int mman_check_any_mapped(uint64_t root_id, uint64_t address, uint64_t size) {
    uint64_t root_address =
        (uint64_t)avl_search(&root_map, (void *)root_id);
    if(root_address == 0) return -1;

    if(address & 0xfff) return -1;
    if(size & 0xfff) return -1;

    while(size > 0) {
        uint8_t ok = 0;
        uint64_t eaddr = kmem_paging_addr(root_address, address, 3, &ok);
        if(ok) {
            if(phy_read64(eaddr) & 0x1) return 1;
        }

        address += 0x1000;
        size -= 0x1000;
    }

    return 0;
}

int mman_mirror(uint64_t root_id, uint64_t address, uint64_t sroot_id,
    uint64_t saddress, uint64_t size) {

    uint64_t root_address =
        (uint64_t)avl_search(&root_map, (void *)root_id);
    if(root_address == 0) return -1;

    uint64_t sroot_address =
        (uint64_t)avl_search(&root_map, (void *)sroot_id);
    if(sroot_address == 0) return -1;

    if(mman_check_any_mapped(root_id, address, size) != 0) return 1;
    if(mman_check_all_mapped(sroot_id, saddress, size) != 1) return 1;

    while(size > 0) {
        uint64_t eaddr = paging_addr_create(root_address, address, 3);
        uint64_t saddr = paging_addr_create(sroot_address, saddress, 3);

        uint64_t sentry = phy_read64(saddr);
        phy_write64(eaddr, phy_read64(saddr));
        increment_page(sentry & ~KMEM_FLAG_MASK);

        address += 0x1000;
        saddress += 0x1000;
        size -= 0x1000;
    }

    return 0;
}

int mman_unmap(uint64_t root_id, uint64_t address, uint64_t size) {
    uint64_t root_address =
        (uint64_t)avl_search(&root_map, (void *)root_id);
    if(root_address == 0) return -1;

    if(address & 0xfff) return -1;
    if(size & 0xfff) return -1;

    while(size > 0) {
        uint8_t ok;
        uint64_t eaddr = kmem_paging_addr(root_address, address, 3, &ok);
        if(ok) {
            uint64_t entry = phy_read64(eaddr);
            if(entry & 1) {
                decrement_page(entry & ~KMEM_FLAG_MASK);
                phy_write64(eaddr, 0);
            }
        }

        address += 0x1000;
        size -= 0x1000;
    }

    return 0;
}

int mman_protect(uint64_t root_id, uint64_t address, uint64_t size,
    uint64_t flags) {

    uint64_t root_address =
        (uint64_t)avl_search(&root_map, (void *)root_id);
    if(root_address == 0) return -1;

    if(address & 0xfff) return -1;
    if(size & 0xfff) return -1;

    while(size > 0) {
        uint8_t ok;
        uint64_t eaddr = kmem_paging_addr(root_address, address, 3, &ok);
        if(ok) {
            uint64_t entry = phy_read64(eaddr);
            if(entry & 1) {
                //decrement_page(entry & ~KMEM_FLAG_MASK);
                phy_write64(eaddr, (entry & ~KMEM_FLAG_MASK) | flags);
            }
        }

        address += 0x1000;
        size -= 0x1000;
    }

    return 0;
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
        d_printf("releasing page %x\n", page);
    }
}

uint64_t mman_own_root() {
    return this_root_id;
}

uint64_t mman_make_root() {
    uint64_t address = kmem_create_root();

    return import_root(address);
}

uint64_t mman_import_root(uint64_t cr3) {
    return import_root(cr3);
}

void mman_increment_root(uint64_t root) {
    uint64_t current = (uint64_t)avl_search(&root_refcount, (void *)root);
    avl_insert(&root_refcount, (void *)root, (void *)(current + 1));
}

static void remove_helper(uint64_t root, int level) {
    for(uint64_t i = 0; i < 512; i ++) {
        // skip physical memory map
        if(level == 0 && i == 384) continue;

        // read entry
        uint64_t entry = phy_read64(root + i*8);

        // if entry not present, continue
        if((entry & 1) == 0) continue;

        uint64_t page = entry & ~KMEM_FLAG_MASK;

        if(level < 3) remove_helper(page, level+1);

        decrement_page(page);
    }
}

void mman_decrement_root(uint64_t root) {
    uint64_t current = (uint64_t)avl_search(&root_refcount, (void *)root);

    if(current == 0) return;
    else if(current > 1) {
        avl_insert(&root_refcount, (void *)root, (void *)(current - 1));
    }
    else {
        avl_remove(&root_refcount, (void *)root);
        uint64_t cr3 = (uint64_t)avl_remove(&root_map, (void *)root);
        remove_helper(cr3, 0);

        decrement_page(cr3);
        d_printf("Removed root with CR3 %x\n", cr3);
    }
}

int mman_is_root(uint64_t root) {
    return !!avl_search(&root_map, (void *)root);
}

uint64_t mman_get_root_cr3(uint64_t root) {
    return (uint64_t)avl_search(&root_map, (void *)root);
}

static void import_helper(uint64_t root, int level) {
    for(uint64_t i = 0; i < 512; i ++) {
        // skip physical memory map
        if(level == 0 && i == 384) continue;

        // read entry
        uint64_t entry = phy_read64(root + i*8);

        // if entry not present, continue
        if((entry & 1) == 0) continue;

        uint64_t page = entry & ~KMEM_FLAG_MASK;
        increment_page(page);

        if(level < 3) import_helper(page, level+1);
    }
}

static uint64_t import_root(uint64_t root) {
    // root page is in use
    increment_page(root);
    // mark everything else as in use
    import_helper(root, 0);

    uint64_t id = gen_id();
    avl_insert(&root_map, (void *)id, (void *)root);

    return id;
}
