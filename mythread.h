
typedef void * MyThread;
typedef void * MySemaphore;

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