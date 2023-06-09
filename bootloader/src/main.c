#include "uart.h"
#include "shell.h"
#include "printf.h"
/* Initial Logo */
//   ___  ____  ____ ___   ____   ___ ____  _  __   __                 
//  / _ \/ ___||  _ \_ _| |___ \ / _ \___ \/ | \ \ / /   _ _ __   __ _ 
// | | | \___ \| | | | |    __) | | | |__) | |  \ V / | | | '_ \ / _` |
// | |_| |___) | |_| | |   / __/| |_| / __/| |   | || |_| | | | | (_| |
//  \___/|____/|____/___| |_____|\___/_____|_|   |_| \__,_|_| |_|\__, |
//                                                               |___/ 
char * init_logo = "\n\
  ___  ____  ____ ___   ____   ___ ____  _  __   __\n\
 / _ \\/ ___||  _ \\_ _| |___ \\ / _ \\___ \\/ | \\ \\ / /   _ _ __   __ _\n\
| | | \\___ \\| | | | |    __) | | | |__) | |  \\ V / | | | '_ \\ / _` |\n\
| |_| |___) | |_| | |   / __/| |_| / __/| |   | || |_| | | | | (_| |\n\
 \\___/|____/|____/___| |_____|\\___/_____|_|   |_| \\__,_|_| |_|\\__, |\n\
                                                              |___/\n\n";

int main()
{
    // set up serial console
    uart_init();

    init_printf(0, putc);
    printf("%s", init_logo); // checking if printf() work

    shell_start();

    return 0;
}
