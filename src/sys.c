// System call implementation
#include "sys.h"
#include "printf.h"
#include "mm.h"
#include "cpio.h"
#include "schedule.h"
#include "uart.h"
#include "fork.h"
#include "string.h"
#include "timer.h"
#include "printf.h"
#include <cpio.h>

int sys_getpid(){
	return curr->pid;
}

size_t sys_uartread(char buf[], size_t size){
	for (int i = 0;i < size;i++) {
         buf[i] = uart_getc();
     }
     buf[size] = '\0';

     return size;
}

size_t sys_uartwrite(const char buf[], size_t size){
	for (int i = 0;i < size;i++) {
        uart_send(buf[i]);
    }

    return size;	
}

int sys_exec(const char *name, char *const argv[]){
	preempt_disable();

	// find the program name, load it into memory.
	unsigned long * filesize;
	char * prog_addr = (char *)cpio_get_file((void *)INITRAMFS_ADDR, name, filesize);
	unsigned long required = (unsigned long)(*filesize);
	printf("[sys_exec] At %x, size %x\n", prog_addr, required);

	char * target_address = (char *)kmalloc(required);
	//for (int i = 0; i < required; i++) {
	// 	*(target_address + i) = *(prog_addr + i);
	//}
	memcpy(target_address, prog_addr, required);

	// Count the number of args
	int argc_count = 0;
	while(argv[argc_count] != 0){
		argc_count++;
	}
	printf("[sys_exec] With %d args\n", argc_count);

	// Reset sp to user stack, and move the args to x0
	struct pt_regs *regs = task_pt_regs(curr);
	char **backup = kmalloc(PAGE_SIZE);
	for(int i = 0; i <  argc_count; i++){
		*(backup + i) = argv[i];
	}
	regs->sp = curr->stack + PAGE_SIZE;
	regs->sp = regs->sp - ((argc_count + argc_count % 2) * 8);
	// Write to the actual stack pointer with aligned address space
	char **temp = (char**)regs->sp;
	for(int i = 0; i < argc_count; i++){
		*(temp + i) = *(backup + i);
	}

	// set pc/elr_elx to new function, and starting address of argv[]
	regs->pc = (unsigned long)target_address;
	regs->regs[1] = (unsigned long)regs->sp;

	preempt_enable();
	return argc_count;

}

int sys_fork(){
	preempt_disable();

	unsigned long user_stack = (unsigned long) (PAGE_SIZE);
	int pid = copy_process(0, 0, 0, user_stack);
	
	// fully copy user stack
	memcpy(task[pid]->stack, curr->stack, PAGE_SIZE);
	

	// set user stack to new child process
	// child stack should have the same offset as parent
	struct pt_regs * curr_regs = task_pt_regs(curr);
	int copied_sp_off = curr_regs->sp - curr->stack;
	struct pt_regs * child_regs = task_pt_regs(task[pid]);
	child_regs->sp = task[pid]->stack + copied_sp_off;

	preempt_enable();
	return pid;
}												// 4

void sys_exit(int status){
	exit_process();
}									// 5

int sys_mbox_call(unsigned char ch, unsigned int *mbox){
	return mailbox_call(ch, mbox);
}	// 6

void sys_kill(int pid){
	if (curr->pid == pid){
		printf("[sys_kill] You can't suicide! Please just exit first!");
		return;
	}
	else {
		task[pid] = NULL;
		nr_tasks--;
	}
}										// 7


void * const sys_call_table[] = 
     {sys_getpid, sys_uartread, sys_uartwrite, sys_exec, sys_fork, sys_exit, sys_mbox_call, sys_kill};
