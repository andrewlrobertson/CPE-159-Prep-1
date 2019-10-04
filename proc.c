// proc.c, 159
// processes are coded here

#include "const-type.h"  // VGA_MASK_VAL
#include "ext-data.h"    // sys_time_count
#include "tools.h"       // Number2Str()
#include "syscall.h"     //sys_write, sys_fork etc



/*Code an Idle() function that doesn't have any input or return, but just
flickers the dot symbol at the upper-left corner of the target PC display.*/


void Idle(void){
   unsigned short *upper_left_pos = (unsigned short *)0xb8000;
   int flag = 0;
	while(1){
    sys_rand_count++;
		if(sys_time_count % 100 == 0) {
			if(flag == 0) {
				*upper_left_pos = '*' + VGA_MASK_VAL;
				flag = 1;
			} else {
				*upper_left_pos = ' ' + VGA_MASK_VAL;
				flag = 0;
			}
		}
	}
}

void Init(void) {  // Init, PID 1, asks/tests various OS services
  //declare two integers: my_pid & os_time
  int col, my_pid, rand, forked_pid;
  //declare two 20-char arrays: pid_str & time_str
  char pid_str[20];

  forked_pid = sys_fork();
  if(forked_pid == NONE){
	  sys_write("sys_fork() failed!\n");
  }

  forked_pid = sys_fork();
  if(forked_pid == NONE){
	  sys_write("sys_fork() failed!\n");
  }

  my_pid = sys_get_pid();
  Number2Str(my_pid, pid_str);

   while(1) {
		// sleep for a second,
    for(col = 0; col < 70; col++){
      sys_lock_mutex(VIDEO_MUTEX);
      sys_set_cursor(my_pid, col);
      sys_write(my_pid);
      sys_unlock_mutex(VIDEO_MUTEX);
      rand = (sys_get_rand() % 4) + 1;
      sys_sleep(rand);
    }
    sys_lock_mutex(VIDEO_MUTEX);
    for(col = 0; col < 70; col++){
      sys_set_cursor(my_pid, col);
      sys_write(" ");
    }
    sys_unlock_mutex(VIDEO_MUTEX);
    sys_sleep(30);
   }
}
