#include "timer.h"
#include "exc.h"
#include "types.h"
#include "printf.h"

timer_t timer_q[MAX_TIMER];

void timer_queue_initialize(){
	for(int i = 0; i < MAX_TIMER; i++){
		timer_q[i].used = 0;
		timer_q[i].sec = 0;
		timer_q[i].prop = NULL;
		timer_q[i].fn = NULL;
	}
}

void add_timer(char * prop, int sec, void (*func)(void *)){
	// disable_interrupt();
	int i;
	for(i = 0; i < MAX_TIMER; i++){
		if(timer_q[MAX_TIMER].used==1) continue;
		else {
			timer_q[i].used = 1;
			timer_q[i].sec = sec;
			timer_q[i].prop = prop;
			timer_q[i].fn = func;
			break;;
		}
	}
	// enable_interrupt();
	printf("timer added! with %s print in %d sec\n", timer_q[i].prop, timer_q[i].sec);
}
 void timer_decrement(){
	uart_puts("decreasing\n");
	for(int i = 0; i < MAX_TIMER; i++){
		if(timer_q[i].used == 1) timer_q[i].sec--;
		else continue;
		if(timer_q[i].sec <= 0) {
			timer_q[i].fn(timer_q[i].prop);
			timer_q[i].used = 0;
		}
	}
 }
