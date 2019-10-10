// syscall.c
// system service calls for kernel services

#include "const-type.h"     // for SYS_GET_PID, etc., below

int sys_get_pid(void) {     // phase2
   int pid;

   asm("movl %1, %%eax;     // # for kernel to identify service
        int $128;           // interrupt!
        movl %%ebx, %0"     // after, copy ebx to return
       : "=g" (pid)         // output from asm()
       : "g" (SYS_GET_PID)  // input to asm()
       : "eax", "ebx"       // clobbered registers
   );

   return pid;
}

int sys_get_time(void) {     // similar to sys_get_pid
   int time;

   asm("movl %1, %%eax;     // # for kernel to identify service
        int $128;           // interrupt!
        movl %%ebx, %0"     // after, copy ebx to return
       : "=g" (time)         // output from asm()
       : "g" (SYS_GET_TIME)  // input to asm()
       : "eax", "ebx"       // clobbered registers
   );

   return time;
}

void sys_sleep(int sleep_sec) {  // phase2
   asm("movl %0, %%eax;          // # for kernel to identify service
        movl %1, %%ebx;          // sleep seconds
        int $128"                // interrupt!
       :                         // no output from asm()
       : "g" (SYS_SLEEP), "g" (sleep_sec)  // 2 inputs to asm()
       : "eax", "ebx"            // clobbered registers
   );
}

void sys_write(char* str) {             // similar to sys_sleep
   asm("movl %0, %%eax;          // # for kernel to identify service
        movl %1, %%ebx;          // string to write
        int $128"                // interrupt!
       :                         // no output from asm()
       : "g" (SYS_WRITE), "g" (str)  // 2 inputs to asm()
       : "eax", "ebx"            // clobbered registers
	);
}

void sys_set_cursor(int row, int col) {  // phase3
  asm("movl %0, %%eax;          // # for kernel to identify service
       movl %1, %%ebx;          // row
       movl %2, %%ecx;          // col
       int $128"                // interrupt!
      :                         // no output from asm()
      : "g" (SYS_SET_CURSOR), "g" (row), "g" (col)  // 3 inputs to asm()
      : "eax", "ebx", "ecx"            // clobbered registers
 );
}

int sys_fork(void) {                     // phase3
   int pid;
   asm("movl %1, %%eax;     // # for kernel to identify service
        int $128;           // interrupt!
        movl %%ebx, %0"     // after, copy ebx to return
       : "=g" (pid)         // output from asm()
       : "g" (SYS_FORK)  // input to asm()
       : "eax", "ebx"       // clobbered registers
   );
   return pid;
}

unsigned sys_get_rand(void){
  unsigned rand;
  asm("movl %1, %%eax;     // # for kernel to identify service
       int $128;           // interrupt!
       movl %%ebx, %0"     // after, copy ebx to return
      : "=g" (rand)         // output from asm()
      : "g" (SYS_GET_RAND)  // input to asm()
      : "eax", "ebx"       // clobbered registers
  );
  return rand;
}

void sys_lock_mutex(int mutex_id){
  asm("movl %0, %%eax;          // # for kernel to identify service
       movl %1, %%ebx;          // string to write
       int $128"                // interrupt!
      :                         // no output from asm()
      : "g" (SYS_LOCK_MUTEX), "g" (mutex_id)  // 2 inputs to asm()
      : "eax", "ebx"            // clobbered registers
 );
}

void sys_unlock_mutex(int mutex_id){
  asm("movl %0, %%eax;          // # for kernel to identify service
       movl %1, %%ebx;          // string to write
       int $128"                // interrupt!
      :                         // no output from asm()
      : "g" (SYS_UNLOCK_MUTEX), "g" (mutex_id)  // 2 inputs to asm()
      : "eax", "ebx"            // clobbered registers
 );
}

void sys_exit(int exit_code) {  // phase2
   asm("movl %0, %%eax;          // # for kernel to identify service
        movl %1, %%ebx;          // exit code
        int $128"                // interrupt!
       :                         // no output from asm()
       : "g" (SYS_EXIT), "g" (exit_code)  // 2 inputs to asm()
       : "eax", "ebx"            // clobbered registers
   );
}

