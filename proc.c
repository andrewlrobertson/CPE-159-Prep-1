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

void ShellDir(void){
  sys_lock_mutex(VIDEO_MUTEX);
	sys_write("/boot/\r");
  sys_unlock_mutex(VIDEO_MUTEX);
  sys_exit(0);
}

void ShellRoll(void){
	int die1, die2;
	die1 = (sys_get_rand()%6 + 1);
	die2 = (sys_get_rand()%6 + 1);
	sys_exit(die1+die2);
}

void ShellCal(void){
  sys_lock_mutex(VIDEO_MUTEX);
	sys_write("\r");
  sys_write("      November 2019\r");
  sys_write("   Su Mo Tu We Th Fr Sa\r");
  sys_write("                   1  2\r");
  sys_write("    3  4  5  6  7  8  9\r");
  sys_write("   10 11 12 13 14 15 16\r");
  sys_write("   17 18 19 20 21 22 23\r");
  sys_write("   24 25 26 27 28 29 30\r");
  sys_unlock_mutex(VIDEO_MUTEX);
  sys_exit(0);
}

void VforkOutput(int exit_code, int exit_pid){
  char str[STR_MAX];
  char str2[STR_MAX];
  Number2Str(exit_pid, str);
  Number2Str(exit_code, str2);
  sys_lock_mutex(VIDEO_MUTEX);
  sys_write("Exited PID: ");
  sys_write(str);
  sys_write("   Exit Code: ");
  sys_write(str2);
  sys_write("\r");
  sys_unlock_mutex(VIDEO_MUTEX);
}

void Shell(void){
  char command[STR_MAX];
	int exit_code;
  int exit_pid;
	while(1){
		sys_write("smooth_operators>");
		sys_read(command);
		// Following is a simple C program
			if(StrCmp(command , "dir") == 0){
					sys_vfork(ShellDir);
          exit_pid = sys_wait(&exit_code);
          VforkOutput(exit_code,exit_pid);
			}
			else if(StrCmp(command, "cal") == 0){
					sys_vfork(ShellCal);
          exit_pid = sys_wait(&exit_code);
          VforkOutput(exit_code,exit_pid);
			}
			else if(StrCmp(command, "roll") == 0){
					sys_vfork(ShellRoll);
          exit_pid = sys_wait(&exit_code);
          VforkOutput(exit_code,exit_pid);
			}
			else{
					sys_write("\r   Valid commands are:\r");
				  sys_write("      dir -- displays directory content\r");
					sys_write("      cal -- displays calendar\r");
				  sys_write("      roll -- roll a pair of die\r");
			}
	}
}

void Login(void) {
      char login_str[STR_MAX], passwd_str[STR_MAX];

      while(1) {
         sys_write("login:");
         sys_read(login_str);
         sys_write("password:");
         sys_read(passwd_str);

         if (StrCmp(login_str, passwd_str) == 0){
           sys_write("login successful!\r");  // break; in next prep
	         sys_vfork(Shell);  // virtual-fork Shell (virtual memeory) phase8
	         break;
         }
         else{
           sys_write("login failed!\r");
         }
      }
      sys_exit(0);
}

void Init(void) {  // Init, PID 1, asks/tests various OS services
  char pid_str[20];
        int my_pid, forked_pid,
            i, col,
            sleep_period, total_sleep_period;

sys_signal(SIGCHLD, MyChildExitHandler);

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
    sys_sleep(10);
    sys_kill(0, SIGCONT);

    while(1){
    sys_lock_mutex(VIDEO_MUTEX);
    sys_set_cursor(my_pid, 0);
    sys_write(pid_str);
    sys_unlock_mutex(VIDEO_MUTEX);

    sys_sleep(10);

    sys_lock_mutex(VIDEO_MUTEX);
    sys_set_cursor(my_pid, 0);
    sys_write("-");                  //may need to modify
    if(my_pid > 9) sys_write("-");
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

          sleep_period = ((sys_get_rand()/my_pid) % 4) + 1;
          sys_sleep(sleep_period);
          total_sleep_period += sleep_period;

          sys_lock_mutex(VIDEO_MUTEX);
          sys_set_cursor(my_pid, col);
          sys_write(".");                //may need to modify
          if (col==69) sys_write(".");
          sys_unlock_mutex(VIDEO_MUTEX);

          col++;
    }
    sys_exit(total_sleep_period);
  }
}
