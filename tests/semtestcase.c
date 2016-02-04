#include <stdio.h>
#include <stdlib.h>
#include "mythread.h"

MySemaphore stemp;

void show2(void *a)
{
	printf("In show2\n");
	MySemaphoreWait(stemp);
	MyThreadYield();
	MySemaphoreSignal(stemp);
	printf("Show2 exiting\n");
	MyThreadExit();
}
void show3(void *a)
{
	printf("In show3\n");
	MySemaphoreWait(stemp);
	MySemaphoreSignal(stemp);
	printf("Show3 exiting\n");
	MyThreadExit();
}
void show(void *a)
{
	int num = 2;

	printf("In show\n");
	MySemaphoreWait(stemp);
	MyThreadCreate(show2, &num);
	MyThreadCreate(show3, &num);
	// MyThreadYield();
	MyThreadJoinAll();
	MySemaphoreSignal(stemp);
	printf("Returned after join\n");
	MyThreadExit();
}

int main()
{
	int num1 = 1;
	
	stemp = MySemaphoreInit(2);
	MyThreadInit( show, &num1);
	MySemaphoreDestroy(stemp);
	printf("End of trial\n");
	return 0;
}
