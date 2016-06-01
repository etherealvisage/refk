#include "clib/comm.h"
#include "clib/mem.h"
#include "clib/str.h"
#include "clib/avl.h"
#include "clib/heap.h"

#include "klib/task.h"
#include "klib/d.h"

#include "id.h"
#include "task.h"
#include "mman.h"

#define TASK_CHANNEL_START 0xffff800000000000
#define LOCAL_CHANNEL_BASE 0xcadd40000
#define LOCAL_CHANNEL_SIZE 0x1000000
#define CHANNEL_SIZE 0x100000

#define LOCAL_STORAGE_BASE 0x510400000
#define LOCAL_STORAGE_SIZE 0x1000

#define TEMPORARY_MAP_ADDRESS 0x50000000

avl_tree_t task_map; // map from task ID to task_info_t *
avl_tree_t named_tasks; // map from strings to task IDs

void task_init() {
    avl_initialize(&task_map, avl_ptrcmp, 0);
    avl_initialize(&named_tasks, (avl_comparator_t)str_cmp, heap_free);
}

static uint64_t find_available_local() {
    for(uint64_t i = 0; i < LOCAL_CHANNEL_SIZE / CHANNEL_SIZE; i ++) {
        uint64_t addr = LOCAL_CHANNEL_BASE + i * LOCAL_CHANNEL_SIZE;
        if(mman_check_any_mapped(mman_own_root(), addr, 0x1000)) continue;

        return addr;
    }

    return -1;
}

static uint64_t add_channel(uint64_t root_id, uint64_t *addr) {
    uint64_t local_addr = find_available_local();
    for(uint64_t i = 0; ; i ++) {
        uint64_t caddr = TASK_CHANNEL_START + i*CHANNEL_SIZE;
        if(mman_check_any_mapped(root_id, caddr, CHANNEL_SIZE)) continue;

        mman_anonymous(root_id, caddr, CHANNEL_SIZE);
        mman_mirror(mman_own_root(), local_addr, root_id, caddr, CHANNEL_SIZE);

        *addr = caddr;

        break;
    }

    return local_addr;
}

static uint64_t add_storage(uint64_t root_id) {
    for(uint64_t i = 0; ; i ++) {
        uint64_t saddr = LOCAL_STORAGE_BASE + i * LOCAL_STORAGE_SIZE;
        if(mman_check_any_mapped(root_id, saddr, LOCAL_STORAGE_SIZE)) continue;

        mman_anonymous(root_id, saddr, LOCAL_STORAGE_SIZE);

        return saddr;
    }
    return 0;
}

static void task_setup(task_state_t *ts, task_info_t *info) {
    // initially not waiting on a synch object
    info->synch = 0;

    // point GS towards task-local storage
    ts->gs_base = add_storage(info->root_id);
    mman_mirror(mman_own_root(), TEMPORARY_MAP_ADDRESS, info->root_id,
        ts->gs_base, 0x1000);

    uint64_t *tls = (void *)TEMPORARY_MAP_ADDRESS;
    tls[0] = info->id;

    // create scheduler channel
    uint64_t addr;
    uint64_t local_addr = add_channel(info->root_id, &addr);
    //avl_insert(&local_schedchannel, (void *)id, (void *)local_addr);

    tls[1] = addr;
    tls[2] = addr + CHANNEL_SIZE/2;

    info->sin = (comm_t *)local_addr;
    info->sout = (comm_t *)(local_addr + CHANNEL_SIZE/2);

    comm_init(info->sin, CHANNEL_SIZE/2, COMM_BUCKETED | COMM_BUCKETSIZE2(8));
    comm_init(info->sout, CHANNEL_SIZE/2, COMM_SIMPLE);

    // create incoming message channel
    local_addr = add_channel(info->root_id, &addr);
    info->gin = (void *)local_addr;

    tls[3] = addr;

    comm_init(info->gin, CHANNEL_SIZE, COMM_SIMPLE);

    // unmap thread-local storage
    mman_unmap(mman_own_root(), TEMPORARY_MAP_ADDRESS, 0x1000);
}

uint64_t sched_task_attach(task_state_t *ts, task_info_t *info) {
    uint64_t id = gen_id();
    info->id = id;
    avl_insert(&task_map, (void *)id, info);
    
    uint64_t root_id = mman_import_root(ts->cr3);
    mman_increment_root(root_id);

    info->state = ts;
    info->root_id = root_id;

    task_setup(ts, info);

    return id;
}

void sched_set_name(uint64_t task_id, const char *name) {
    uint64_t len = str_len(name);
    char *name_copy = heap_alloc(len+1);
    mem_copy(name_copy, name, len+1);
    avl_insert(&named_tasks, name_copy, (void *)task_id);
}

uint64_t sched_named_task(const char *name) {
    return (uint64_t)avl_search(&named_tasks, (void *)name);
}

uint64_t sched_task_create(uint64_t root_id, task_info_t *info) {
    if(!mman_is_root(root_id)) return -1;

    task_state_t *ts = task_create();

    info->state = ts;

    uint64_t id = gen_id();
    info->id = id;
    avl_insert(&task_map, (void *)id, info);

    mman_increment_root(root_id);
    ts->cr3 = mman_get_root_cr3(root_id);
    info->root_id = root_id;

    task_setup(ts, info);

    return id;
}

task_info_t *sched_task_reap(uint64_t task_id) {
    task_info_t *info = avl_search(&task_map, (void *)task_id);
    if(!info) return 0;

    mman_decrement_root(info->state->cr3);
    info->state->state = 0;
    info->state->cr3 = 0;

    if(info->gin) {
        mman_unmap(mman_own_root(), (uint64_t)info->gin, CHANNEL_SIZE);
    }

    avl_remove(&task_map, (void *)task_id);

    return info;
}

void sched_set_state(uint64_t task_id, uint64_t index, uint64_t value) {
    task_info_t *info = avl_search(&task_map, (void *)task_id);

    if(info && index < 32) {
        uint64_t *indexed = (void *)info->state;
        indexed[index] = value;
    }
}

task_info_t *sched_get_info(uint64_t task_id) {
    return avl_search(&task_map, (void *)task_id);
}
