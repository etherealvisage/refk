#include "klib/kutil.h"
#include "klib/sheap.h"
#include "klib/task.h"
#include "klib/avl.h"

#include "synch.h"
#include "task.h"
#include "mman.h"

typedef struct page_object_t {
    struct page_object_t *next;
    synchobj_t *object;
} page_object_t;

avl_tree_t synch_objects;
avl_tree_t synch_pages;

static void free_objects(uint64_t page_addr);

void synch_init() {
    avl_initialize(&synch_objects, avl_ptrcmp, sheap_free);
    avl_initialize(&synch_pages, avl_ptrcmp, 0);

    mman_set_pagefree_callback(free_objects);
}

synchobj_t *synch_make(uint64_t phy) {
    // NOTE: assumes mapped, permissions to make...

    // check for page overflow
    uint64_t over = (phy & 0xfff) + 8;
    if(over < 8) return 0;

    synchobj_t *ret = sheap_alloc(sizeof(*ret));

    ret->phy_addr = phy;
    ret->head = 0;

    page_object_t *pobj = sheap_alloc(sizeof(*pobj));
    pobj->object = ret;
    pobj->next = avl_insert(&synch_pages, (void *)(phy & ~0xfff), pobj);

    return ret;
}

void synch_destroy(synchobj_t *object) {
    synch_wait_t *w = object->head;
    while(1) {
        if(!w) break;

        task_info_t *info = sched_get_info(w->task_id);
        info->state->state &= ~TASK_STATE_BLOCKED;

        w = w->next;
        synch_wait_t *t = w;
        sheap_free(t);
    }

    page_object_t *pobj = avl_search(&synch_pages, (void *)(object->phy_addr & ~0xfff));
    // head case
    if(pobj->object == object) {
        avl_insert(&synch_pages, (void *)(object->phy_addr & ~0xfff), pobj->next);
        sheap_free(pobj);
    }
    else while(1) {
        if(!pobj) break;
        if(!pobj->next) break;
        page_object_t *n = pobj->next;

        if(n->object == object) {
            pobj->next = n->next;
            sheap_free(n);
            break;
        }
        else pobj = n;
    }

    sheap_free(object);
}

static void free_objects(uint64_t page_addr) {
    page_object_t *pobj;
    while((pobj = avl_search(&synch_pages, (void *)page_addr))) {
        synch_destroy(pobj->object);
    }
}

synchobj_t *synch_from_phy(uint64_t phy) {
    synchobj_t *result = avl_search(&synch_objects, (void *)phy);
    return result;
}

int synch_wait(synchobj_t *object, uint64_t value) {
    if(phy_read64(object->phy_addr) == value) {
        return 0;
    }
    else return 1;
}

void synch_wake(synchobj_t *object, uint64_t value, uint64_t count) {
    if(phy_read64(object->phy_addr) == value) {
        synch_wait_t *w = object->head;
        for(uint64_t i = 0; i < count; i ++) {
            if(!w) break;

            task_info_t *info = sched_get_info(w->task_id);
            info->state->state &= ~TASK_STATE_BLOCKED;

            w = w->next;
            synch_wait_t *t = w;
            sheap_free(t);
        }
    }
}
