#include <stdlib.h>
#include <ucontext.h>
#include <stdio.h>
#include <unistd.h>

#define STACK_SIZE 8192
#define true 1
#define false 0

typedef void * MyThread;
typedef void * MySemaphore;
typedef int bool;

struct lT;
typedef struct lT * threadVector;

typedef struct threads
{
	struct threads* parent;
	ucontext_t value;
	threadVector blockingChildren;
	bool completed;

} THREAD;

THREAD *currentThread;
ucontext_t threadHandlerContext;

struct lT
{
	THREAD * val;
	threadVector next;
	int count;
};


typedef struct semaphores
{
	int num;
	threadVector blockedThreads;
} SEMAPHORE;



threadVector threadList;
threadVector readyQueue;

int push_back(threadVector head, THREAD *T)
{
	threadVector temp = (threadVector)malloc( sizeof(struct lT) );
	
	if(temp == NULL) return -1;

	head->count += 1;

	while(head->next !=NULL) head = head->next;
	
	head->next = temp;
	temp->val = T;
	temp->next = NULL;
	temp->count = -1;

	return 0;

}

THREAD * front (threadVector head)
{
	if(head->next == NULL) return NULL;
	return (head->next->val); 
}

int pop(threadVector head)
{
	threadVector temp;

	if(head == NULL) return -1;
	temp = head->next;
	if(head->count == 0) return -1;
	
	head->next = temp->next;
	head->count -= 1;
	free(temp);
	
	return 0;
}

bool empty(threadVector head)
{
	if( head->count == 0) return true;
	return false;
}

int size(threadVector head)
{
	if(head == NULL) return 0;
	return head->count;
}

int erase(threadVector head)
{
	threadVector temp, prev;
	prev = temp = head;

	if(head == NULL) return -1;

	while(temp->next != NULL)
	{
		temp= temp->next;
		if(temp->val == currentThread )
		{
			head->count -= 1;
			prev->next = temp->next;
			free ( temp );
			return 0;
		}
		prev = temp;
	}
	return -1;
}

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
	SEMAPHORE * temp = (SEMAPHORE*) malloc( sizeof(SEMAPHORE) );
	temp->num = initialValue;
	temp->blockedThreads = (threadVector) malloc( sizeof(struct lT) );
	if(temp->blockedThreads == NULL) return NULL;
	temp->blockedThreads->val = NULL;
	temp->blockedThreads->next = NULL;
	temp->blockedThreads->count = 0;

	return (MySemaphore)temp;
}

void MySemaphoreSignal(MySemaphore sem)
{
	if(sem == NULL) return ;

	SEMAPHORE * temp = (SEMAPHORE *)sem;
	temp->num++;

	if(temp->num <= 0)
	{
		if( size(temp->blockedThreads) != 0)
		{
			push_back(readyQueue, front(temp->blockedThreads) );
			pop(temp->blockedThreads);
		}
	}
}

void MySemaphoreWait(MySemaphore sem)
{
	if(sem == NULL) return ;

	SEMAPHORE * temp = (SEMAPHORE *)sem;
	(temp->num)--;
	if( temp->num < 0)
	{
		if(temp->blockedThreads == NULL)
		{
			temp->blockedThreads = (threadVector) malloc(sizeof(struct lT));
			if(temp->blockedThreads == NULL) return ;
			temp->blockedThreads->count = 0;
			temp->blockedThreads->next = NULL;
			temp->blockedThreads->val = NULL;
		}
		push_back( temp->blockedThreads,  currentThread );
		swapcontext( &(currentThread->value), &threadHandlerContext );
	}
}

int MySemaphoreDestroy(MySemaphore sem)
{
	if(sem == NULL) return -1;
	
	SEMAPHORE * temp = (SEMAPHORE *)sem;
	if( size(temp->blockedThreads) == 0 )
	{
		// cout << "semaphore delete!\n";
 		free(temp);
		return 0;
	}
	else
	{
		// cout << "Illegal semaphore delete!\n";
		return -1;
	}
}

void threadHandler()
{
	do
	{
		currentThread = front(readyQueue);
		pop(readyQueue);
		// cout<< "Handler"currentThread->no<<endl;
		swapcontext(&threadHandlerContext, &(currentThread->value) );
	}while(!empty(readyQueue));
}

void MyThreadExit()
{
	//	cout<<"In Exit : "<<currentThread->no<<endl;
	if(!InitFlag)
	{
		// cout<<"Illegal MyThreadCreate call!\n";
		return ;
	}

	THREAD * temp = currentThread->parent;

	if( temp != NULL)
	{
		switch( size( temp->blockingChildren ) )
		{
			case 0:
				// cout<<"No blocking children\n";
				break;
			case 1:
			{
				if( front( temp->blockingChildren ) == currentThread )
				{
					push_back(readyQueue, temp);
					pop( temp->blockingChildren );
				}
				break;
			}
			default:
			{	
				if( erase( temp->blockingChildren ) == -1 ) return;
	    		break;
			} 
		}
	}	
	currentThread->completed = true;
	free (currentThread->value.uc_stack.ss_sp) ;
}

void MyThreadJoinAll()
{
	int liveChildrenCount = 0;
	threadVector temp = threadList;

	if(!InitFlag)
	{
		// cout<<"Illegal MyThreadCreate call!\n";
		return ;
	}
	
	while(temp->next != NULL)
	{
		temp = temp->next;
		if( currentThread == (temp->val)->parent && (temp->val)->completed == false )
		{
			if(currentThread->blockingChildren == NULL)
			{
				currentThread->blockingChildren = (threadVector) malloc( sizeof(struct lT) );
				if( currentThread->blockingChildren == NULL ) return ;
				currentThread->blockingChildren->count = 0;
				currentThread->blockingChildren->next = NULL;
				currentThread->blockingChildren->val = NULL;
			}
			push_back((currentThread->blockingChildren), temp->val);
			liveChildrenCount++;
		}
	}	
	
	if(liveChildrenCount == 0) return ;
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
	if(currentThread->blockingChildren == NULL)
	{
		currentThread->blockingChildren = (threadVector) malloc( sizeof(struct lT) );
		if( currentThread->blockingChildren == NULL ) return ;
		currentThread->blockingChildren->count = 0;
		currentThread->blockingChildren->next = NULL;
		currentThread->blockingChildren->val = NULL;
	}
	push_back((currentThread->blockingChildren), (THREAD *)T );
	swapcontext( &(currentThread->value), &threadHandlerContext );
}

void MyThreadYield(void)
{
	if(!InitFlag)
	{
		// cout<<"Illegal MyThreadCreate call!\n";
		return ;
	}
	push_back(readyQueue, currentThread );
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

	newThread = (THREAD *) malloc(sizeof(THREAD));
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

    newThread->blockingChildren = (threadVector) malloc(sizeof(threadVector));
    if( newThread->blockingChildren == NULL ) return NULL;
    newThread->blockingChildren->count = 0;
    newThread->blockingChildren->next = NULL;
    newThread->blockingChildren->val = NULL; 
    
    makecontext( &(newThread->value), (void(*)(void))start_func, 1, args );
	push_back(threadList, newThread);
	push_back(readyQueue, newThread);
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
	threadList = (threadVector) malloc( sizeof(struct lT) );
	if(threadList == NULL) return ;
	threadList->count = 0;
	threadList->val = NULL;
	threadList->next = NULL;

	readyQueue = (threadVector) malloc( sizeof(struct lT) );
	if(readyQueue == NULL) return ;
	readyQueue->count = 0;
	readyQueue->val = NULL;
	readyQueue->next = NULL;

	newThread = (THREAD *) malloc(sizeof(THREAD));
	if( newThread == NULL )  
		return ;
    newThread->parent = NULL;
	getcontext( &(newThread->value) );
	newThread->value.uc_link = &threadHandlerContext;
	newThread->value.uc_stack.ss_sp =  malloc(STACK_SIZE);
    if( newThread->value.uc_stack.ss_sp == NULL ) return ;
    newThread->value.uc_stack.ss_size = STACK_SIZE;
    newThread->value.uc_stack.ss_flags = 0;   
    
    newThread->completed = false;
    
    
    newThread->blockingChildren = (threadVector) malloc(sizeof(threadVector));
    if( newThread->blockingChildren == NULL ) return ;
    newThread->blockingChildren->count = 0;
    newThread->blockingChildren->next = NULL;
    newThread->blockingChildren->val = NULL;

    makecontext( &(newThread->value), (void(*)(void))start_func, 1, args );
	
	push_back(threadList, newThread);
	push_back(readyQueue, newThread);
	
	threadHandler();
}
