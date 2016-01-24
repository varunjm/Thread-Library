#include <iostream>
#include <ucontext.h>
#include <stdio.h>
#include <unistd.h>
#include <vector>
#include <queue>
#include <stack>
#define STACK_SIZE 8192

using namespace std;

typedef void * MyThread;
typedef struct threads
{
	struct threads* parent;
	ucontext_t value;
} THREAD;

vector<THREAD *> threadList;
queue<THREAD *> readyQueue;

THREAD current;

bool InitFlag = false;
/*
void show(void *a)
{
	cout<<"The number is : "<<*(float *)a << endl;
	MyThread t = MyThreadCreate( show2, a);

	currentStack.push(current.value);
	current.value.uc_stack.ss_sp = new stack_t [STACK_SIZE];
    current.value.uc_stack.ss_size = STACK_SIZE;
    current.value.uc_stack.ss_flags = 0; 
	getcontext(&current.value);

	swapcontext( &(current.value), (ucontext_t *)t );

	current
}
*/
void show2(void *a)
{
	
	cout<<"The number is also : "<<*(float *)a << endl;
}

MyThread MyThreadCreate ( void(*start_func)(void *), void *args)
{ 
	THREAD *newThread;

	newThread = new THREAD;
	
    newThread->parent = &(current);
	getcontext( &(newThread->value) );
	newThread->value.uc_link = &(current.value);
	newThread->value.uc_stack.ss_sp = new stack_t [STACK_SIZE];
    newThread->value.uc_stack.ss_size = STACK_SIZE;
    newThread->value.uc_stack.ss_flags = 0;   

    makecontext( &(newThread->value), (void(*)(void))start_func, 1, args );
	threadList.push_back(newThread);
	readyQueue.push(newThread);

	return &(newThread->value);
}

MyThread MyThreadInit ( void(*start_func)(void *), void *args)
{ 
	THREAD *newThread;

	InitFlag = true;

	
	newThread = new THREAD;
    newThread->parent = NULL;
	getcontext( &(newThread->value) );
	newThread->value.uc_link = &(current.value);
	newThread->value.uc_stack.ss_sp = new stack_t [STACK_SIZE];
    newThread->value.uc_stack.ss_size = STACK_SIZE;
    newThread->value.uc_stack.ss_flags = 0;   

    makecontext( &(newThread->value), (void(*)(void))start_func, 1, args );
	threadList.push_back(newThread);
	
	swapcontext( &(current.value) , &(newThread->value) );
}

int main()
{
	float num1 = 69.01;
	MyThread temp;

	temp = MyThreadCreate(show2, &num1 );
	
	current.value.uc_stack.ss_sp = new stack_t [STACK_SIZE];
    current.value.uc_stack.ss_size = STACK_SIZE;
    current.value.uc_stack.ss_flags = 0;        
    getcontext(&current.value);
    
	swapcontext( &(current.value), (ucontext_t *)temp );

	cout<<"End of trial\n";
	return 0;
}