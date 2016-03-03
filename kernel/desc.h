#ifndef DESC_H
#define DESC_H

#define DESC_BASE 0xffffffffffc00000
#define DESC_GDT_ADDR (DESC_BASE)
#define DESC_IDT_ADDR (DESC_BASE + 0x1000)

void desc_init();

#endif
