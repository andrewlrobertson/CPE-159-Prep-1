// ksr.c, 159

//need to include spede.h, const-type.h, ext-data.h, tools.h
#include "spede.h"
#include "const-type.h"
#include "ext-data.h"
#include "tools.h"



// to create a process: alloc PID, PCB, and process stack
// build trapframe, initialize PCB, record PID to ready_que
void SpawnSR(func_p_t p) {     // arg: where process code starts
   int pid;

   /*use a tool function to check if available queue is empty:
      a. cons_printf("Panic: out of PID!\n");
      b. and go into GDB*/

   if( QueEmpty(&avail_que) == 1){
      cons_printf("Panic: out of PID!\n");
	  breakpoint();
   }

   //get 'pid' initialized by dequeuing the available queue
   pid = DeQue(&avail_que);

   //use a tool function to clear the content of PCB of process 'pid'
   Bzero((char *)&pcb[pid], sizeof(pcb_t));

   //set the state of the process 'pid' to READY
   pcb[pid].state = READY;

   //if 'pid' is not IDLE, use a tool function to enqueue it to the ready queue
   if(pid != IDLE) EnQue(pid, &ready_que);

   //use a tool function to copy from 'p' to DRAM_START, for STACK_MAX bytes
   MemCpy((char*)DRAM_START + pid * STACK_MAX, (char*)p, STACK_MAX);

   //create trapframe for process 'pid:'

   //1st position trapframe pointer in i+ts PCB to the end of the stack
   pcb[pid].tf_p = (tf_t*)((DRAM_START + pid * STACK_MAX) + STACK_MAX - sizeof(tf_t));

   //set efl in trapframe to EF_DEFAULT_VALUE|EF_INTR  // handle intr
   pcb[pid].tf_p->efl = EF_DEFAULT_VALUE | EF_INTR;
   //set cs in trapframe to return of calling get_cs() // duplicate from CPU
   pcb[pid].tf_p->cs = get_cs();
   //set eip in trapframe to DRAM_START                // where code copied
   pcb[pid].tf_p->eip = DRAM_START + pid * STACK_MAX;
}

// count run time and switch if hitting time limit
void TimerSR(void) {
	int x;
   //1st notify PIC control register that timer event is now served
   outportb(PIC_CONT_REG, TIMER_SERVED_VAL);
   //increment system time count by 1
   sys_time_count++;
   //increment the time count of the process currently running by 1
   pcb[run_pid].time_count++;
   //increment the life span count of the process currently running by 1
   pcb[run_pid].total_time++;


   //Use a loop to look for any processes that need to be waken up!
   for(x = 0; x < PROC_MAX; x++){
		if (pcb[x].wake_time == sys_time_count){
			pcb[x].state = READY;
			EnQue(x, &ready_que);
		}
   }
   //also add here that:
   //if run_pid is IDLE, just simply return;    // Idle exempt from below, phase2
   if(run_pid == IDLE) return;



   //if the time count of the process is reaching maximum allowed runtime
   if(pcb[run_pid].time_count == TIME_MAX){
      //move the process back to the ready queue
	   EnQue(run_pid, &ready_que);
      //alter its state to indicate it is not running but ...
	   pcb[run_pid].state = READY;
      //reset the PID of the process in run to NONE
	   run_pid = NONE;
   }
}

void SysSleep(void) {
   int sleep_sec;
   sleep_sec = pcb[run_pid].tf_p->ebx;
   pcb[run_pid].wake_time = sys_time_count + 100*(sleep_sec);
   pcb[run_pid].state = SLEEP;
   run_pid = NONE;
   /*calculate the wake time of the running process using the current system
   time count plus the sleep_sec times 100
   alter the state of the running process to SLEEP
   alter the value of run_pid to NONE*/
}

void SysWrite(void) {
  char* str;
  str = (char*)pcb[run_pid].tf_p->ebx;

  while (*str != '\0'){
    *sys_cursor = *str + VGA_MASK_VAL;
    sys_cursor++;
    str++;
    if(sys_cursor >= (VIDEO_END)){
       sys_cursor = VIDEO_START;
    }
  }
   /*char *str =  ... passed over by a register value wtihin the trapframe
   show the str one char at a time (use a loop)
      onto the console (at the system cursor position)
      (while doing so, the cursor advances and may wrap to top-left corner if needed)*/
}

void SyscallSR(void) {
   switch ( pcb[run_pid].tf_p->eax)
   {
      case SYS_GET_PID:    pcb[run_pid].tf_p->ebx = run_pid;
                           break;
      case SYS_GET_TIME:   pcb[run_pid].tf_p->ebx = sys_time_count;
                           break;
      case SYS_SLEEP:      SysSleep();
                           break;
      case SYS_WRITE:      SysWrite();
                           break;
      default:             cons_printf("Kernel Panic: no such syscall!\n");
                           breakpoint();
   }
}

void SysSetCursor(void){
   sys_cursor = ((unsigned short *)0xb8000 + pcb[run_pid].tf_p->ebx * pcb[run_pid].tf_p->ecx);
}
