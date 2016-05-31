#include "str.h"

uint64_t str_len(const char *s) {
    uint64_t ret = 0;
    while(*s) ret ++, s ++;
    return ret;
}

int str_cmp(const char *s1, const char *s2) {
    return str_ncmp(s1, s2, -1u);
}

int str_ncmp(const char *s1, const char *s2, uint64_t maxlen) {
    while(maxlen) {
        if(*s1 == 0 && *s2 == 0) return 0;
        if(*s1 < *s2) return -1;
        if(*s1 > *s2) return 1;
        s1 ++, s2 ++;
        maxlen --;
    }
    return 0;
}

int str_lto(char *s, uint64_t d) {
    if(d == 0) {
        s[0] = '0';
        s[1] = 0;
        return 0;
    }

    char *p = s;
    while(d) {
        *p = (d%10) + '0';
        p ++;
        d /= 10;
    }

    *p = 0;
    p --;
    char *q = s;
    while(q < p) {
        char t = *q;
        *q = *p;
        *p = t;
        q ++;
        p --;
    }

    return 0;
}
