#include "mm.h"
#include "schedule.h"
#include "fork.h"
#include "entry.h"
#include "exception.h"
#include "printf.h"
#include "types.h"

// Create PCB
int copy_process(unsigned long flags, unsigned long fn, unsigned long arg, unsigned long stack){
	printf("Dump all tasks before copy:\n");
	dump_task_state();
	preempt_disable();
	struct task_struct * p;
	p = (struct task_struct *)kmalloc(PAGE_SIZE);

	if(p == NULL){
		printf("[Allocated failed] Fail to kmalloc\n");
		return -1;
	}
	struct pt_regs * child_regs = task_pt_regs(p);
	memzero((unsigned long)child_regs, sizeof(struct pt_regs));
	memzero((unsigned long)&p->cpu_context, sizeof(struct cpu_context));

	if(flags & PF_KTHREAD){
		printf("Copy to Kernel Thread\n");
		// Clone a Kernel Thread
		// if function is wrapped by "move_to_user_mode", then fn is 0
		p->cpu_context.x19 = fn;
		p->cpu_context.x20 = arg;

		// Kernel process has no user stack
		p->stack = (unsigned long) NULL;
	} else {
		// printf("Copy to User Thread\n");
		// Clone a User Thread
		// Page Table and stack of child and stack put to the next Page
		// * curr is the source to fork, p is the new fork-to process
		// * curr_regs is the source pt_regs, and child_regs is the new stack pointer
		struct pt_regs * curr_regs = task_pt_regs(curr);
		*child_regs = *curr_regs;
		child_regs->regs[0] = 0; // Distinguish from parent
		child_regs->sp = stack + PAGE_SIZE;
		// user process fork has user stack
		// "fork" will pass stack = new [page] to reach stack
		p->stack = stack;
	}

	// Flags define the User/Kernel process
	// The priority is from the scheduler now
	// Preempt disable until init done
	p->flags = flags;
	p->priority = curr->priority;
	p->state =TASK_RUNNING;
	p->preempt_count = 1;
	// "ret_to_fork" will use x19 and x20
	// *** Here is the most critical part ***
	// Stack registers contains pc, and it points to the usr process
	p->cpu_context.lr = (unsigned long)ret_from_fork;
	p->cpu_context.sp = (unsigned long)child_regs;

	int pid = assignPID();
	// printf("Process %d is about to execute %x", pid, p->cpu_context.x19);
	task[pid] = p;
	task[pid]->pid = pid;

	preempt_enable();
	return pid;
}

int move_to_user_mode(unsigned long pc){
	printf("[move_to_user_mode] Now excute %x\n", pc);
	// move the current process from kernel mode to user mode
	// a kernel process has no user stack, so we have to initialize it.
	struct pt_regs * regs = task_pt_regs(curr);
	memzero(regs, sizeof(struct pt_regs));
	regs->pc = pc;
	regs->pstate = PSR_MODE_EL0t;
	unsigned long stack = (unsigned long)kmalloc(PAGE_SIZE);
	if (!stack) return -1;
	// allocate user mode stack
	regs->sp = stack + PAGE_SIZE;
	// current stack for user mode
	curr->stack = stack;
	return 0;
}

/*
 * Based pm the current execution, gives you another pair of registers
 */
struct pt_regs * task_pt_regs(struct task_struct * task){
	// The next thread size, a.k.a. the next page(size)
	unsigned long p = (unsigned long)task + THREAD_SIZE - sizeof(struct pt_regs);
	return (struct pt_regs *)p;
}

int assignPID(){
	for (long i = 0; i < NR_TASKS; i++){
		if (task[i] == NULL) {
			nr_tasks++;
			return i;
		}
	}
	return -1;
}
