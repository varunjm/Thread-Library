#include <iostream>
#include <ucontext.h>
#include <stdio.h>
#include <unistd.h>
#include <vector>

#define STACK_SIZE 8192

typedef void * MyThread;

using namespace std;

typedef struct threads
{
	struct threads* parent;
	ucontext_t value;
} THREAD;

vector<THREAD *> threadList;
vector<THREAD *> readyQueue;

THREAD current;

void show(void *a)
{
	cout<<"The number is : "<<*(float *)a << endl;
}

MyThread MyThreadCreate ( void(*start_func)(void *), void *args)
{ 
	THREAD *newThread;

	newThread = new THREAD;
	cout<<"Creating thread! "<<endl;

    newThread->parent = &(current);
	getcontext( &(newThread->value) );
	newThread->value.uc_link = &(current.value);
	newThread->value.uc_stack.ss_sp = new stack_t [STACK_SIZE];
    newThread->value.uc_stack.ss_size = STACK_SIZE;
    newThread->value.uc_stack.ss_flags = 0;   

    
	
	makecontext( &(newThread->value), (void(*)(void))start_func, 1, args );
	threadList.push_back(newThread);
	readyQueue.push_back(newThread);

	return &(newThread->value);
}

int main()
{
	float num1 = 69.01;
	MyThread temp;

	getcontext( &(current.value) );
	temp = MyThreadCreate(show, &num1 );
	
	current.value.uc_stack.ss_sp = new stack_t [STACK_SIZE];
    current.value.uc_stack.ss_size = STACK_SIZE;
    current.value.uc_stack.ss_flags = 0;        
    
    swapcontext( &(current.value), (ucontext_t *)temp );
	
	cout<<"End of trial\n";
	return 0;
}