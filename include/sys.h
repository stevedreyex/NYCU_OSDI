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

int sys_getpid();
size_t sys_read(char buf[], size_t size);
size_t sys_uartwrite(const char buf[], size_t size);		// 2
int sys_exec(const char *name, char *const argv[]);			// 3
int sys_fork();												// 4
void sys_exit(int status);									// 5
int sys_mbox_call(unsigned char ch, unsigned int *mbox);	// 6
void sys_kill(int pid);										// 7

// System call interface
int getpid();											// 0
size_t uartread(char buf[], size_t size);				// 1
size_t uartwrite(const char buf[], size_t size);		// 2
int exec(const char *name, char *const argv[]);			// 3
int fork();												// 4
void exit(int status);									// 5
int mbox_call(unsigned char ch, unsigned int *mbox);	// 6
void kill(int pid);										// 7

#endif
#endif
