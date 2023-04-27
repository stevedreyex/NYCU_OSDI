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
#include "mailbox.h"

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

// 0
void user_proc_test_syscall_getPid(){
     /* Test syscall getPID */
    printf("[user_getPID] Test syscall getpid\n");
    int current_pid = getpid();
	printf("[user_getPID] Current Pid = %d\n", current_pid);
    delay(1000000);
	schedule();
	exit_process();
}

// 2
void user_proc_test_syscall_uart_write(){
	/* Test syscall uart_write*/
	int size = 0;
    printf("[uart_write] Test syscall uart_write\n");
    size = uartwrite("[uart_write] syscall test\n", 26);
    printf("[uart_write] How many byte written = %d\n", size);
    delay(2000000);
	schedule();
	exit_process();
}

// 1
void user_proc_test_syscall_uart_read(){
	int size;
	printf("[uart_read] Test syscall uart_read\n");
    char uart_read_buf[20];
    printf("[uart_read] Enter your input: ");
    size = uartread(uart_read_buf, 4);
    printf("\n[uart_read] Read buf = %s, How many byte read = %d\n", uart_read_buf, size);
    delay(2000000);
	schedule();
	exit_process();
}

// 3
void user_proc_test_syscall_args(){
	/* Test syscall exec with argument passing */
    char* argv[] = {"argv_test", "-o", "arg2", 0};
    exec("syscall.img", argv);
	// printf("[exit] Task%d exit\n", call_sys_getPID());
 	// call_sys_exit();
	delay(2000000);
	schedule();
	exit_process();

}

// 4
void user_proc_test_syscall_fork(){
	printf("\nFork Test, pid %d\n", get_pid());
    int cnt = 1;
    int ret = 0;
    if ((ret = fork()) == 0) { // child
        long long cur_sp;
        asm volatile("mov %0, sp" : "=r"(cur_sp));
        printf("first child pid: %d, cnt: %d, ptr: %x, sp : %x\n", get_pid(), cnt, &cnt, cur_sp);
        ++cnt;

        if ((ret = fork()) != 0){
            asm volatile("mov %0, sp" : "=r"(cur_sp));
            printf("first child pid: %d, cnt: %d, ptr: %x, sp : %x\n", get_pid(), cnt, &cnt, cur_sp);
        }
        else{
            while (cnt < 5) {
                asm volatile("mov %0, sp" : "=r"(cur_sp));
                printf("second child pid: %d, cnt: %d, ptr: %x, sp : %x\n", get_pid(), cnt, &cnt, cur_sp);
                delay(1000000);
                ++cnt;
            }
        }
        exit();
    }
    else {
        printf("parent here, pid %d, child %d\n", get_pid(), ret);
    }
}

// 6: mbox
void user_proc_test_syscall_mailbox(){
	volatile unsigned int  __attribute__((aligned(16))) mbox[36];
	// get the board's unique serial number with a mailbox call
    mbox[0] = 7*4;                  // length of the message
    mbox[1] = MBOX_REQUEST;         // this is a request message
    
    mbox[2] = GET_BOARD_REVISION;   // get serial number command
    mbox[3] = 4;                    // buffer size
    mbox[4] = TAG_REQUEST_CODE;
    mbox[5] = 0;                    // clear output buffer
    mbox[6] = END_TAG;

	mbox_call(MBOX_CH_PROP, mbox);
	printf("[mailbox] the board's revision is %x\n", mbox[5]);
	delay(2000000);
	schedule();
	exit_process();
}

// 7: kill

/*
 * End of a bunch of tests
 */


void kernel_proc(){
	printf("[kernel_proc] Kernel Process at EL%d\n", get_el());
	// int err = move_to_user_mode((unsigned long)&user_proc_test_syscall_getPid);			// 0
	// int err = move_to_user_mode((unsigned long)&user_proc_test_syscall_uart_read);		// 1
	// int err = move_to_user_mode((unsigned long)&user_proc_test_syscall_uart_write);		// 2
	int err = move_to_user_mode((unsigned long)&user_proc_test_syscall_args);			// 3
	// int err = move_to_user_mode((unsigned long)&user_proc_test_syscall_fork);				// 4
	// int err = move_to_user_mode((unsigned long)&user_proc_test_syscall_mailbox);			// 6																						
	if(err < 0) printf("Error with handle kernel process\n");
	printf("[kernel_proc] End of function\n");
	schedule();
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
