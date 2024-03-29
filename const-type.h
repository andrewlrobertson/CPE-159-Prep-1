// const-type.h, 159, needed constants & types

#ifndef _CONST_TYPE_           // to prevent name mangling recursion
#define _CONST_TYPE_           // to prevent name redefinition

#define TIMER_EVENT 32         // timer interrupt signal code
#define PIC_MASK_REG 0x21      // I/O loc # of PIC mask
#define PIC_CONT_REG 0x20      // I/O loc # of PIc control
#define TIMER_SERVED_VAL 0x60  // control code sent to PIC
#define VGA_MASK_VAL 0x0f00    // bold face, white on black

#define SIGCHLD 17
#define SIGCONT 18
#define TIME_MAX 310            // max timer count, then rotate process
#define PROC_MAX 20             // max number of processes
#define STACK_MAX 4096          // process stack in bytes
#define QUE_MAX 20              // capacity of a process queue
#define STR_MAX 20

#define VIDEO_MUTEX 0
#define UNLOCKED 0
#define LOCKED 1
#define NONE -1                 // to indicate none
#define IDLE 0                  // Idle thread PID 0
#define DRAM_START 0xe00000     // 14 MB

#define SYSCALL_EVENT 128       // syscall event identifier code, phase2
#define SYS_GET_PID 129         // different types of syscalls
#define SYS_GET_TIME 130
#define SYS_SLEEP 131
#define SYS_WRITE 132
#define SYS_FORK 133
#define SYS_SET_CURSOR 134
#define SYS_GET_RAND 135
#define SYS_LOCK_MUTEX 136
#define SYS_UNLOCK_MUTEX 137
#define SYS_EXIT 138
#define SYS_WAIT 139
#define SYS_SIGNAL 140
#define SYS_KILL 141
#define SYS_READ 142

#define VIDEO_START (unsigned short *)0xb8000
#define VIDEO_END ((unsigned short *)0xb8000 + 25 * 80)

#define SYS_VFORK 143   //creation of a virtual-space running process
#define PAGE_MAX 100    //OS has 100 DRAM pages to dispatch
#define PAGE_SIZE 4096  //each page size in bytes
#define G1 0x40000000   //virtual space starts
#define G2 0x80000000   //vitual space ends (1 less byte)
#define PRESENT 0x01    //page present flag
#define RW 0x02         //page is both read & writable
#define RO 0x00         //page is read only

#define CONSOLE 100             // phase9, for STDIN of Idle
#define TTY 200                 // for STDIN of Shell and its children
#define TTY_EVENT 35            // TTY0/2, use 36 for TTY1
#define PIC_MASK_VAL ~0x09      // new mask: ~0..01001
#define TTY_SERVED_VAL 0x63     // also for COM4, 0x64 for COM3
#define TTY0 0x2f8              // TTY1 0x3e8, TTY2 0x2e8

typedef void (*func_p_t)(void); // void-return function pointer type

typedef enum {AVAIL, READY, RUN, SLEEP, SUSPEND, WAIT, ZOMBIE, IO_WAIT} state_t;

typedef struct{
   unsigned int
      edi, esi, ebp, esp, ebx, edx, ecx, eax, event, eip, cs, efl;
} tf_t;

typedef struct{
   state_t state;
   tf_t *tf_p;
   unsigned int time_count;
   unsigned int total_time;
   unsigned int wake_time;
   unsigned int ppid;
   unsigned Dir;
   func_p_t signal_handler[32];
   int STDOUT;
   int STDIN;
} pcb_t;

typedef struct{
   int pid;
   union{
      unsigned addr; // its byte address (starting DRAM 0xe00000)
      char *content; // use its addr as ptr to content (bytes)
      unsigned *entry; // use its addr as an 'entry' array
   } u;
} page_t;

typedef struct{
   int tail;
   int que[QUE_MAX];
} que_t;

typedef struct{
   int lock;
   que_t suspend_que;
} mutex_t;

typedef struct{
   que_t buffer;
   que_t wait_que;
} kb_t;

typedef struct{
   char *dsp_str;           // string ptr (to display these chars)
   que_t dsp_wait_que;      // processes wait for strings to get displayed
   int port;                // 0x2f8 (COM2), 0x3e8 (COM3), 0x2e8 (COM4)
   char *kb_str;            // string ptr (to get terminal input)
   que_t kb_wait_que;       // processes wait for terminal input strings
   que_t echo;              // input queued here to echo back to terminal
} tty_t;

#endif                          // to prevent name mangling
