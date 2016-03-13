#include "klib/kutil.h"
#include "klib/sheap.h"
#include "klib/task.h"

#include "synch.h"
#include "task.h"

synchobj_t *synch_find_from_phy(uint64_t phy) {
    return 0;
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
