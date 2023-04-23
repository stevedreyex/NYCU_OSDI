#include "uart.h"
#include "shell.h"
#include "printf.h"
#include "mm.h"
#include "timer.h"
#include "utils.h"
#include "entry.h"
#include "fork.h"
#include "schedule.h"

void foo(){
     for(int i = 0; i < 10; ++i) {
         printf("Thread id: %d %d\n", curr->pid, i);
         delay(1000000);
         schedule();
     }

     exit_process();
 }

int main()
{
    // set up serial console
    uart_init();
    
    init_printf(0, putc);

    // Initialize memory allcoator
    mm_init();

    // Initialize timer list for timeout events
    timer_list_init();

	// IRQ
	enable_irq();
	printf("IRQ enabled\n");

    // start shell
    // shell_start();
	for(int i = 0; i < 2; ++i) { // N should bigger than 2
		int res = copy_process(PF_KTHREAD, (unsigned long)&foo, 0, 0);
 		if (res < 0) {
 			printf("error while starting kernel process");
 			return 0;
 		}
		printf("Thread %d successfully created!", i);
     }

 	while (1) {
 		printf("\nIn kernel main()\n\n");
 		// kill_zombies(); // reclaim threads marked as DEAD
 		schedule();
        delay(10000000);
     }

    return 0;
}
