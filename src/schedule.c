#include "schedule.h"
#include "entry.h"
#include "printf.h"
#include "fork.h"
#include "mm.h"

static struct task_struct init_task = INIT_TASK;
struct task_struct *curr = &(init_task);
struct task_struct *task[NR_TASKS] = {&(init_task),};
int nr_tasks = 1;

void preempt_disable(){
	curr->preempt_count++;
}

void preempt_enable(){
	curr->preempt_count--;
}

void schedule_tail(){
	preempt_enable();
}

void schedule(){
	curr->counter = 0;
	_schedule();
}

void _schedule(){
	printf("[schedule] Schedule activated:\n");
	preempt_disable();
	// printf("Now Scheduling\n");
	int next, c;
	struct task_struct *p;
	while(1){
		c = -1;
		next = 0;
		// Find task to switch to
		for(int i = 0; i < NR_TASKS; i++){
			p = task[i];
			if(!p) continue;
			// Choose the task with bigger counter
			printf("p at: %x, %d > %d\n", p, p->counter, c);
			if (p && p->state == TASK_RUNNING && p->counter > c) {
				c = p->counter;
				next = i;
			}
		}
		
		// some p->counter is not NULL
		if(c) break;
		// all	p->counter are NULL
		// do the increment
		for(int i = 0; i < NR_TASKS; i++){
			p = task[i];
			// If e.g. some process not been choosed in this round
			// Then its counter increases.
			// A process with higher priority may increase more
			// With old counter value aging
			// The process with the largest counter value will be choosed next round
			// At the time the process is chosen, the counter value turns to 0;
			if(p) p->counter = (p->counter >> 1) + p->priority;
		}
	}
	switch_to(task[next], next);
	preempt_enable();
}

void switch_to(struct task_struct * next, int index){
	if(curr == next) return;
	struct task_struct * prev = curr;
	curr = next;
	printf("Switch from %d to %d!\n", prev->pid, next->pid);
	cpu_switch_to(prev, next);
}

void exit_process(){
	preempt_disable();
	curr->state = TASK_ZOMBIE;
	dump_task_state();
	// Free the process stack
	if(curr->stack){
		kfree((void *)curr->stack);
	}
	preempt_enable();
	schedule();
}


void dump_task_state() {
	printf("--------TSK--------\n");
 	for (int i = 0;i < nr_tasks;i++) {
 		printf("Task %d, %d: ", i, task[i]->state);

 		switch(task[i]->state) {
 		case TASK_RUNNING:
 			printf("RUNNING");
 			break;
 		case TASK_WAITING:
 			printf("WAITING");
 			break;
 		case TASK_ZOMBIE: 
 			printf("ZOMBIE");
 			break;
 		default:
 			printf("Unknown State");
 		}
 		printf("\n");
		struct pt_regs * temp = (struct pt_regs *)task[i]->stack;
		// printf("user/kernel\tcounter\tpriority\tpreempt\t|| stack:\tsp\tpc\tpstate\n");
		// printf("%d\t\t%d\t%d\t\t%d\t\t\t%d\t%d\t%d\n", task[i]->flags, task[i]->counter, task[i]->priority, task[i]->preempt_count, temp->sp, temp->pc, temp->pstate);
		// printf("fp\tlr\tsp\n");
		// printf("%x\t%x\t%x\n", task[i]->cpu_context.fp, task[i]->cpu_context.lr, task[i]->cpu_context.sp);
 	} 
	printf("--------END--------\n");
 }
