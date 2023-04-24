#include "sys.h"
#include "printf.h"
#include "mm.h"
#include "cpio.h"
#include "schedule.h"
#include "uart.h"
#include "fork.h"
#include "string.h"
#include "timer.h"

int sys_getPID(){
	return curr->pid;
}

void * const sys_call_table[] = 
     {sys_getPID};
