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
unsigned KDir;
page_t page[PAGE_MAX];

struct i386_gate *idt;         // interrupt descriptor table

void BootStrap(void) {         // set up kernel!

	int x;
	sys_time_count = 0;
	sys_rand_count = 0;
	sys_cursor = VIDEO_START;
	KDir = get_cr3();
	for (x = 0; x < PAGE_MAX; x++){
		page[x].pid = NONE;
		page[x].u.addr = DRAM_START + x * PAGE_SIZE;
	}

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

void TTYinit(void) {                // phase9
      int i, j;

      Bzero((char *)&tty, sizeof(tty_t));
      tty.port = TTY0;

      outportb(tty.port+CFCR, CFCR_DLAB);             // CFCR_DLAB is 0x80
      outportb(tty.port+BAUDLO, LOBYTE(115200/9600)); // period of each of 9600 bauds
      outportb(tty.port+BAUDHI, HIBYTE(115200/9600));
      outportb(tty.port+CFCR, CFCR_PEVEN|CFCR_PENAB|CFCR_7BITS);

      outportb(tty.port+IER, 0);
      outportb(tty.port+MCR, MCR_DTR|MCR_RTS|MCR_IENABLE);

      for(i=0; i<166667; i++)asm("inb $0x80");       // wait .1 sec
      outportb(tty.port+IER, IER_ERXRDY|IER_ETXRDY); // enable TX & RX intr
      for(i=0; i<166667; i++)asm("inb $0x80");       // wait .1 sec

      for(j=0; j<3; j++) {                           // clear 3 lines
         outportb(tty.port, 'V');
         for(i=0; i<83333; i++)asm("inb $0x80");     // wait .5 sec should do
         outportb(tty.port, '\n');
         for(i=0; i<83333; i++)asm("inb $0x80");
         outportb(tty.port, '\r');
         for(i=0; i<83333; i++)asm("inb $0x80");
      }
      inportb(tty.port);                             // get 1st key PROCOMM logo
      for(i=0; i<83333; i++)asm("inb $0x80");        // wait .5 sec
   }

int main(void) {               // OS starts
   //do the boot strap things 1st
   BootStrap();

   SpawnSR(Idle);
   SpawnSR(Login);
   run_pid = IDLE;
   //call Loader() to load the trapframe of Idle
   set_cr3(pcb[run_pid].Dir);
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

   //if '$' key on target PC is pressed, goto the GDB prompt
   KBSR();
   //call Scheduler() to change run_pid if needed
   Scheduler();
   //Call Loader() to load the trapframe of the selected process
   set_cr3(pcb[run_pid].Dir);
	 Loader(pcb[run_pid].tf_p);
}
