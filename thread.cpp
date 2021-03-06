#include <iostream>
#include <cstdlib>
#include <ucontext.h>
#include <stdio.h>
#include <unistd.h>
#include <vector>
#include <queue>
#include <stack>
#include <list>
#define STACK_SIZE 8192

using namespace std;

typedef void * MyThread;
typedef void * MySemaphore;

typedef struct threads
{
	struct threads* parent;
	ucontext_t value;
	list <struct threads*> blockingChildren;
	bool completed;

} THREAD;

typedef struct semaphores
{
	int num;
	queue <struct threads*> blockedThreads;
} SEMAPHORE;

vector<THREAD *> threadList;
queue<THREAD *> readyQueue;

list<SEMAPHORE *> semaphoreList;

THREAD *currentThread;
ucontext_t threadHandlerContext;

bool InitFlag = false;

void threadHandler();
MyThread MyThreadCreate ( void(*)(void *), void *);
void MyThreadInit ( void(*)(void *), void *);
void MyThreadYield (void);
int MyThreadJoin(MyThread);
void MyThreadJoinAll();
void MyThreadExit();
MySemaphore MySemaphoreInit(int initialValue);
void MySemaphoreSignal(MySemaphore sem);
void MySemaphoreWait(MySemaphore sem);
int MySemaphoreDestroy(MySemaphore sem);

MySemaphore MySemaphoreInit(int initialValue)
{
	SEMAPHORE * temp = new SEMAPHORE;
	temp->num = initialValue;
	semaphoreList.push_back(temp);
	return (MySemaphore)temp;
}

void MySemaphoreSignal(MySemaphore sem)
{
	if(sem == NULL) return;
	SEMAPHORE * temp = (SEMAPHORE *)sem;
	if(temp->num++ < 0)
	{
		readyQueue.push(temp->blockedThreads.front());
		temp->blockedThreads.pop();
	}
}

void MySemaphoreWait(MySemaphore sem)
{
	if(sem == NULL) return;
	SEMAPHORE * temp = (SEMAPHORE *)sem;
	if(--temp->num < 0)
	{
		(temp->blockedThreads).push( currentThread );
		swapcontext( &(currentThread->value), &threadHandlerContext );
	}
}

int MySemaphoreDestroy(MySemaphore sem)
{
	if(sem == NULL) return -1;
	SEMAPHORE * temp = (SEMAPHORE *)sem;
	if(temp->blockedThreads.empty())
	{
		// cout << "semaphore delete!\n";
 		delete temp;
		return 0;
	}
	// cout << "Illegal semaphore delete!\n";
	return -1;
}

void threadHandler()
{
	do
	{
		currentThread = readyQueue.front();
		readyQueue.pop();
		// cout<< "Handler"currentThread->no<<endl;
		swapcontext(&threadHandlerContext, &(currentThread->value) );
	}while(!readyQueue.empty());
	//cout<<"Out of here!\n";
}

void MyThreadExit()
{
	//	cout<<"In Exit : "<<currentThread->no<<endl;
	if(!InitFlag)
	{
		// cout<<"Illegal MyThreadCreate call!\n";
		return;
	}

	THREAD *temp;
	if(temp = currentThread->parent)
	{
		temp->blockingChildren.remove(currentThread);
		if((temp->blockingChildren).empty())
			readyQueue.push(temp);
	}	
	currentThread->completed = true;
	free (currentThread->value.uc_stack.ss_sp) ;
}

void MyThreadJoinAll()
{
	int liveChildrenCount = 0;

	if(!InitFlag)
	{
		// cout<<"Illegal MyThreadCreate call!\n";
		return;
	}
	for(auto &thread : threadList)
	{
		if( currentThread == thread->parent && thread->completed == false )
		{
			(currentThread->blockingChildren).push_back(thread);
			liveChildrenCount++;
		}
	}
	if(liveChildrenCount == 0)
		return;
	// cout<<"Thread join on "<<liveChildrenCount<<" children\n";
	swapcontext( &(currentThread->value), &threadHandlerContext );
}

int MyThreadJoin(MyThread T)
{
	if(!InitFlag)
	{
		// cout<<"Illegal MyThreadCreate call!\n";
		return -1;
	}
	if( ((THREAD *)T)->parent != currentThread)
		return -1;
	
	if( ((THREAD *)T)->completed == true )
	//		cout<<"Invoking completed child\n";
		return 0;
	
	//	cout<<"Correct invocation: "<<((THREAD *)T)->no<<endl;
	
	(currentThread->blockingChildren).push_back((THREAD *)T);
	swapcontext(&(currentThread->value), &threadHandlerContext);
	return 0;
}

void MyThreadYield(void)
{
	if(!InitFlag)
	{
		// cout<<"Illegal MyThreadCreate call!\n";
		return;
	}
	readyQueue.push( currentThread );
	swapcontext( &(currentThread->value), &threadHandlerContext );
}

MyThread MyThreadCreate ( void(*start_func)(void *), void *args)
{ 
	THREAD * newThread;

	if(!InitFlag)
	{
		// cout<<"Illegal MyThreadCreate call!\n";
		return NULL;
	}

	newThread = new THREAD;
	if( newThread == NULL )  
		return NULL;
   
	newThread->parent = currentThread;
	getcontext( &(newThread->value) );
	newThread->value.uc_link = &threadHandlerContext;
	newThread->value.uc_stack.ss_sp =  malloc(STACK_SIZE);
	if( newThread->value.uc_stack.ss_sp == NULL ) return NULL;
    newThread->value.uc_stack.ss_size = STACK_SIZE;
    newThread->value.uc_stack.ss_flags = 0;   
    newThread->completed = false;
    
    makecontext( &(newThread->value), (void(*)(void))start_func, 1, args );
	threadList.push_back(newThread);
	readyQueue.push(newThread);
	return (MyThread)newThread;
}

void MyThreadInit ( void(*start_func)(void *), void *args)
{ 
	THREAD *newThread;

	if(InitFlag)
	{
		// cout<<"Illegal Init call!\n";
		return;
	}

	InitFlag = true;

	newThread = new THREAD;
	if( newThread == NULL )  
		return;
    newThread->parent = NULL;
	getcontext( &(newThread->value) );
	newThread->value.uc_link = &threadHandlerContext;
	newThread->value.uc_stack.ss_sp =  malloc(STACK_SIZE);
    if( newThread->value.uc_stack.ss_sp == NULL ) return;
    newThread->value.uc_stack.ss_size = STACK_SIZE;
    newThread->value.uc_stack.ss_flags = 0;   
    
    newThread->completed = false;
    
    makecontext( &(newThread->value), (void(*)(void))start_func, 1, args );
	threadList.push_back(newThread);
	readyQueue.push(newThread);

	threadHandler();
}