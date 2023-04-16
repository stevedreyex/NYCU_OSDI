#include "cpio.h"
#include "dtb.h"
#include "mini_uart.h"
#include "mm.h"
#include "peripherals/mailbox.h"
#include "utils.h"
#define MAX_CMD 64

#define ARR_SIZE(ARR) (sizeof(ARR) / sizeof(ARR[0]))
typedef void (*handler_t)();

void hello_service() { uart_puts("Hello World!\n"); }

void reboot_service() { reset(10); }

void mailbox_service() { mbox_call(MBOX_CH_PROP); }

void ls_service() { initrd_list(); }

void cat_service() { cat_list(); }

void lshw_service() { dtb_list(); }

void initramfs_service() { print_initramfs(); }

void prog_service() { exec_prog(load_prog("usr.img")); }

void async_service() {
  uart_puts("Asynchronous IO Starts, Please Type:");
  uart_puts("\n$ ");

  // testing, if success interrupt, then this will halt
  char cmd[MAX_CMD];
  buf_clear(cmd, MAX_CMD);
  enable_mini_uart_interrupt();
  uart_async_getc(cmd, 16);
  delay(1);
  uart_async_send(cmd, 16);
  return 1;
}

void buddy_service() { buddy_init(); }

void dyn_service() { dyn_init(); }

void simp_malloc_service() { simple_malloc_demo(); }

void help_service() {
  uart_puts("help:\t\tprint this help menu\n");
  uart_puts("hello:\t\tprint Hello World!\n");
  uart_puts("mailbox:\tMailbox address and size\n");
  uart_puts("ls:\t\tshow the directory\n");
  uart_puts("cat:\t\ttshow file content\n");
  uart_puts("lshw:\t\tshow hardware resuorces\n");
  uart_puts("reboot:\t\treboot the device\n");
  uart_puts("prog:\t\trun a user program\n");
  uart_puts("async_io:\tStarts a Read by Interrupt\n");
  uart_puts("buddy:\t\tDemo Buddy System[1, 2, 1, 1, 3]\n");
  uart_puts("dyn:\t\tDemo Dynamic Allocator (size: 2000)\n");
  uart_puts("simp_malloc:\tsimple malloc on heap\n");
  uart_puts("timer:\tenable core timer\n");
  uart_puts("test:\ttest functionality slot\n");
}

void enable_timer_service() {
  core_timer_enable();
  uart_puts("Core Timer is now enabled!\n");
}

void test_service() { asm volatile("svc 1"); }

unsigned int parse_cmd(char *cmd) {
  struct {
    char *name;
    handler_t handler;
  } commands[] = {{.name = "help", .handler = help_service},
                  {.name = "hello", .handler = hello_service},
                  {.name = "reboot", .handler = reboot_service},
                  {.name = "mailbox", .handler = mailbox_service},
                  {.name = "ls", .handler = ls_service},
                  {.name = "cat", .handler = cat_service},
                  {.name = "lshw", .handler = lshw_service},
                  {.name = "initramfs", .handler = initramfs_service},
                  {.name = "prog", .handler = prog_service},
                  {.name = "async", .handler = async_service},
                  {.name = "buddy", .handler = buddy_service},
                  {.name = "dyn", .handler = dyn_service},
                  {.name = "simp_malloc", .handler = simp_malloc_service},
                  {.name = "timer", .handler = enable_timer_service},
                  {.name = "test", .handler = test_service}};
  for (int i = 0; i < ARR_SIZE(commands); i++) {
    if (str_comp(cmd, commands[i].name)) {
      commands[i].handler();
      break;
    }
  }
  buf_clear(cmd, MAX_CMD);
}

void shell_input(char *cmd) {
  int idx = 0;
  char c;
  while ((c = uart_getc()) != '\n') {
    /*TODO BS*/
    // if(c == 8 && idx > 0){
    //     cmd[--idx] = '\0';
    //     uart_send('\b');
    //     uart_send('\b');
    // }
    // else {
    uart_send(c);
    cmd[idx] = c;
    idx++;
    // }
  }
  uart_puts("\n");
}

int str_comp(char *x, char *y) {
  for (int i = 0; x[i] != '\0'; i++) {
    if (x[i] != y[i])
      return 0;
  }
  return 1;
}

void exception_entry() {
  unsigned long spsrel1, elrel1, esrel1;
  asm volatile("mrs %0, SPSR_EL1" : "=r"(spsrel1));
  uart_puts("SPSR_EL1: 0x");
  uart_hexlong(spsrel1);
  uart_puts("\n");
  asm volatile("mrs %0, ELR_EL1" : "=r"(elrel1));
  uart_puts("ELR_EL1: 0x");
  uart_hexlong(elrel1);
  uart_puts("\n");
  asm volatile("mrs %0, ESR_EL1" : "=r"(esrel1));
  uart_puts("ESR_EL1: 0x");
  uart_hexlong(esrel1);
  uart_puts("\n");
}

void print_core_timer(unsigned long frq, unsigned long cnt) {
  uart_puts("Core timer: ");
  uart_ulong(cnt / frq);
  uart_puts("\n");
}

void exec_prog(char *addr) {
  uart_hex(addr);
  uart_puts("\n");
  // Holds the saved process state when an exception is taken to EL0
  asm volatile("mov x0, 0");
  asm volatile("msr spsr_el1, x0");
  // Set the user program start address
  asm volatile("msr elr_el1, %0" : : "r"(addr));
  // Set user prog stack at
  asm volatile("mov x0, 0x60000");
  asm volatile("msr sp_el0, x0");
  asm volatile("eret");
}
