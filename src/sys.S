#include "sys.h"

// The w8 save the system call number
// The arguments is at x0, x1, ..., they will be preserved
// even if we enter the critical section
// the args are saved and won't be replaced unless enter err_hang

.global getpid
getpid:
	mov w8, SYS_GETPID
	svc #0
	ret

.global uartread
uartread:
	mov w8, SYS_READ
	svc #0
	ret

.global uartwrite
uartwrite:
	mov w8, SYS_WRITE
	svc #0
	ret

.global exec
exec:
	mov w8, SYS_EXEC
	svc #0
	ret

.global fork
fork:
	mov w8, SYS_FORK
	svc #0
	ret

.global exit
exit:
	mov w8, SYS_EXIT
	svc #0
	ret

.global mbox_call
mbox_call:
	mov w8, SYS_MBOX
	svc #0
	ret

.global kill
kill:
	mov w8, SYS_KILL
	svc #0
	ret
