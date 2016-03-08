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
static void process(queue_entry *q) {
    sched_in_packet_t in;
    sched_out_packet_t out;
    uint64_t in_size;
    while(!kcomm_get(q->in, &in, &in_size)) {
        d_printf("Received!\n");
        switch(in.type) {
        case SCHED_SET_NAME: {
            in.set_name.name[31] = 0;
            uint64_t id = in.set_name.task_id;
            if(id == 0) id = q->task_id;
            sched_set_name(id, in.set_name.name);
            break;
        }
        case SCHED_GET_NAMED: {
            in.get_named.name[31] = 0;
            out.type = SCHED_GET_NAMED;
            out.get_named.task_id = sched_named_task(in.get_named.name);
            kcomm_put(q->out, &out, sizeof(out));
            break;
        }
        case SCHED_SPAWN: {
            uint64_t root_id = in.spawn.root_id;
            kcomm_t *sin, *sout;
            uint64_t task_id = sched_task_create(root_id, &sin, &sout);

            out.type = SCHED_SPAWN;
            out.req_id = in.req_id;
            out.spawn.root_id = root_id;
            out.spawn.task_id = task_id;

            queue[queue_size].task_id = task_id;
            queue[queue_size].in = sin;
            queue[queue_size].out = sout;
            queue_size ++;

            kcomm_put(q->out, &out, sizeof(out));
            break;
        }
        case SCHED_SET_STATE: {
            sched_set_state(in.set_state.task_id, in.set_state.index,
                in.set_state.value);
            break;
        }
        case SCHED_REAP: {
            uint64_t id = in.reap.task_id;
            if(id == 0) id = q->task_id;
            sched_task_reap(id);
            remove_from_queue(id);
            break;
        }
        default:
            d_printf("Unknown sched_in packet type!\n");
            break;
        }
    }
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
        for(int i = 0; i < queue_size; i ++) {
            process(queue + i);
        }
        // TODO: yield timeslice here
    }
}
