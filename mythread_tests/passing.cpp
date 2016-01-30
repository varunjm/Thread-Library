#include <stdio.h>
#include "mythread.h"

int mode = 0;

void t0(void * n)
{
  MyThread T;

  int n1 = (int)n; 
  printf("t0 start %d\n", n1);

  int n2 = n1 -1 ;
  if (n1 > 0) {
    printf("t0 create\n");
    T = MyThreadCreate(t0, (void *)n2);
    if (mode == 1)
      MyThreadYield();
    else if (mode == 2)
      MyThreadJoin(T);
  }
  printf("t0 end\n");
  MyThreadExit();
}

int main(int argc, char *argv[])
{
  int count; 
  
  if (argc < 2 || argc > 3)
    return -1;
  count = atoi(argv[1]);
  if (argc == 3)
    mode = atoi(argv[2]);
  MyThreadInit(t0, (void *)count);
}
