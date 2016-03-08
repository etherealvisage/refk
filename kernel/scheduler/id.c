#include "id.h"

// other state
uint64_t next_id = 0x1000;

uint64_t gen_id() {
    return next_id ++;
}
