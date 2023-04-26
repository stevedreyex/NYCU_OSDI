#ifndef _SCHEDULE_H
#define _SCHEDULE_H

#define THREAD_CPU_CONTEXT 0

#ifndef __ASSEMBLER__

#define THREAD_SIZE 4096
#define NR_TASKS 64

#define TASK_RUNNING 0
#define TASK_WAITING 1
#define TASK_ZOMBIE 2

#define PF_KTHREAD 0x00000002

// CPU context x19~x28, sp, fp and pc
// pid
// state, counter, priority, preempt_count, stack, flags
#define INIT_TASK \
{{0,0,0,0,0,0,0,0,0,0,0},0,0,0,1,0,0,PF_KTHREAD}

extern struct task_struct * curr;
extern struct task_struct * task[NR_TASKS];
extern int nr_tasks;

struct cpu_context {
	unsigned long x19;
	unsigned long x20;
	unsigned long x21;
	unsigned long x22;
	unsigned long x23;
	unsigned long x24;
	unsigned long x25;
	unsigned long x26;
	unsigned long x27;
	unsigned long x28;
	unsigned long fp;
	unsigned long lr;
	unsigned long sp;
};

/**
 * @stack:	For User
 * @flags:  1 For User/2 For kernel
 * @pstate: Exception Level Setup
 */
struct task_struct {
	struct cpu_context cpu_context;
	long pid;
	long state;
	long counter;
	long priority;
	long preempt_count;
	unsigned long stack;
	unsigned long flags;
};


extern void preempt_disable();
extern void preempt_enable();
extern void schedule_tail();
extern void cpu_switch_to(struct task_struct * prev, struct task_struct * next);
extern void switch_to(struct task_struct * next, int index);
extern void exit_process();
extern void dump_task_state();

#endif
#endif
