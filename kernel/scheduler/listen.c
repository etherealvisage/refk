#include <stdint.h>

#include "klib/kcomm.h"
#include "klib/avl.h"
#include "klib/kutil.h"
#include "klib/task.h"

#include "listen.h"
#include "interface.h"

#define CHANNEL_BASE 0xcadd40000
#define CHANNEL_SIZE 0x1000

// task data structures
avl_tree_t task_map;
avl_tree_t named_tasks;


// other state
uint64_t g_next_id = 0;

static uint64_t next_id() {
    return ++ g_next_id;
}

static void process(kcomm_t *schedin, kcomm_t *schedout) {
    comm_in_packet_t in;
    comm_out_packet_t out;
    while(1) {
        uint64_t in_size;
        if(kcomm_get(schedin, &in, &in_size)) continue;

        switch(in.type) {
        case COMM_SET_NAME: {
            char *key = sheap_alloc(32);
            memcpy(key, in.set_name.name, 32);
            key[31] = 0;
            avl_insert(&named_tasks, in.set_name.name,
                (void *)in.set_name.task_id);
            break;
        }
        case COMM_GET_NAMED: {
            char *key = sheap_alloc(32);
            memcpy(key, in.get_named.name, 32);
            key[31] = 0;
            void *result = avl_search(&named_tasks, key);
            sheap_free(key);
            out.type = COMM_GET_NAMED;
            out.req_id = in.req_id;
            out.get_named.task_id = (uint64_t)result;
            kcomm_put(schedout, &out, sizeof(out));
            break;
        }
        case COMM_SPAWN: {
            task_state_t *ts = task_create();
            uint64_t nid = next_id();
            avl_insert(&task_map, ts, (void *)nid);
            ts->cr3 = in.spawn.cr3;
            out.req_id = in.req_id;
            out.type = COMM_SPAWN;
            out.spawn.task_id = nid;
            kcomm_put(schedout, &out, sizeof(out));
            break;
        }
        case COMM_SET_STATE: {
            uint64_t task_id = in.set_state.task_id;
            task_state_t *ts = avl_search(&task_map, (void *)task_id);
            if(ts) {
                uint64_t *indexed = (void *)ts;
                indexed[in.set_state.index] = in.set_state.value;
            }
            break;
        }
        default:
            d_printf("Unknown comm_in packet type!\n");
            break;
        }
    }
}

void listen() {
    avl_initialize(&task_map, avl_ptrcmp, 0);
    avl_initialize(&named_tasks, (avl_comparator_t)strcmp, sheap_free);

    g_next_id = 0x1000;

    while(1) {
        for(int i = 0; i < 0; i ++) {
            
        }
    }
}
