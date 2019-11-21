// proc.h, 159

#ifndef _PROC_
#define _PROC_

void Idle(void);
void Init(void);
void MyChildExitHandler(void);
void ShellDir(void);
void ShellRoll(void);
void ShellCal(void);
void VforkOutput(int exit_code, int exit_pid);
void Shell(void);
void Login(void);

#endif
