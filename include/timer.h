#ifndef _TIMER_H
#define _TIMER_H
#define MAX_TIMER 64

typedef struct timer{
	int used;
	int sec;
	char * prop;
	void (*fn)(void *);
} timer_t;

#endif
