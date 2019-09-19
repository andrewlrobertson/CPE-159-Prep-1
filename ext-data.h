// ext-data.h, 159
// kernel data are all declared in kernel.c during bootstrap
// .c code, if needed these, include this

#ifndef _EXT_DATA_
#define _EXT_DATA_


#include "const-type.h"

extern int run_pid;                 // PID of current selected running process
extern unsigned int sys_time_count;
extern que_t avail_que;
extern que_t ready_que;
extern pcb_t pcb[PROC_MAX];
extern unsigned short *sys_cursor;  //like others, add the new sys_cursor newly declared in main.c
extern int sys_get_pid(void);
extern int sys_get_time(void);
extern void sys_sleep(int sleep_sec);
extern void sys_write(char* str);

#endif
