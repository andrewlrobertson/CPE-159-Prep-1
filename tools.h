// tools.h, 159

#ifndef _TOOLS_
#define _TOOLS_

#include "const-type.h" // need definition of 'que_t' below

int QueEmpty(que_t *q);
int QueFull(que_t *q);
void EnQue(int num, que_t *q);
int DeQue(que_t *q);
void Bzero(char *start, unsigned int max);
void MemCpy(char *dst, char *src, unsigned int max);
void Number2Str(int x, char *str);
#endif

