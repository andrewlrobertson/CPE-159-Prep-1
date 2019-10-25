// proc.c, 159
// processes are coded here

#include "const-type.h"  // VGA_MASK_VAL
#include "ext-data.h"    // sys_time_count
#include "tools.h"       // Number2Str()
#include "syscall.h"     //sys_write, sys_fork etc

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
  char pid_str[20], str[20];
        int my_pid, forked_pid,
            i, col, exit_pid, exit_code,
            sleep_period, total_sleep_period;

  for (i = 0; i < 5; i++){
    forked_pid = sys_fork();
    if(forked_pid == 0) break;
    if(forked_pid == NONE){
      sys_write("sys_fork() failed!\n");
      sys_exit(NONE);
    }
  }

  my_pid = sys_get_pid();
  Number2Str(my_pid, pid_str);

  if (my_pid == 1){
    for (i = 0; i < 5; i++){
      exit_pid = sys_wait(&exit_code);
      sys_lock_mutex(VIDEO_MUTEX);
      sys_set_cursor(my_pid, i*14);
      sys_write("PID ");
      Number2Str(exit_pid, str);
      sys_write(str);
      sys_write(": ");
      Number2Str(exit_code, str);
      sys_write(str);
      sys_unlock_mutex(VIDEO_MUTEX);
    }
    sys_write("Init exits.");
    sys_exit(0);
  }
  else{
    col = total_sleep_period = 0;
    while(col < 70){
      sys_lock_mutex(VIDEO_MUTEX);
      sys_set_cursor(my_pid, col);
      sys_write(pid_str);
      sys_unlock_mutex(VIDEO_MUTEX);
      sleep_period = ((sys_get_rand()/my_pid)%4) + 1;
      sys_sleep(sleep_period);
      total_sleep_period += sleep_period;
      col++;
    }
    sys_exit(total_sleep_period);
  }
}
