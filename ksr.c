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

   if( QueEmpty(&avail_que) == 1){
      cons_printf("Panic: out of PID!\n");
	  breakpoint();
   }

   //get 'pid'
   pid = DeQue(&avail_que);
   page[pid].pid = pid;
   //use a tool function to clear the content of PCB of process 'pid'
   Bzero((char *)&pcb[pid], sizeof(pcb_t));
   if(pid == 0){
     pcb[pid].STDOUT = CONSOLE;
     pcb[pid].STDIN = CONSOLE;}
   else{
     pcb[pid].STDOUT = TTY;
     pcb[pid].STDIN = TTY;}
   pcb[pid].Dir = KDir;
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

void KBSR(void){
  char ch;
  int proc;
  if(!cons_kbhit()) return;
  ch = cons_getchar();
  if(ch == '$') breakpoint();

  if (QueEmpty(&(kb.wait_que))){
     EnQue((int)ch, &(kb.buffer));
   }
  else{
     proc = DeQue(&(kb.wait_que));
     EnQue(proc, &ready_que);
     pcb[proc].state = READY;
     set_cr3(pcb[proc].Dir);
     pcb[proc].tf_p->ebx = (int)ch;
     set_cr3(KDir);
   }
}

void TTYdspSR(void){
  int pid;
  char ch;
  if(!QueEmpty(&(tty.echo))){
    ch = (char)DeQue(&(tty.echo));
    outportb(tty.port, ch);
    return;
  }
  if(QueEmpty(&(tty.dsp_wait_que))){
    return;
  }
  pid = tty.dsp_wait_que.que[0];
  set_cr3(pcb[pid].Dir);

  ch = *tty.dsp_str;
  if (ch != '\0'){
    outportb(tty.port, ch);
    tty.dsp_str++;}
  else{
    DeQue(&(tty.dsp_wait_que));
    EnQue(pid, &ready_que);
    pcb[pid].state = READY;
  }
}

void TTYkbSR(void){
  char ch;
  int pid;
  ch = inportb(tty.port);
  if (QueEmpty(&(tty.kb_wait_que))){
    return;
  }
  EnQue((int) ch, &(tty.echo));
  set_cr3(pcb[tty.kb_wait_que.que[0]].Dir);
 // if(StrCmp(&ch, "\r") != 0){
    if(ch != '\r'){
    *tty.kb_str = ch;
    tty.kb_str++;
  }
  else{
    EnQue((int)'\n', &(tty.echo));
    *tty.kb_str = '\0';
    pid = DeQue((&(tty.kb_wait_que)));
    pcb[pid].state = READY;
    EnQue(pid, &ready_que);
  }
}


void TTYSR(void){
   int status;
   outportb(PIC_CONT_REG, TTY_SERVED_VAL);
   status = inportb(tty.port+IIR);
   if (status == IIR_TXRDY){
      TTYdspSR();
   }
   else if (status == IIR_RXRDY){
      TTYkbSR();
      TTYdspSR();
   }
   else{
      //do nothing
   }
}

void SysSleep(void) {
   int sleep_sec;
   sleep_sec = pcb[run_pid].tf_p->ebx;
   pcb[run_pid].wake_time = sys_time_count + 10*(sleep_sec);
   pcb[run_pid].state = SLEEP;
   run_pid = NONE;
}

void SysWrite(void) {
  char* str;
  int offset;

  str = (char*)pcb[run_pid].tf_p->ebx;
  if(pcb[run_pid].STDOUT == CONSOLE){
     while (*str != '\0'){
       if(*str == '\r'){
          offset = (sys_cursor-VIDEO_START) % 80;
          sys_cursor -= offset;
          sys_cursor += 80;
       }
       else{
          *sys_cursor = *str + VGA_MASK_VAL;
          sys_cursor++;
       }
       str++;
       if(sys_cursor >= (VIDEO_END)){
          sys_cursor = VIDEO_START;
          while (sys_cursor < VIDEO_END){
            *sys_cursor = ' ' + VGA_MASK_VAL;
            sys_cursor++;
          }
          sys_cursor = VIDEO_START;
       }
     }
   }
   else if(pcb[run_pid].STDOUT == TTY){
     tty.dsp_str = str;
     EnQue(run_pid, &(tty.dsp_wait_que));
     pcb[run_pid].state = IO_WAIT;
     run_pid = NONE;
     TTYdspSR();
   }
  else{
    cons_printf("NO SUCH DEVICE!\n");
    breakpoint();
  }
}


void SysSetCursor(void){
   sys_cursor = (unsigned short *)(0xb8000 + 2 * ((pcb[run_pid].tf_p->ebx * 80) + pcb[run_pid].tf_p->ecx));
}

void SysFork(void){
	int pid;
	int distance;
  int *p;

	pid = DeQue(&avail_que);
	if (pid == NONE){
		pcb[run_pid].tf_p->ebx = NONE;
		return;
	}
  pcb[pid].Dir = KDir;
  page[pid].pid = pid;
	EnQue(pid, &ready_que);
	pcb[pid] = pcb[run_pid];
	pcb[pid].state = READY;
	pcb[pid].time_count = 0;
	pcb[pid].total_time = 0;
	pcb[pid].ppid = run_pid;

	MemCpy((char*)DRAM_START + pid * STACK_MAX, (char*)DRAM_START + run_pid * STACK_MAX, STACK_MAX);

	// 4. calculate the byte distance between the two processes
	distance = (pid - run_pid) * STACK_MAX;
	// 5. apply the distance to the trapframe location in child's PCB
	(char*)pcb[pid].tf_p = (char*)pcb[run_pid].tf_p + distance;
  pcb[pid].tf_p->eip = pcb[run_pid].tf_p->eip + distance;
	pcb[pid].tf_p->ebp = pcb[run_pid].tf_p->ebp + distance;      //This is to change the location pointed to
	p = (int *)pcb[pid].tf_p->ebp;
  *p += distance;
  p++;
  *p += distance;
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
        pcb[released_pid].state = READY;
        EnQue(released_pid, &ready_que);
      } else {
         video_mutex.lock = UNLOCKED;
      }
    } else {
      cons_printf("Panic: no such mutex ID!\n");
      breakpoint();
  }
}

void AlterStack(int pid, func_p_t p){
	int old_eip;
  set_cr3(pcb[pid].Dir);
	old_eip = pcb[pid].tf_p->eip;                                           //save old eip
	MemCpy((char*)(pcb[pid].tf_p) - 4, (char*)pcb[pid].tf_p, sizeof(tf_t));     //Lower trap frame by 4 bytes
	(char*)pcb[pid].tf_p -= 4;                                                 //adjust tf pointer to new location
	(char*)pcb[pid].tf_p->eip =  (char*)p;                                      //replace eip with 'p'
	*(int*)(pcb[pid].tf_p + 1) = old_eip;                                       //place old eip in gap
  set_cr3(KDir);
}

void SysExit(void){
	int ppid, exit_code, x;

  exit_code = pcb[run_pid].tf_p->ebx;
	ppid = pcb[run_pid].ppid;

	if(pcb[ppid].state != WAIT){
		//running process cannot exit, it becomes a zombie
		pcb[run_pid].state = ZOMBIE;
        //no running process anymore
        if(pcb[ppid].signal_handler[SIGCHLD] != 0){
            AlterStack(ppid, pcb[ppid].signal_handler[SIGCHLD]);
        }
		run_pid = NONE;
	} else {
		//release parent:
            //upgrade parent's state
			pcb[ppid].state = READY;
            //move parent to be ready to run again
			EnQue(ppid, &ready_que);
         //also:
            //pass over exiting PID to parent
      set_cr3(pcb[ppid].Dir);
			pcb[ppid].tf_p->ecx = run_pid;
            //pass over exit code to parent
			*(int*)pcb[ppid].tf_p->ebx = exit_code;
         //also:
            //reclaim child resources (alter state, move it)
			pcb[run_pid].state = AVAIL;
			EnQue(run_pid, &avail_que);
            //no running process anymore
      for(x = 0; x < PAGE_MAX; x++){
        if(page[x].pid == run_pid) page[x].pid = NONE;
      }
      set_cr3(KDir);
			run_pid = NONE;
	}
}

void SysWait(void){
   int * exit_code_ptr;
   int cpid, x;
   cpid = NONE;
   exit_code_ptr = (int*)pcb[run_pid].tf_p->ebx;
   for(x=0; x<PROC_MAX;x++){
	   if((pcb[x].state == ZOMBIE) && (pcb[x].ppid == run_pid)){
		   cpid = x;
		   break;
	   }
   }
   if(cpid == NONE){
	   pcb[run_pid].state = WAIT;
	   run_pid = NONE;
   }
   else{
	   pcb[run_pid].tf_p->ecx = cpid;
	   //need to modify exit code here
     set_cr3(pcb[cpid].Dir);
	   *exit_code_ptr = pcb[cpid].tf_p->ebx;
	   pcb[cpid].state = AVAIL;
     EnQue(cpid, &avail_que);
     for(x = 0; x < PAGE_MAX; x++){
       if(page[x].pid == cpid) page[x].pid = NONE;
     }
     set_cr3(KDir);
   }
}

void SysKill(void){
  int pid;
  int signal_name;
  int x;

  pid = pcb[run_pid].tf_p->ebx;
  signal_name = pcb[run_pid].tf_p->ecx;

  if((pid == 0) && (signal_name == SIGCONT)){
      for(x=0;x<PROC_MAX;x++){
        if((pcb[x].ppid == run_pid) && (pcb[x].state == SLEEP)){
            pcb[x].state = READY;
            EnQue(x, &ready_que);
        }
      }
  }
}

void SysSignal(void){
  int signal_name;
  func_p_t p;

  signal_name = pcb[run_pid].tf_p->ebx;
  p = (func_p_t)pcb[run_pid].tf_p->ecx;

  pcb[run_pid].signal_handler[signal_name] = p;
}

void SysRead(void){
  int ch;

  if(pcb[run_pid].STDIN == CONSOLE){

  	if(!QueEmpty(&(kb.buffer))){
    	   ch = DeQue(&(kb.buffer));
    	   pcb[run_pid].tf_p->ebx = ch;
  	}
  	else{
    	   EnQue(run_pid, &(kb.wait_que));
    	   pcb[run_pid].state = IO_WAIT;
    	   run_pid = NONE;
  	}
  }
   else if(pcb[run_pid].STDIN == TTY){
	tty.kb_str = (char*)pcb[run_pid].tf_p->ebx;
	pcb[run_pid].state = IO_WAIT;
	EnQue(run_pid, &(tty.kb_wait_que));
        run_pid = NONE;
   }
   else{
	cons_printf("NO SUCH DEVICE");
        breakpoint();
   }
}

void SysVfork(void){
  int DIR, IT, DT, IP, DP, x, i, pid;
  int indices[5];
  func_p_t funct;
  unsigned * KDir_ptr;

  i = 0;
  x = 0;
  funct = (func_p_t)pcb[run_pid].tf_p->ebx;
  KDir_ptr = (unsigned *)KDir;

  pid = DeQue(&avail_que);
	EnQue(pid, &ready_que);
  pcb[pid] = pcb[run_pid];
	pcb[pid].state = READY;
	pcb[pid].time_count = 0;
	pcb[pid].total_time = 0;
	pcb[pid].ppid = run_pid;
  pcb[pid].tf_p =(tf_t *)(G2 - sizeof(tf_t));

  while(i<5){          //while we have less than five pages
   //look for a page
     if(page[x].pid == NONE){
        indices[i] = x;
        i++;
     }
     x++;
     if (x == PAGE_MAX || i == 5) break;
  }

  if (i<5) {
    cons_printf("NOT ENOUGH PAGES!");
    breakpoint();
  }

  for(x = 0; x < 5; x++){
     page[indices[x]].pid = pid;
     Bzero(page[indices[x]].u.content, sizeof(PAGE_SIZE));
  }

  DIR = indices[0];
  IT = indices[1];
  DT = indices[2];
  IP = indices[3];
  DP = indices[4];

  for(x = 0; x < 16; x++){                //build DIR page
     page[DIR].u.entry[x] = KDir_ptr[x];
  }
  page[DIR].u.entry[256] = (page[IT].u.addr | PRESENT | RW);
  page[DIR].u.entry[511] = (page[DT].u.addr | PRESENT | RW);

  page[IT].u.entry[0] = (page[IP].u.addr | PRESENT | RO);       // build IT page

  page[DT].u.entry[1023] = (page[DP].u.addr | PRESENT | RW);     // build DT page

  MemCpy(page[IP].u.content, (char*)funct, PAGE_SIZE);      //build IP page

  page[DP].u.entry[1023] = EF_DEFAULT_VALUE | EF_INTR;     //build DP page
  page[DP].u.entry[1022] = get_cs();
  page[DP].u.entry[1021] = G1;

  pcb[pid].Dir = page[DIR].u.addr;
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
      case SYS_EXIT:          SysExit();
			                        break;
      case SYS_WAIT:          SysWait();
			                        break;
      case SYS_SIGNAL:        SysSignal();
			                        break;
      case SYS_KILL:          SysKill();
			                        break;
      case SYS_READ:          SysRead();
                        			break;
      case SYS_VFORK:         SysVfork();
                              break;
      default:             cons_printf("Kernel Panic: no such syscall!\n");
                           breakpoint();
   }

   if(run_pid != NONE){
      pcb[run_pid].state = READY;
      EnQue(run_pid, &ready_que);
      run_pid = NONE;
   }
   set_cr3(KDir);
}
