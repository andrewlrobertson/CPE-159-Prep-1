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
void SysLockMutex(void);
void SysExit(void);
void SysWait(void);
void AlterStack(int pid, func_p_t p);
void SysKill(void);
void SysSignal(void);
void KBSR(void);
void SysRead(void);
void SysVfork(void);
void TTYdspSR(void);
#endif
