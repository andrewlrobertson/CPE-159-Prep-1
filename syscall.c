// syscall.c
// system service calls for kernel services

#include "const-type.h"     // for SYS_GET_PID, etc., below

int sys_get_pid(void) {
   int pid;
   asm("movl %1, %%eax;
        int $128;
        movl %%ebx, %0"     //pid
       : "=g" (pid)         // output from asm()
       : "g" (SYS_GET_PID)  // input to asm()
       : "eax", "ebx"       // clobbered registers
   );

   return pid;
}

int sys_get_time(void) {
   int time;
   asm("movl %1, %%eax;
        int $128;
        movl %%ebx, %0"     //time
       : "=g" (time)
       : "g" (SYS_GET_TIME)
       : "eax", "ebx"
   );

   return time;
}

void sys_sleep(int sleep_sec) {
   asm("movl %0, %%eax;
        movl %1, %%ebx;          // sleep seconds
        int $128"
       :
       : "g" (SYS_SLEEP), "g" (sleep_sec)
       : "eax", "ebx"
   );
}

void sys_write(char* str) {
   asm("movl %0, %%eax;
        movl %1, %%ebx;          // string to write
        int $128"
       :
       : "g" (SYS_WRITE), "g" (str)
       : "eax", "ebx"
	);
}

void sys_set_cursor(int row, int col) {
  asm("movl %0, %%eax;
       movl %1, %%ebx;          // row
       movl %2, %%ecx;          // col
       int $128"
      :
      : "g" (SYS_SET_CURSOR), "g" (row), "g" (col)
      : "eax", "ebx", "ecx"
 );
}

int sys_fork(void) {
   int pid;
   asm("movl %1, %%eax;
        int $128;
        movl %%ebx, %0"     // pid
       : "=g" (pid)
       : "g" (SYS_FORK)
       : "eax", "ebx"
   );
   return pid;
}

unsigned sys_get_rand(void){
  unsigned rand;
  asm("movl %1, %%eax;
       int $128;
       movl %%ebx, %0"     // rand
      : "=g" (rand)
      : "g" (SYS_GET_RAND)
      : "eax", "ebx"
  );
  return rand;
}

void sys_lock_mutex(int mutex_id){
  asm("movl %0, %%eax;
       movl %1, %%ebx;          // mutex_id
       int $128"
      :
      : "g" (SYS_LOCK_MUTEX), "g" (mutex_id)
      : "eax", "ebx"
 );
}

void sys_unlock_mutex(int mutex_id){
  asm("movl %0, %%eax;
       movl %1, %%ebx;          // mutex_id
       int $128"
      :
      : "g" (SYS_UNLOCK_MUTEX), "g" (mutex_id)
      : "eax", "ebx"
 );
}

void sys_exit(int exit_code) {
   asm("movl %0, %%eax;
        movl %1, %%ebx;          // exit code
        int $128"
       :
       : "g" (SYS_EXIT), "g" (exit_code)
       : "eax", "ebx"
   );
}

int sys_wait(int * exit_code) {
   int cpid;
   asm("movl %1, %%eax;
	      movl %2, %%ebx;     //exit_code addr
        int $128;
        movl %%ecx, %0"     // cpid
       : "=g" (cpid)
       : "g" (SYS_WAIT), "g" (exit_code)
       : "eax", "ebx" , "ecx"
   );
   return cpid;
}

void sys_signal(int signal_name, func_p_t p) {
  asm("movl %0, %%eax;
       movl %1, %%ebx;          // signal name
       movl %2, %%ecx;          // p
       int $128"
      :
      : "g" (SYS_SIGNAL), "g" (signal_name), "g" (p)
      : "eax", "ebx", "ecx"
 );
}

void sys_kill(int pid, int signal_name) {
  asm("movl %0, %%eax;
       movl %1, %%ebx;          // pid
       movl %2, %%ecx;          // signal name
       int $128"
      :
      : "g" (SYS_KILL), "g" (pid), "g" (signal_name)
      : "eax", "ebx", "ecx"
 );
}

void sys_read(char *str){
     char ch;
     int temp;
     char small[2];
     int i;
     i = 0;
     while( i < (STR_MAX - 1)){

       asm("movl %1, %%eax;
            int $128;
            movl %%ebx, %0"
           : "=g" (temp)
           : "g" (SYS_READ)
           : "eax", "ebx"
       );

       ch = (char)temp;
       small[0] = ch;
       small[1] = '\0'
       sys_write(small);

       if (ch == '\r'){
         *str = '\0';
         return;
       }
       *str = ch;
       i++;
       str++;

       if (i == STR_MAX - 1){
         *str = '\0';
         return;
       }
     }
}
