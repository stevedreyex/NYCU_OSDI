#include "sys.h"

.global call_sys_getPID
call_sys_getPID:
	mov w8, SYS_GETPID
	svc #0
	ret

