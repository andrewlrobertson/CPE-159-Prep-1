// tools.c, 159

//this .c code needs to include spede.h, const-type.h, and ext-data.h
#include "spede.h"
#include "const-type.h"
#include "ext-data.h"

int QueEmpty(que_t *q){
   if (q->tail == 0) return 1;
   return 0;
}

int QueFull(que_t *q){
   if (q->tail == (QUE_MAX)) return 1;
   return 0;
}

int DeQue(que_t *q){
   int x;
   int head_num;
   if (QueEmpty(q))return NONE;
   head_num = q->que[0];
   for (x = 0; x < q->tail-1; x++){
      q->que[x] = q->que[x+1];
   }
   q->tail--;
   return head_num;
}

void EnQue(int num, que_t *q){
   if (QueFull(q)){
      cons_printf("Panic: queue is full, cannot EnQue!\n");
      breakpoint();
   }
   q->que[q->tail] = num;
   q->tail++;
}

void Bzero(char *start, unsigned int max){
	int x;
	for (x = 0; x < max; x++){
	   start[x] = '\0';
	}
}

void MemCpy(char *dst, char *src, unsigned int max){
	int x;
	for (x = 0; x < max; x++){
	   dst[x] = src[x];
	}
}

void Number2Str(int x, char *str) {
	int i = 0;
  int stop, offset, end;
  char swap;

	while(x != 0){
		str[i] = x % 10 + 48;
		x = x / 10;
		i++;
		if(x == 0){
		str[i] = '\0';
		}
	}

  if (i>1){
     end = i-1;
     if(i%2==0){
       stop = (i/2)-1;
     }
     else{
       stop = (i/2);
     }

     for(offset = 0; offset<=stop; offset++){
       swap = str[offset];
       str[offset] = str[end-offset];
       str[end-offset] = swap;
     }
  }
}
