// proc.c, 159
// processes are coded here

#include "const-type.h"  // VGA_MASK_VAL
#include "ext-data.h"    // sys_time_count
#include "syscall.h"     // sys service calls
#include "tools.h"       // Number2Str()



/*Code an Idle() function that doesn't have any input or return, but just
flickers the dot symbol at the upper-left corner of the target PC display.

It reads sys_time_count and at each second interval writes '.' or ' ' via
an unsigned short pointer to the VGA video memory location 0xb8000.
Apply the VGA_MASK_VAL when showing . or space so it will appear to be
boldface writing.
    ... declare a pointer and set it to the upper-left display corner ...
    ... declare and set a flag ...
    in an infinite loop:
       whenever the system time reaches a multiple of 100 (per 1 second):
          a. judging from the flag and show either the dot or space
          b. alternate the flag*/

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

   //call sys_get_pid() to get my_pid
   my_pid = sys_get_pid();
   //call Number2Str() to convert it to pid_str
   Number2Str(my_pid, pid_str);

   while(1) {
      // call sys_write() to show "my PID is "
	  sys_write("my PID is ");
      // call sys_write() to show my pid_str
	  sys_write(pid_str);
      // call sys_write to show "... "
	  sys_write("... ");
      // call sys_sleep() to sleep for 1 second
	  sys_sleep(1);
      // call sys_get_time() to get current os_time
	  os_time = sys_get_time();
      // call Number2Str() to convert it to time_str
	  Number2Str(os_time, time_str);
      // call sys_write() to show "sys time is "
	  sys_write("sys time is ");
      // call sys_write() to show time_str
	  sys_write(time_str);
      // call sys_write to show "... "
	  sys_write("... ");
      // call sys_sleep() to sleep for 1 second
	  sys_sleep(1);
   }
}