#include <stdint.h>

#include "klib/kcomm.h"
#include "klib/avl.h"
#include "klib/kutil.h"
#include "klib/task.h"

#include "listen.h"
#include "interface.h"
#include "id.h"
#include "mman.h"
#include "task.h"

typedef struct {
    uint64_t task_id;
    kcomm_t *in, *out;
} queue_entry;

queue_entry queue[100];
int queue_size;

static void remove_from_queue(uint64_t id);

// task data structures
static int process(queue_entry *q) {
    sched_in_packet_t in;
    sched_out_packet_t status;
    uint64_t in_size;
    int ret = 0;
    while(!kcomm_get(q->in, &in, &in_size)) {
        d_printf("Processing packet of type %x\n", in.type);
        ret = 1;
        status.type = in.type;
        status.req_id = in.req_id;
        status.result = 0;
        switch(in.type) {
        case SCHED_MAP_ANONYMOUS: {
            uint64_t id = in.map_anonymous.root_id;
            if(id == 0) id = sched_get_root(q->task_id);
            status.result = mman_anonymous(id, in.map_anonymous.address,
                in.map_anonymous.size);
            break;
        }
        case SCHED_MAP_PHYSICAL: {
            uint64_t id = in.map_physical.root_id;
            if(id == 0) id = sched_get_root(q->task_id);
            status.result = mman_physical(id, in.map_physical.address,
                in.map_physical.phy_addr, in.map_physical.size);
            break;
        }
        case SCHED_UNMAP: {
            uint64_t id = in.unmap.root_id;
            if(id == 0) id = sched_get_root(q->task_id);
            status.result = mman_unmap(id, in.unmap.address,
                in.unmap.size);
            break;
        }
        case SCHED_SET_NAME: {
            in.set_name.name[31] = 0;
            uint64_t id = in.set_name.task_id;
            if(id == 0) id = q->task_id;
            sched_set_name(id, in.set_name.name);
            break;
        }
        case SCHED_GET_NAMED: {
            in.get_named.name[31] = 0;
            status.type = SCHED_GET_NAMED;
            status.get_named.task_id = sched_named_task(in.get_named.name);
            break;
        }
        case SCHED_SPAWN: {
            uint64_t root_id = in.spawn.root_id;
            kcomm_t *sin, *sout;
            uint64_t task_id = sched_task_create(root_id, &sin, &sout);

            status.spawn.root_id = root_id;
            status.spawn.task_id = task_id;

            queue[queue_size].task_id = task_id;
            queue[queue_size].in = sin;
            queue[queue_size].out = sout;
            queue_size ++;

            break;
        }
        case SCHED_SET_STATE: {
            sched_set_state(in.set_state.task_id, in.set_state.index,
                in.set_state.value);
            break;
        }
        case SCHED_REAP: {
            uint64_t id = in.reap.task_id;
            if(id == 0) {
                id = q->task_id;
                // definitely don't send status update if reaping self...
                status.req_id = 0;
            }
            sched_task_reap(id);
            remove_from_queue(id);
            break;
        }
        default:
            d_printf("Unknown sched_in packet type!\n");
            break;
        }

        if(status.req_id) kcomm_put(q->out, &status, sizeof(status));
    }

    return ret;
}

static void remove_from_queue(uint64_t id) {
    for(int i = 0; i < queue_size; i ++) {
        if(queue[i].task_id == id) {
            queue[i] = queue[queue_size-1];
            queue_size --;
            break;
        }
    }
}

void listen(task_state_t *hw_task) {
    queue_size = 0;

    {
        // add hw task to queue
        queue[queue_size].task_id = sched_task_attach(hw_task,
            &queue[queue_size].in, &queue[queue_size].out);
        queue_size ++;

        hw_task->state |= TASK_STATE_RUNNABLE;
    }

    d_printf("Total number of queue entries: %x\n", queue_size);
    while(1) {
        int any = 0;
        for(int i = 0; i < queue_size; i ++) {
            any |= process(queue + i);
        }
        if(!any) {
            d_printf("done with everything on incoming sched queues\n");
            __asm__ ("int $0xff");
        }
        // TODO: yield timeslice here
    }
}

void process_for(uint64_t task_id) {
    for(int i = 0; i < queue_size; i ++) {
        if(queue[i].task_id == task_id) {
            process(queue + i);
            break;
        }
    }
}
