#include "printf.h"
#include "sys.h"
#include "uart.h"

int main(void) {
    init_printf(0, putc);
    
    // printf("\n[fork_test]Fork Test, pid %d\n", getpid());
    uartwrite("\n[fork_test]Fork Test, pid", 27);

    int cnt = 1;
    int ret = 0;
    // printf("pid: %d, cnt: %d, cnt_adress: 0x%x\n", getpid(), cnt, &cnt);
    uartwrite("(Outer) pid: %d, cnt: %d, cnt_adress: 0x%x\n", 44);

    if ((ret = fork()) == 0) { // child
        uartwrite("(Inner 1) pid: %d, cnt: %d, cnt_adress: 0x%x\n", 46);
        ++cnt;
        fork();
        while (cnt < 5) {
            uartwrite("(Inner 2) pid: %d, cnt: %d, cnt_adress: 0x%x\n",46);
            for(int i = 0;i < 100000;i++);
                ++cnt;
        }
    } else {
        uartwrite("parent here, pid %d, child %d\n",31);
    }
    // dumpTasksState();
    uartwrite("[exit] Task%d\n", 15);
    exit();
    return 0;
}