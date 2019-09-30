
#ifndef _SYSCALL_
#define _SYSCALL_

int sys_get_pid(void);
int sys_get_time(void);
void sys_sleep(int sleep_sec);
void sys_write(char* str);
void sys_set_cursor(int row, int col);
int sys_fork(void);

#endif
