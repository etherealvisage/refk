#include "klib/task.h"
#include "klib/avl.h"
#include "klib/kutil.h"
#include "klib/kcomm.h"
#include "klib/sheap.h"

#include "id.h"
#include "task.h"
#include "mman.h"

#define TASK_CHANNEL_START 0xffff800000000000
#define LOCAL_CHANNEL_BASE 0xcadd40000
#define LOCAL_CHANNEL_SIZE 0x1000000
#define CHANNEL_SIZE 0x1000

avl_tree_t task_map;
avl_tree_t task_root;
avl_tree_t named_tasks;
avl_tree_t local_channel;

void task_init() {
    avl_initialize(&task_map, avl_ptrcmp, 0);
    avl_initialize(&task_root, avl_ptrcmp, 0);
    avl_initialize(&named_tasks, (avl_comparator_t)strcmp, sheap_free);
    avl_initialize(&local_channel, avl_ptrcmp, 0);
}

static uint64_t find_available_local() {
    for(uint64_t i = 0; i < LOCAL_CHANNEL_SIZE / CHANNEL_SIZE; i ++) {
        uint64_t addr = LOCAL_CHANNEL_BASE + i * LOCAL_CHANNEL_SIZE;
        if(mman_check_any_mapped(mman_own_root(), addr, 0x1000)) continue;

        return addr;
    }

    return -1;
}

static uint64_t add_channel(uint64_t root_id, uint64_t *target_addr) {
    uint64_t local_addr = find_available_local();
    for(uint64_t i = 0; ; i ++) {
        uint64_t caddr = TASK_CHANNEL_START + i*CHANNEL_SIZE;
        if(mman_check_any_mapped(root_id, caddr, CHANNEL_SIZE)) continue;

        mman_anonymous(root_id, caddr, CHANNEL_SIZE);
        mman_mirror(mman_own_root(), local_addr, root_id, caddr, CHANNEL_SIZE);

        *target_addr = caddr;

        break;
    }

    return local_addr;
}

static void task_setup(uint64_t id, uint64_t root_id, kcomm_t **sin,
    kcomm_t **sout, uint64_t *target_addr) {

    // create incoming message channel
    uint64_t local_addr = add_channel(root_id, target_addr);
    avl_insert(&local_channel, (void *)id, (void *)local_addr);

    *sin = (kcomm_t *)local_addr;
    *sout = (kcomm_t *)(local_addr + CHANNEL_SIZE/2);

    kcomm_init(*sin, CHANNEL_SIZE/2);
    kcomm_init(*sout, CHANNEL_SIZE/2);
}

uint64_t sched_task_attach(task_state_t *ts, kcomm_t **sin, kcomm_t **sout) {
    uint64_t id = gen_id();
    avl_insert(&task_map, (void *)id, ts);
    
    uint64_t root_id = mman_import_root(ts->cr3);
    mman_increment_root(root_id);
    avl_insert(&task_root, (void *)id, (void *)root_id);

    uint64_t t_addr;
    task_setup(id, root_id, sin, sout, &t_addr);

    ts->rdi = t_addr;
    ts->rsi = t_addr + CHANNEL_SIZE/2;

    return id;
}

void sched_set_name(uint64_t task_id, const char *name) {
    uint64_t len = strlen(name);
    char *name_copy = sheap_alloc(len+1);
    memcpy(name_copy, name, len+1);
    avl_insert(&named_tasks, name_copy, (void *)task_id);
}

uint64_t sched_named_task(const char *name) {
    return (uint64_t)avl_search(&named_tasks, (void *)name);
}

uint64_t sched_task_create(uint64_t root_id, kcomm_t **sin, kcomm_t **sout) {
    if(mman_is_root(root_id)) return -1;

    task_state_t *ts = task_create();

    uint64_t id = gen_id();
    avl_insert(&task_map, (void *)id, ts);

    mman_increment_root(root_id);
    ts->cr3 = mman_get_root_cr3(root_id);
    avl_insert(&task_root, (void *)id, (void *)root_id);

    uint64_t unused;
    task_setup(id, root_id, sin, sout, &unused);

    return id;
}

void sched_task_reap(uint64_t task_id) {
    task_state_t *ts = avl_search(&task_map, (void *)task_id);
    if(!ts) return;

    avl_remove(&task_map, (void *)task_id);
    avl_remove(&task_root, (void *)task_id);

    mman_decrement_root(ts->cr3);
    ts->state = 0;
    ts->cr3 = 0;

    void *local = avl_search(&local_channel, (void *)task_id);
    if(local) {
        mman_unmap(mman_own_root(), (uint64_t)local, CHANNEL_SIZE);
    }
}

void sched_set_state(uint64_t task_id, uint64_t index, uint64_t value) {
    task_state_t *ts = avl_search(&task_map, (void *)task_id);
    if(ts && index < 32) {
        uint64_t *indexed = (void *)ts;
        indexed[index] = value;
    }
}

uint64_t sched_get_root(uint64_t task_id) {
    return (uint64_t)avl_search(&task_root, (void *)task_id);
}
