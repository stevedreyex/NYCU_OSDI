#ifndef _MM_H
#define _MM_H

#include "list.h"
#include "types.h"

// refrence: https://www.kernel.org/doc/gorman/html/understand/understand009.html
#define PAGE_SIZE (1 << 12) //4096
#define MAX_ORDER 5 // Levels of Order
#define TOTAL_MAX_SIZE (1 << MAX_ORDER) // Total size of buddy system

#define PAGE_FRAME_NUM 32
#define BUDDY_LO 0x10000000
#define BUDDY_HI (BUDDY_LO + MAX_SIZE * PAGE_SIZE)

#define FIND_BUDDY_PFN(pfn, order) ((pfn) ^ (1<<(order)))
#define FIND_LBUDDY_PFN(pfn, order)((pfn) & (~(1<<(order))))

enum page_status {
	Free,
	Taken
};

typedef struct free_area_struct {
	struct list_head free_list;
	unsigned long map;
 } free_area_t;

typedef struct page {
     struct list_head list;

    int order;
    int pfn; // page frame number
    int used;
	uint64_t phy_addr;
 } page_t;


void page_init();
void free_area_init();
void push2free(page_t *page, free_area_t *free_area, int order);
void pop_free(page_t *page, free_area_t * freearea);
void buddy_free(struct page * page);
void buddy_init();

#endif
