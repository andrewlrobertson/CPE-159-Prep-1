// kernel.c, 159, phase 1
//
// Team Name: smooth_operators
// Members:   Halterman, Ehsan
//            Johnson, Kaicha
//            Robertson, Andrew

#include "spede.h"
#include "const-type.h"
#include "entry.h"    // entries to kernel (TimerEntry, etc.)
#include "tools.h"    // small handy functions
#include "ksr.h"      // kernel service routines
#include "proc.h"     // all user process code here

// declare kernel data
// current running PID; if -1, none selected
int run_pid;
que_t avail_que, ready_que;
pcb_t pcb[PROC_MAX];
unsigned short *sys_cursor;
unsigned int sys_time_count;
mutex_t video_mutex;
unsigned sys_rand_count;
kb_t kb;

struct i386_gate *idt;         // interrupt descriptor table

void BootStrap(void) {         // set up kernel!

	int x;
	sys_time_count = 0;
	sys_rand_count = 0;
	sys_cursor = VIDEO_START;

   Bzero((char *)&avail_que, sizeof(que_t));
   Bzero((char *)&ready_que, sizeof(que_t));
	 Bzero((char *)&video_mutex, sizeof(mutex_t));
	 Bzero((char *)&kb, sizeof(kb_t));
   for(x = 0; x < PROC_MAX; x++){
      EnQue(x, &avail_que);
   }

   //get IDT location
   idt = get_idt_base();
   //addr of TimerEntry and SyscallEntry is placed into proper IDT entry
   fill_gate(&idt[TIMER_EVENT], (int)TimerEntry, get_cs(), ACC_INTR_GATE, 0);
   //use fill_gate() to set entry # SYSCALL_EVENT to SyscallEntry
   fill_gate(&idt[SYSCALL_EVENT], (int)SyscallEntry, get_cs(), ACC_INTR_GATE, 0);
   //send PIC control register the mask value for timer handling
   outportb(PIC_MASK_REG, PIC_MASK_VAL);
}

int main(void) {               // OS starts
   //do the boot strap things 1st
   BootStrap();

   SpawnSR(Idle);
   SpawnSR(Login);
   run_pid = IDLE;
   //call Loader() to load the trapframe of Idle
   Loader(pcb[run_pid].tf_p);



   return 0; // never would actually reach here
}

void Scheduler(void) {              // choose a run_pid to run
   if(run_pid > IDLE) return;       // a user PID is already picked

   if(QueEmpty(&ready_que)) {
      run_pid = IDLE;               // use the Idle thread
   } else {
      pcb[IDLE].state = READY;
      run_pid = DeQue(&ready_que);  // pick a different proc
   }

   pcb[run_pid].time_count = 0;     // reset runtime count
   pcb[run_pid].state = RUN;
}

void Kernel(tf_t *tf_p) {       // kernel runs

   char ch;
   //copy tf_p to the trapframe ptr (in PCB) of the process in run
   pcb[run_pid].tf_p = tf_p;

	switch(tf_p->event) {
	case TIMER_EVENT:
      TimerSR();         // handle tiemr event
      break;
	case SYSCALL_EVENT:
      SyscallSR();       // all syscalls go here 1st
      break;
	default:
      cons_printf("Kernel Panic: no such event!\n");
      breakpoint();
   }

   //if 'b' key on target PC is pressed, goto the GDB prompt
   if(cons_kbhit()){
      ch = cons_getchar();
      if(ch == 'b'){ breakpoint();}
      if(ch == ' '){ SpawnSR(Init);}
   }
   //call Scheduler() to change run_pid if needed
   Scheduler();
   //Call Loader() to load the trapframe of the selected process
   Loader(pcb[run_pid].tf_p);
}
