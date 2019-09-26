// proc.c, 159
// processes are coded here

#include "const-type.h"  // VGA_MASK_VAL
#include "ext-data.h"    // sys_time_count
#include "tools.h"       // Number2Str()



/*Code an Idle() function that doesn't have any input or return, but just
flickers the dot symbol at the upper-left corner of the target PC display.*/


void Idle(void){
   unsigned short *upper_left_pos = (unsigned short *)0xb8000;
   int flag = 0;
	while(1){
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
  int my_pid, os_time;
  //declare two 20-char arrays: pid_str & time_str
  char pid_str[20];
  char time_str[20];
  
  int forked_pid;
  
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
		sys_sleep(1);
		// set cursor position to my row (equal to my PID), column 0,
		sys_set_cursor(my_pid, 0);
		// call sys_write a few times to show my PID as before,
		sys_write("my PID is ");
		sys_write(pid_str);
		sys_write("... ");
		// get time, and convert it,
		os_time = sys_get_time();
		Number2Str(os_time, time_str)
		// sleep for a second,
		sys_sleep(1);
		// set cursor position back again,
		sys_set_cursor(0, 0);
		// call sys_write a few times to show sys time as before.
		sys_write("sys time is ");
		sys_write(time_str);
		sys_write("... ");	   

   }
}
