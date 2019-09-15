// kernel.c, 159, phase 1
//
// Team Name: Please_give_us_an_A 
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
//declare an integer: run_pid;  // current running PID; if -1, none selected
int run_pid;
//declare 2 queues: avail_que and ready_que;  // avail PID and those created/ready to run
que_t avail_que, ready_que;
//declare an array of PCB type: pcb[PROC_MAX];  // Process Control Blocks
pcb_t pcb[PROC_MAX];

//declare an unsigned integer: sys_time_count
unsigned int sys_time_count;

struct i386_gate *idt;         // interrupt descriptor table

void BootStrap(void) {         // set up kernel!
   //set sys time count to zero
	sys_time_count = 0;
	int x;

   //call tool Bzero() to clear avail queue
   Bzero((char *)&avail_que, sizeof(que_t));
   //call tool Bzero() to clear ready queue
   Bzero((char *)&ready_que, sizeof(que_t));
   //enqueue all the available PID numbers to avail queue
   for(x = 0; x < PROC_MAX; x++){
      EnQue(x, &avail_que);
   }

   //get IDT location
   idt = get_idt_base();
   //addr of TimerEntry is placed into proper IDT entry
   fill_gate(&idt[TIMER_EVENT], (int)TimerEntry, get_cs(), ACC_INTR_GATE, 0);
   //send PIC control register the mask value for timer handling
   outportb(PIC_MASK_REG, PIC_MASK_VAL);
}

int main(void) {               // OS starts
   //do the boot strap things 1st
   BootStrap();

   SpawnSR(Idle);              // create Idle thread
   //set run_pid to IDLE
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

   //call the timer service routine
   TimerSR();

   //if 'b' key on target PC is pressed, goto the GDB prompt
   if(cons_kbhit() == 1) ch = cons_getchar();
   if(ch == 'b') breakpoint (); 
   //call Scheduler() to change run_pid if needed
   Scheduler();
   //Call Loader() to load the trapframe of the selected process
   Loader(tf_p); 
}

