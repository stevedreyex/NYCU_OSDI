#include "printf.h"
#include "sys.h"
#include "uart.h"

int main(void) {
    init_printf(0, putc);
    
    printf("\n[fork_test]Fork Test, pid %d\n", getpid());
    int cnt = 1;
    int ret = 0;
    printf("pid: %d, cnt: %d, cnt_adress: 0x%x\n", getpid(), cnt, &cnt);
    if ((ret = fork()) == 0) { // child
        printf("pid: %d, cnt: %d, cnt_adress: 0x%x\n", getpid(), cnt, &cnt);
        ++cnt;
        fork();
        while (cnt < 5) {
            printf("pid: %d, cnt: %d, cnt_adress: 0x%x\n", getpid(), cnt, &cnt);
            for(int i = 0;i < 100000;i++);
            ++cnt;
        }
    } else {
        printf("parent here, pid %d, child %d\n", getpid(), ret);
    }
    // dumpTasksState();
    printf("[exit] Task%d\n", getpid());
    exit();
    return 0;
}