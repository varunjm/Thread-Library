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
typedef struct threads
{
	struct threads* parent;
	ucontext_t value;
	int no;
	list <struct threads*> blockingChildren;
	bool completed;

} THREAD;

vector<THREAD *> threadList;
queue<THREAD *> readyQueue;
list<THREAD *> blockedList;

THREAD *currentThread;
ucontext_t threadHandlerContext;

bool InitFlag = false;

void show3(void *);
void show2(void *);
void show(void *);
void threadHandler();
MyThread MyThreadCreate ( void(*)(void *), void *);
void MyThreadInit ( void(*)(void *), void *);
void MyThreadYield (void);
int MyThreadJoin(MyThread);
void MyThreadJoinAll();
void MyThreadExit();

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
	THREAD * temp;

	temp = currentThread->parent;

	if( temp != NULL)
	{
		switch( (temp->blockingChildren).size() )
		{
			case 0:
				// cout<<"No blocking children\n";
				break;
			case 1:
			{
				if( (temp->blockingChildren).front() == currentThread )
				{
					readyQueue.push(temp);
					(temp->blockingChildren).erase((temp->blockingChildren).begin());
				}
				break;
			}
			default:
			{	
				for (list<struct threads *>::iterator it=(temp->blockingChildren).begin() ; it != (temp->blockingChildren).end(); ++it)
	    			if( *it == currentThread )
	    			{
	    				(temp->blockingChildren).erase(it);
	    				break;
	    			}
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

	for(vector<struct threads *>::iterator it=threadList.begin() ; it != threadList.end(); ++it)
	{
		if( currentThread == (*it)->parent && (*it)->completed == false )
		{
			(currentThread->blockingChildren).push_back(*it);
			liveChildrenCount++;
		}
	}
	if(liveChildrenCount == 0)
		return ;
	// cout<<"Thread join on "<<liveChildrenCount<<" children\n";
	swapcontext( &(currentThread->value), &threadHandlerContext );

}

int MyThreadJoin(MyThread T)
{
	if( ((THREAD *)T)->parent != currentThread)
	{
		return -1;
	}	

	if( ((THREAD *)T)->completed == true )
	{
	//		cout<<"Invoking completed child\n";
		return 0;
	}
	//	cout<<"Correct invocation: "<<((THREAD *)T)->no<<endl;
	(currentThread->blockingChildren).push_back( (THREAD *)T );
	swapcontext( &(currentThread->value), &threadHandlerContext );
}

void MyThreadYield(void)
{
	readyQueue.push( currentThread );
	swapcontext( &(currentThread->value), &threadHandlerContext );
}

MyThread MyThreadCreate ( void(*start_func)(void *), void *args)
{ 
	THREAD * newThread = new THREAD;
	
	newThread->parent = currentThread;
	getcontext( &(newThread->value) );
	newThread->value.uc_link = &threadHandlerContext;
	newThread->value.uc_stack.ss_sp =  malloc(STACK_SIZE);
    newThread->value.uc_stack.ss_size = STACK_SIZE;
    newThread->value.uc_stack.ss_flags = 0;   
    newThread->no = *(int *)args;
    
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
		cout<<"Illegal Init call!\n";
		return;
	}

	InitFlag = true;

	newThread = new THREAD;
    newThread->parent = NULL;
	getcontext( &(newThread->value) );
	newThread->value.uc_link = &threadHandlerContext;
	newThread->value.uc_stack.ss_sp =  malloc(STACK_SIZE);
    newThread->value.uc_stack.ss_size = STACK_SIZE;
    newThread->value.uc_stack.ss_flags = 0;   
    newThread->no = *(int *)args;
   
    newThread->completed = false;
    
    makecontext( &(newThread->value), (void(*)(void))start_func, 1, args );
	threadList.push_back(newThread);
	readyQueue.push(newThread);

	threadHandler();
}

void show2(void *a)
{
	MyThreadYield();
	cout<<"In show2\n";
	MyThreadExit();
}
void show3(void *a)
{
	cout<<"In show3\n";
	MyThreadExit();
}
void show(void *a)
{
	int num = 2;
	
	cout<<"In show\n";
	MyThreadCreate(show2, &num);
	MyThreadCreate(show3, &num);
	// MyThreadYield();
	MyThreadJoinAll();
	cout<<"Returned after join\n";
	MyThreadExit();
}

int main()
{
	int num1 = 1;
	
	MyThreadInit( show, &num1);
	cout<<"End of trial\n";
	return 0;
}
