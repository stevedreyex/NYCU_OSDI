#include "mm.h"
#include "schedule.h"
#include "fork.h"
#include "entry.h"
#include "exception.h"
#include "printf.h"
#include "types.h"

int copy_process(unsigned long flags, unsigned long fn, unsigned long arg, unsigned long stack){
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
		// Clone a Kernel Thread
		p->cpu_context.x19 = fn;
		p->cpu_context.x20 = arg;
	} else {
		// Clone a User Thread
	}

	// Flags define the User/Kernel process
	// The priority is from the scheduler now
	// Preempt disable until init done
	p->flags = flags;
	p->priority = curr->priority;
	p->preempt_count = 1;
	// init done
	printf("Fork to %x", p->cpu_context.x19);
	p->cpu_context.lr = (unsigned long)ret_from_fork;
	p->cpu_context.sp = (unsigned long)p + THREAD_SIZE;

	int pid = nr_tasks++;
	task[pid] = p;
	task[pid]->pid = pid;

	preempt_enable();
	return pid;
}

struct pt_regs * task_pt_regs(struct task_struct * task){
	// The next thread size, a.k.a. the next page(size)
	unsigned long p = (unsigned long)task + THREAD_SIZE - sizeof(struct pt_regs);
	return (struct pt_regs *)p;
}
