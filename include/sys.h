#ifndef _SYS_H
#define _SYS_H

#include "types.h"

#define __NR_SYSCALLS 8

#define SYS_GETPID	0
#define SYS_READ	1
#define SYS_WRITE	2
#define SYS_EXEC	3
#define SYS_FORK	4
#define SYS_EXIT	5
#define SYS_MBOX	6
#define SYS_KILL	7
#ifndef __ASSEMBLER__

int sys_getPID();

int call_sys_getPID();

#endif
#endif
