#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "mythread.h"

int n, m;
int yield = 0;
int join = 0;

void t2(void * who)
{
  printf("t2 %d start\n", (intptr_t)who);
  MyThreadExit();
}

void t1(void *);

int makeThreads(char *me, void (*t)(void *), int many)
{
  MyThread T;
  int i;
  for (i = 0; i < many; i++) {
    printf("%s create %d\n", me, i);
    T = MyThreadCreate(t, (void *)i);
    if (yield)
      MyThreadYield();      
  }
  if (join)
    MyThreadJoin(T);
}

void t1(void * who_)
{
  char me[16];
  int who = (intptr_t)who_;
  sprintf(me, "t1 %d", who);
  printf("%s start\n", me);
  makeThreads(me, t2, m);
  printf("t1 %d end\n", who);
  MyThreadExit();
}

void t0(void * dummy)
{
  printf("t0 start\n");
  makeThreads("t0", t1, n);
  printf("t0 end\n");
  MyThreadExit();
}

int main(int argc, char *argv[])
{
  if (argc != 5) {
    return -1;
  }
  n = atoi(argv[1]);
  m = atoi(argv[2]);
  yield = atoi(argv[3]);
  join = atoi(argv[4]);

  MyThreadInit(t0, NULL);
}
