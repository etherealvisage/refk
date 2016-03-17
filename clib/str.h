#ifndef CLIB_STR_H
#define CLIB_STR_H

#include <stdint.h>

uint64_t str_len(const char *s);
int str_cmp(const char *s1, const char *s2);
int str_ncmp(const char *s1, const char *s2, uint64_t maxlen);

#endif
