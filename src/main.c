#include "uart.h"
#include "shell.h"
#include "printf.h"
#include "mm.h"
#include "timer.h"
#include "utils.h"
#include "entry.h"
#include "fork.h"
#include "schedule.h"
#include "sys.h"

void foo(){
     for(int i = 0; i < 10; ++i) {
         printf("Thread id: %d %d\n", curr->pid, i);
         delay(1000000);
         schedule();
     }

     exit_process();
}

/*
 * Start of a bunch of tests
 */


void user_proc_test_syscall_write(){
	
     /* Test syscall write */
 	// call_sys_write("[user_process]User process started\n");
}

void user_proc_test_syscall_getPid(){
     /* Test syscall getPID */
    printf("[user_getPID] Test syscall getpid\n");
    int current_pid = call_sys_getPID();
	printf("[user_getPID] Current Pid = %d\n", current_pid);
	exit_process();
}

void user_proc_test_syscall_uart_write(){
	/* Test syscall uart_write*/
    printf("[uart_write] Test syscall uart_write\n");
    // int size = call_sys_uart_write("[uart_write] syscall test\n", 26);
    // printf("[uart_write] How many byte written = %d\n", size);
}

void user_proc_test_syscall_uart_read(){
	printf("[uart_read] Test syscall uart_read\n");
    char uart_read_buf[20];
    printf("[uart_read] Enter your input: ");
    // size = call_sys_uart_read(uart_read_buf, 4);
    // printf("\n[uart_read] Read buf = %s, How many byte read = %d\n", uart_read_buf, size);
}

void user_proc_test_syscall_malloc(){
	/* Test syscall malloc */
    printf("[malloc] Test syscall malloc\n");
    // void *malloc_return = call_sys_malloc(PAGE_SIZE);
    // printf("[malloc] Allocated starting address of page = 0x%x\n", malloc_return);
}

void user_proc_test_syscall_args(){
	/* Test syscall exec with argument passing */
    char* argv[] = {"argv_test", "-o", "arg2", 0};
    // call_sys_exec(exec_argv_test, argv);
	// printf("[exit] Task%d exit\n", call_sys_getPID());
 	// call_sys_exit();
}

/*
 * End of a bunch of tests
 */


void kernel_proc(){
	printf("[kernel_proc] Kernel Process at EL%d\n", get_el());
	int err = move_to_user_mode((unsigned long)&user_proc_test_syscall_getPid);
	if(err < 0) printf("Erorr with handle kernel process\n");
	printf("[kernel_proc] End of Kernel\n");
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

/*
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
*/

	int res = copy_process(PF_KTHREAD, (unsigned long)&kernel_proc, 0, 0);
 	if (res < 0) {
 		printf("error while starting kernel process");
 		return 0;
 	}

 	while (1) {
 		printf("\nIn kernel main()\n\n");
		dump_task_state();
 		kill_zombies(); // reclaim threads marked as DEAD
 		schedule();
        delay(10000000);
     }

    return 0;
}

void kill_zombies(){
	//TODO
}
