// tools.c, 159

//this .c code needs to include spede.h, const-type.h, and ext-data.h
#include "spede.h"
#include "const-type.h"
#include "ext-data.h"

/*code a QueEmpty() function that checks whether a queue (located by a
given pointer) is emptyr; returns 1 if yes, or 0 if not*/

int QueEmpty(que_t *q){
   if (q->tail == 0) return 1;
   return 0;
}

/*similarly, code a QueFull() function to check for being full or not*/
int QueFull(que_t *q){
   if (q->tail == (QUE_MAX)) return 1;
   return 0;
}

/*code a DeQue() function that dequeues the 1st number in the queue (given
by a pointer); if empty, return constant NONE (-1)*/
int DeQue(que_t *q){
   int x;
   int head_num;
   if (QueEmpty(q))return NONE;
   head_num = q->que[0];
   for (x = 0; x < q->tail-1; x++){
      q->que[x] = q->que[x+1];
   }
   q->tail--;
   q->que[q->tail] = NONE;
   return head_num;
}

/*code an EnQue() function given a number and a queue (by a pointer), it
appends the number to the tail of the queue, or shows an error message
and go into the GDB:
      cons_printf("Panic: queue is full, cannot EnQue!\n");
      breakpoint();*/
void EnQue(int num, que_t *q){
   if (QueFull(q)){
      cons_printf("Panic: queue is full, cannot EnQue!\n");
      breakpoint();
   }
   q->que[q->tail] = num;
   q->tail++;   
}

/*code a Bzero() function to clear a memory region (by filling with NUL
characters), the beginning of the memory location will be given via a
character pointer, and the size of the memory will be given by an unsigned
int 'max'*/
void Bzero(char *start, unsigned int max){
	int x;
	for (x = 0; x < max; x++){
	   start[x] = '\0';
	}
}

/*code a MemCpy() function to copy a memory region located at a given
character pointer 'dst,' from the starting location at a given character
pointer 'src,' the size will also be given as an unsigned integer 'max'*/
void MemCpy(char *dst, char *src, unsigned int max){
	int x;
	for (x = 0; x < max; x++){
	   dst[x] = src[x];
	}
}
