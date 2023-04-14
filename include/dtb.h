#include <stdint.h>

#ifndef _DTB_H
#define _DTB_H

uint32_t bswap_32(uint32_t num);
uint64_t bswap_64(uint64_t num);

void set_dtb(void * addr);
void dtb_list();
int find_dtb(void *dtb, const char* name, int name_len, int (*func)(void*, int));

#endif
