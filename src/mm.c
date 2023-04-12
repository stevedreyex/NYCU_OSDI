#include "mm.h"
#include "types.h"
#include "mini_uart.h"
// Dynamic allocation starts at line 'Start Dynamic allocation'
page_t pageframe[PAGE_FRAME_NUM];
free_area_t free_area[MAX_ORDER+1];
object_allocator_t obj_alloc_pool[MAX_DYNAMIC_ALLOC_NUM];

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
	uart_puts("\nAlloc start: Try to alloc a block in order ");
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

// Start of Dynamic allocation

void obj_alloc_init(object_allocator_t * obj_alloc, int objsize){
	INIT_LIST_HEAD(&obj_alloc->empty);
	INIT_LIST_HEAD(&obj_alloc->partial);
	INIT_LIST_HEAD(&obj_alloc->full);
	obj_alloc->curr = NULL;
	obj_alloc->objsize = objsize;
	obj_alloc->obj_per_page = PAGE_SIZE / objsize;
	obj_alloc->obj_used = 0;
	obj_alloc->page_used = 0;
}

void obj_page_init(page_t *page){
	page->obj_used = 0;
	page->free =  NULL;
}

int obj_alloc_reg(int objsize){
	// Find a Dynamic allocator that has not been used
	for(int i = 0; i< MAX_DYNAMIC_ALLOC_NUM; i++){
		if(obj_alloc_pool[i].objsize != 0)
			continue;
		else{
			obj_alloc_init(&obj_alloc_pool[i], objsize);
			uart_puts("Register a object at pool! At object ");
			uart_int(i);
			return i;
		}
	}
	// Iterate through the dynamic alloc pool turn out can't find any available allocator
	return -1;
}

void * obj_allocate(int token){
	object_allocator_t * obj_alloc = &obj_alloc_pool[token];
	void * alloc_addr = NULL;

	// This pool object didn't have a page yet
	if (obj_alloc->curr == NULL) {
		page_t * page;
		// Partial is exist then use the partial one
		if (!list_empty(&obj_alloc->partial)){
			page = (page_t *) obj_alloc->partial.next;
			list_del(&page->list);
		} 
		// Use a empty page
		else if (!list_empty(&obj_alloc->empty)){
			page = (page_t *) obj_alloc->empty.next;
			list_del(&page->list);
		} 
		// Get a new page from the buddy system
		else {
			page = buddy_alloc(0);
			obj_page_init(page);
			// Is a page with Dynamic allocation, shall store this object
			page->object_alloc = obj_alloc;
			obj_alloc->page_used += 1;
		}

		obj_alloc->curr = page;
	}

	struct list_head * obj_freelist = obj_alloc->curr->free;
	// obj_page_init was first called, which in the previous case 3
	if(obj_freelist != NULL){
		alloc_addr = obj_freelist;
		obj_freelist = obj_freelist->next;
	} else {
		// page base address add (num of objs) * (size per object)
		alloc_addr = (void *) obj_alloc->curr->phy_addr + obj_alloc->curr->obj_used * obj_alloc->objsize;
	}
	obj_alloc->obj_used += 1;
	obj_alloc->curr->obj_used += 1;

	// Page might full after this obj is added, so check it
	// Compare <obj_allocator:number for a page can store> with <Number of items already stored in a page>
	if(obj_alloc->obj_per_page == obj_alloc->curr->obj_used){
		list_add_tail(&obj_alloc->curr->list, &obj_alloc->full);
		// Is now full, need a new current page next time
		obj_alloc->curr = NULL;
	}

	uart_puts("\nAllocated Successfully, the allocated address is at: ");
	uart_hex(alloc_addr);
	dump_dyn_area(obj_alloc);
	return alloc_addr;
}

void obj_free(void * obj_addr){
	int obj_pfn = PHY_ADDR_TO_PFN(obj_addr);
	page_t * page = &pageframe[obj_pfn];
	object_allocator_t * obj_alloc = page->object_alloc;

	// Page's list shall always points to the first free object
	struct list_head * temp = page->free;
	page->free = (struct list_head *) obj_addr;
	page->free->next =  temp;

	obj_alloc->obj_used -= 1;
	page->obj_used -= 1;

	// Full page may reduce to partial
	if(obj_alloc->obj_per_page-1 == page->obj_used){
		list_del(&page->list);
		list_add_tail(&page->list, &obj_alloc->partial);
	}

	// Partial page may reduce to empty
	if(page->obj_used == 0 && obj_alloc->curr != page){
		list_del(&page->list);
		list_add_tail(&page->list, &obj_alloc->empty);
	}
	dump_dyn_area(obj_alloc);
}

void dyn_init(){
	uart_puts("\nDynamic Allocator init start:\n");
	int token = obj_alloc_reg(2000);
    void *addr1 = obj_allocate(token); // 0
    void *addr2  = obj_allocate(token);

    void *addr3 = obj_allocate(token); // 1
    void *addr4 = obj_allocate(token);

    void *addr5 = obj_allocate(token); // 2
    void *addr6 = obj_allocate(token);

    void *addr7 = obj_allocate(token); // 3
    void *addr11 = obj_allocate(token);

    void *addr12 = obj_allocate(token); // 4
    void *addr13 = obj_allocate(token);
    obj_free(addr1);
    obj_free(addr2);
    void *addr14 = obj_allocate(token); // 0
    void *addr15 = obj_allocate(token);

    void *addr16 = obj_allocate(token); // 5

    obj_free(addr11);
    obj_free(addr5);
    obj_free(addr15);
    obj_free(addr3);

    void *addr17 = obj_allocate(token); // 5
    void *addr18 = obj_allocate(token); // 3	
}

// Below are the area to dump imformations

void dump_free_area(){
	uart_puts("\n----------BUDDY----------");
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

void dump_dyn_area(object_allocator_t * obj_alloc){
	uart_puts("\n-----------DYN-----------\n");
	uart_puts("objsize: ");
	uart_int(obj_alloc->objsize);
	uart_puts("\nobj_per_page: ");
	uart_int(obj_alloc->obj_per_page);
	uart_puts("\nobj_used: ");
	uart_int(obj_alloc->obj_used);
	if(obj_alloc->curr != NULL){
		uart_puts("\nobj_alloc->curr->phy_addr: ");
		uart_hex(obj_alloc->curr->phy_addr);
		uart_puts("\nobj_alloc->curr->free: ");
		uart_hex(obj_alloc->curr->free);
		uart_puts("\nobj_alloc->curr->obj_used: ");
		uart_hex(obj_alloc->curr->obj_used);
		uart_puts("\nobj_alloc->curr->pfn: ");
		uart_int(obj_alloc->curr->pfn);
	} else {
		uart_puts("\ncurr is now NULL");
	}
	struct list_head * pos;
	// Dump full list
	uart_puts("\nFull list ");
	list_for_each(pos, (struct list_head *)&obj_alloc->full){
		uart_puts("-> [");
		uart_int(((struct page *)pos)->pfn);
		uart_puts("] ");
	}
	// Dump Partial list
	uart_puts("\nPartial list ");
	list_for_each(pos, (struct list_head *)&obj_alloc->partial){
		uart_puts("-> [");
		uart_int(((struct page *)pos)->pfn);
		uart_puts("] ");
	}
	// Dump Empty list
	uart_puts("\nEmpty list ");
	list_for_each(pos, (struct list_head *)&obj_alloc->empty){
		uart_puts("-> [");
		uart_int(((struct page *)pos)->pfn);
		uart_puts("] ");
	}
	uart_puts("\n-----------END-----------\n");
}

