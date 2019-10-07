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
     if ((pcb[x].state == SLEEP) && (pcb[x].wake_time == sys_time_count)){
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
   pcb[run_pid].wake_time = sys_time_count + 10*(sleep_sec);
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

void SysSetCursor(void){
   sys_cursor = (unsigned short *)(0xb8000 + 2 * ((pcb[run_pid].tf_p->ebx * 80) + pcb[run_pid].tf_p->ecx));
}

void SysFork(void){
	int pid;
	int distance;
  int *pnew, *pold, *ebpold, *ebpnew;
	// 1. allocate a new PID and add it to ready_que (similar to start of SpawnSR)
	pid = DeQue(&avail_que);
	EnQue(pid, &ready_que);
	// 2. copy PCB from parent process, but alter these:
	// process state, the two time counts, and ppid
	pcb[pid] = pcb[run_pid];
	pcb[pid].state = READY;
	pcb[pid].time_count = 0;
	pcb[pid].total_time = 0;
	pcb[pid].ppid = run_pid;
	// 3. copy the process image (the 4KB DRAM) from parent to child:
	// figure out destination and source byte addresses
	// use tool MemCpy() to do the copying
	MemCpy((char*)DRAM_START + pid * STACK_MAX, (char*)DRAM_START + run_pid * STACK_MAX, STACK_MAX);

	// 4. calculate the byte distance between the two processes
	// = (child PID - parent PID) * 4K
	distance = (pid - run_pid) * STACK_MAX;
	// 5. apply the distance to the trapframe location in child's PCB
	(char*)pcb[pid].tf_p = (char*)pcb[run_pid].tf_p + distance;  //There's no definition of add for our user defined type
	// 6. use child's trapframe pointer to adjust these in the trapframe:

  // eip (so it points o child's own instructions),
  //pcb[pid].tf_p->eip = DRAM_START + pid * STACK_MAX;
  pcb[pid].tf_p->eip = pcb[run_pid].tf_p->eip + distance;
  // But this change gives a segmentation fault, may be because of something else though

  // ebp (so it points to child's local data),
	// also, the value where ebp points to:
	// treat ebp as an integer pointer and alter what it points to
	/*------------NOT SURE ABOUT THESE 2 LINES -----------------*/
	pcb[pid].tf_p->ebp = pcb[run_pid].tf_p->ebp + distance;      //This is to change the location pointed to
	*(int *)pcb[pid].tf_p->ebp = *(int *)pcb[run_pid].tf_p->ebp + distance; //This is to change the value at that address
	pnew = (int*)pcb[pid].tf_p->ebp;
  pold = (int*)pcb[run_pid].tf_p->ebp;
  ebpnew = (int*)(*pnew);
  ebpold = (int*)(*pold);
  *(ebpnew+1) = *(ebpold+1) + distance; 
	// 7. correctly set return values of sys_fork():
	// ebx in the parent's trapframe gets the new child PID
	// ebx in the child's trapframe gets ?
	pcb[run_pid].tf_p->ebx = pid;
	pcb[pid].tf_p->ebx = 0;

}

void SysLockMutex(void) {   // phase4
   int mutex_id;

   mutex_id = pcb[run_pid].tf_p->ebx;

   if(mutex_id == VIDEO_MUTEX) {
      if (video_mutex.lock == UNLOCKED){
         video_mutex.lock = LOCKED;
      } else {
        EnQue(run_pid, &video_mutex.suspend_que);
        pcb[run_pid].state = SUSPEND;
        run_pid = NONE;
      }
    } else {
      cons_printf("Panic: no such mutex ID!\n");
      breakpoint();
  }
}


void SysUnlockMutex(void) {
   int mutex_id, released_pid;

   mutex_id = pcb[run_pid].tf_p->ebx;

   if(mutex_id == VIDEO_MUTEX) {
      if(!QueEmpty(&video_mutex.suspend_que)) {
        released_pid = DeQue(&video_mutex.suspend_que);
        pcb[released_pid].state = RUN;
        run_pid = released_pid;
      } else {
         video_mutex.lock = UNLOCKED;
      }
    } else {
      cons_printf("Panic: no such mutex ID!\n");
      breakpoint();
  }
}

void SyscallSR(void) {
   switch ( pcb[run_pid].tf_p->eax)
   {
      case SYS_GET_PID:       pcb[run_pid].tf_p->ebx = run_pid;
                              break;
      case SYS_GET_TIME:      pcb[run_pid].tf_p->ebx = sys_time_count;
                              break;
      case SYS_SLEEP:         SysSleep();
                              break;
      case SYS_WRITE:         SysWrite();
                              break;
      case SYS_SET_CURSOR:    SysSetCursor();
                              break;
      case SYS_FORK:          SysFork();
                              break;
      case SYS_GET_RAND:      pcb[run_pid].tf_p->ebx = sys_rand_count;
                              break;
      case SYS_LOCK_MUTEX:    SysLockMutex();
                              break;
      case SYS_UNLOCK_MUTEX:  SysUnlockMutex();
                              break;
      default:             cons_printf("Kernel Panic: no such syscall!\n");
                           breakpoint();
   }

   if(run_pid != NONE){
      pcb[run_pid].state = READY;
      EnQue(run_pid, &ready_que);
      run_pid = NONE;
   }
}
