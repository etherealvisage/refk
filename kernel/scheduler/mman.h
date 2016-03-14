#ifndef SCHEDULER_MMAN_H
#define SCHEDULER_MMAN_H

#include <stdint.h>

void mman_init(uint64_t bootproc_cr3);

int mman_anonymous(uint64_t root_id, uint64_t address, uint64_t size);
int mman_physical(uint64_t root_id, uint64_t address, uint64_t paddress,
    uint64_t size);
int mman_check_all_mapped(uint64_t root_id, uint64_t address, uint64_t size);
int mman_check_any_mapped(uint64_t root_id, uint64_t address, uint64_t size);
int mman_mirror(uint64_t root_id, uint64_t address, uint64_t sroot_id,
    uint64_t saddress, uint64_t size);
int mman_unmap(uint64_t root_id, uint64_t address, uint64_t size);
int mman_protect(uint64_t root_id, uint64_t address, uint64_t size,
    uint64_t flags);

uint64_t mman_own_root(void);
uint64_t mman_make_root(void);
uint64_t mman_import_root(uint64_t cr3);

uint64_t mman_get_phy(uint64_t root, uint64_t address);

void mman_set_pagefree_callback(void (*callback)(uint64_t address));

void mman_increment_root(uint64_t root);
void mman_decrement_root(uint64_t root);
int mman_is_root(uint64_t root);
uint64_t mman_get_root_cr3(uint64_t root);

#endif
