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

void MyChildExitHandler(void){
    int cpid, exit_code;   //my_pid isn't needed
    char cpid_str[20], ec_str[20];

    cpid = sys_wait(&exit_code);
    //my_pid = sys_get_pid(); This isn't actually needed

    Number2Str(cpid, cpid_str);
    Number2Str(exit_code, ec_str);

    sys_lock_mutex(VIDEO_MUTEX);
    sys_set_cursor(cpid, 72);
    sys_write(cpid_str);
    sys_write(":");
    sys_write(ec_str);
    sys_unlock_mutex(VIDEO_MUTEX);

}

void Init(void) {  // Init, PID 1, asks/tests various OS services
  char pid_str[20], str[20];
        int my_pid, forked_pid,
            i, col, exit_pid, exit_code,
            sleep_period, total_sleep_period;

//sys_signal(SIGCHLD, MyChildExitHandler);

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

  if (forked_pid > 0){
    sys_lock_mutex(VIDEO_MUTEX);
    sys_set_cursor(my_pid, 0);
    sys_write(pid_str);
    sys_unlock_mutex(VIDEO_MUTEX);

    sys_sleep(10);

    sys_lock_mutex(VIDEO_MUTEX);
    sys_set_cursor(my_pid, 0);
    sys_write("-");                  //may need to modify
    sys_unlock_mutex(VIDEO_MUTEX);

    sys_sleep(10);
    }

  }
  else{
    sys_sleep(1000000);

       col = total_sleep_period = 0;
       while (col<70) {
          sys_lock_mutex(VIDEO_MUTEX);
          sys_set_cursor(my_pid, col);
          sys_write(pid_str);
          sys_unlock_mutex(VIDEO_MUTEX);

          sleep_period = ((SYS_GET_RAND()/my_pid) % 4) + 1;
          sys_sleep(sleep_period);
          total_sleep_period += sleep_period;

          sys_lock_mutex(VIDEO_MUTEX);
          sys_set_cursor(my_pid, col);
          sys_write(".");                //may need to modify
          sys_unlock_mutex(VIDEO_MUTEX);

          col++;
    }
    sys_exit(total_sleep_period);
  }
}
