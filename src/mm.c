#include "mm.h"
#include "mini_uart.h"

page_t pageframe[PAGE_FRAME_NUM];
free_area_t free_area[MAX_ORDER+1];

void page_init(){
	for(int i = 0; i < PAGE_FRAME_NUM; i++){
		// -1 means unallocated
		pageframe[i].order = -1;
		pageframe[i].pfn = i;
		pageframe[i].used = Free;
		pageframe[i].phy_addr = BUDDY_LO + PAGE_SIZE * i;
	}
}

void free_area_init(){
	// Made all page into a node of linked list
	for(int i = 0; i <= MAX_ORDER; i++){
		INIT_LIST_HEAD(&free_area[i].free_list);	
	}

	// Add the top order to the freelist and wait for alloc. Pages will split when alloc is needed.
	push2free(&pageframe[0], &free_area[MAX_ORDER], MAX_ORDER);
}

/**
 *  For Splitted blocks, if unused, then add to freelist to preserve the space
 */
void push2free(page_t *page, free_area_t *free_area, int order){
	list_add_tail(&page->list, &free_area->free_list);
	page->order = order;
	page->used = Free;
	free_area->map += 1;
}

/**
 *  Two Scenarios:
 *  1. If alloc direcly find list from freelist, remove from it.
 *	2. If the memory block can merge to a bigger one, remove and merge.
 */
void pop_free(page_t *page, free_area_t * freearea){
	list_del(&page->list);
	page->used = Taken;
	free_area->map -=1;
}

struct page * buddy_alloc(int order){
	uart_puts("Alloc start: Try to alloc a block in order ");
	uart_int(order);

	for(int i = order; i<=MAX_ORDER; i++){
		// Find if corresponding order block exists, else find higher order block then spilt
		uart_puts("\nCurrent Find Block Order:");
		uart_int(i);

		if(list_empty(&free_area[i].free_list)){
			uart_puts("\nNo free block in this order, split upper level order: ");
			uart_int(i+1);
			continue;
		}
		// Find Current or higher level block to alloc
		struct page * target = (struct page *) free_area[i].free_list.next;
		pop_free(target, &free_area[i]);
		target->order = order;
		
		for(int j = i; j > order; j--){
			int downward_buddy_pfn = FIND_BUDDY_PFN(target->pfn, j-1);
			uart_puts("\nDownward buddy pfn: ");
			uart_int(downward_buddy_pfn);
			struct page * downward_buddy = &pageframe[downward_buddy_pfn];
			push2free(downward_buddy, &free_area[j-1], j-1);
			uart_puts("\nSplitted into 2 chunks in order: ");
			uart_int(j-1);
		}

		uart_puts("\nFinally puts chunk in memory:");
		uart_hex(target->phy_addr);
		dump_free_area();
		return target;
	}
}

void buddy_free(struct page * page){
	uart_puts("\nFree Start: \n");
	int buddy_pfn =  FIND_BUDDY_PFN(page->pfn, page->order);
	page_t * buddy = &pageframe[buddy_pfn];
	while(page->order < MAX_ORDER && page->order == buddy->order && buddy->used == Free){
		uart_puts("\nCan merge to bigger block in order: ");
		uart_int(page->order + 1);
		uart_puts("\nPage pfn: ");
		uart_hex(page->pfn);
		uart_puts("\nBuddy pfn: ");
		uart_hex(buddy_pfn);
		
		// Free from freelist and coalesce
		pop_free(buddy, &free_area[buddy->order]);
		page = &pageframe[FIND_LBUDDY_PFN(page->pfn, page->order)];
		page->order += 1;


		buddy_pfn = FIND_BUDDY_PFN(page->pfn, page->order);
		buddy = &pageframe[buddy_pfn];
	}

	push2free(page, &free_area[page->order], page->order);

	uart_puts("\nFree Done.\n");
	dump_free_area();
}

void buddy_init(){
	page_init();
	free_area_init();
	dump_free_area();

	int allocate_test1[] = {1, 2, 1, 1, 3};
    int test1_size = sizeof(allocate_test1) / sizeof(int);
    page_t *(one_pages[test1_size]);
    for (int i = 0;i < test1_size;i++) {
        page_t *one_page = buddy_alloc(allocate_test1[i]); // Allocate one page frame
        //printf("\n Allocated Block{ pfn(%d), order(%d), phy_addr_16(0x%x) }: %u\n", one_page->pfn, one_page->order, one_page->phy_addr);
        one_pages[i] = one_page;
    }
    buddy_free(one_pages[2]);
    buddy_free(one_pages[1]);
    buddy_free(one_pages[4]);
    buddy_free(one_pages[3]);
    buddy_free(one_pages[0]);
}

void dump_free_area(){
	uart_puts("\n----------DEBUG----------");
	for(int i = MAX_ORDER; i >= 0; i--){
		uart_puts("\nOrder:");
		uart_int(i);
		struct list_head *pos;
		list_for_each(pos, (struct list_head *) &free_area[i].free_list) {
			 uart_puts("-> [pfn: ");
			 uart_hex(((struct page *)pos)->pfn);
			 uart_puts(" phy_addr: ");
			 uart_hex(((struct page *)pos)->phy_addr);
			 uart_puts("] ");
         }
	}
	uart_puts("\n-----------END-----------\n");
		
}
