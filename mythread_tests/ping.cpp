#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "mythread.h"
int n;

void t1(void * who)
{
  int i;

  printf("t%d start\n", (intptr_t)who);
  for (i = 0; i < n; i++) {
    printf("t%d yield\n", (intptr_t)who);
     MyThreadYield();
  }
  printf("t%d end\n", (intptr_t)who);
  MyThreadExit();
}

void t0(void * dummy)
{
  MyThreadCreate(t1, (void *)1);
  t1(0);
}

int main(int argc, char *argv[])
{
  if (argc != 2)
    return -1;
  n = atoi(argv[1]);
  MyThreadInit(t0, 0);
}
