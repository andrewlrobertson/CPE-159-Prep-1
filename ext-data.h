// ext-data.h, 159
// kernel data are all declared in kernel.c during bootstrap
// .c code, if needed these, include this

#ifndef _EXT_DATA_
#define _EXT_DATA_

#include "kernel.c"

extern int run_pid;                 // PID of current selected running process
extern unsigned in sys_time_count;

#endif
