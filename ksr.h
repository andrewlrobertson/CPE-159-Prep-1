// ksr.h, 159

#ifndef _KSR_
#define _KSR_

#include "const-type.h"

void SpawnSR(func_p_t p);
void TimerSR(void);
void SysSleep(void);
void SysWrite(void);
void SyscallSR(void);
void SysSetCursor(void);
void SysFork(void);
void SysUnlockMutex(void);

#endif
