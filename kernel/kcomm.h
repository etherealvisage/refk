#ifndef KCOMM_H
#define KCOMM_H

#include <stdint.h>

struct kcomm_t;
typedef struct kcomm_t kcomm_t;

void kcomm_init(kcomm_t *kc, uint64_t length);

int kcomm_put(kcomm_t *kc, void *data, uint64_t data_size);
int kcomm_get(kcomm_t *kc, void *data, uint64_t *data_size);

#endif
