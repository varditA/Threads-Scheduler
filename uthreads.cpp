#include <list>
#include <cstdio>
#include <iostream>
#include <setjmp.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include "uthreads.h"

/***************************** start of black box code ****************************/

#ifdef __x86_64__
/* code for 64 bit Intel arch */

typedef unsigned long address_t;

#define JB_SP 6
#define JB_PC 7

/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t translate_address(address_t addr)
{
    address_t ret;
    asm volatile("xor    %%fs:0x30,%0\n"
            "rol    $0x11,%0\n"
    : "=g" (ret)
    : "0" (addr));
    return ret;
}

#else
/* code for 32 bit Intel arch */

typedef unsigned int address_t;
#define JB_SP 4
#define JB_PC 5

/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t translate_address(address_t addr)
{
    address_t ret;
    asm volatile("xor    %%gs:0x18,%0\n"
		"rol    $0x9,%0\n"
                 : "=g" (ret)
                 : "0" (addr));
    return ret;
}
#endif

/***************************** end of black box code ****************************/

using namespace std;

/* map with current threads
 * set with the feed indexes
 * if the set is empty:
 *  check if the max in the map is smaller than MAX_THREAD_NUM */

typedef struct
{
    int id;
    int quantumsNum;
    char *state; //TODO what is this for?
    char stack[STACK_SIZE];
    sigjmp_buf env;
} thread;

int quantumUsecs;                   /* the length of a quantum in micro-seconds */
int totalNumQuantum;                /* the total number of quantum */
int threadsNum;                     /* how many threads exist now */
int maxThreadIndex;
int timer;                      /* how many ms are left for this thread to run */

thread *currentRunning;              /* the thread that running right now */
list<thread *> readyThreads;    /* a list of pointers to READY threads */

int mainLoop();                     /* the timer loop */
int switchThreads(thread& newThread);/* switches between threads */


/*
* Description: This function initializes the thread library.
* You may assume that this function is called before any other thread library
* function, and that it is called exactly once. The input to the function is
* the length of a quantum in micro-seconds. It is an error to call this
* function with non-positive quantum_usecs.
 *
* Return value: On success, return 0. On failure, return -1.
*/
int uthread_init(int quantum_usecs)
{
    if (quantum_usecs <= 0)
    {
        fprintf(stderr,"thread library error: invalid input\n");
        return -1;
    }
    quantumUsecs = quantum_usecs;
    totalNumQuantum = 0;
    timer = 0;
    readyThreads = *(new list<thread *>);
    return 0;
}

/*
 * Description: This function creates a new thread, whose entry point is the
 * function f with the signature void f(void). The thread is added to the end
 * of the READY threads list. The uthread_spawn function should fail if it
 * would cause the number of concurrent threads to exceed the limit
 * (MAX_THREAD_NUM). Each thread should be allocated with a stack of size
 * STACK_SIZE bytes.
 * Return value: On success, return the ID of the created thread.
 * On failure, return -1.
*/
int uthread_spawn(void (*f)(void))
{
    if (threadsNum == MAX_THREAD_NUM)
    {
        fprintf(stderr,"thread library error: Can't create more threads.\n");
        return -1;
    }

    address_t sp, pc;

    thread *newThread = new thread;

    sp = (address_t)newThread->stack + STACK_SIZE - sizeof(address_t);
    pc = (address_t)f;
    sigsetjmp(newThread->env, 1);
    (newThread->env->__jmpbuf)[JB_SP] = translate_address(sp);
    (newThread->env->__jmpbuf)[JB_PC] = translate_address(pc);
    sigemptyset(&(newThread->env)->__saved_mask); //todo: do we need to save the mask?

    return 0;
}


/*
 * Description: This function terminates the thread with ID tid and deletes
 * it from all relevant control structures. All the resources allocated by
 * the library for this thread should be released. If no thread with ID tid
 * exists it is considered as an error. Terminating the main thread
 * (tid == 0) will result in the termination of the entire process using
 * exit(0) [after releasing the assigned library memory].
 * Return value: The function returns 0 if the thread was successfully
 * terminated and -1 otherwise. If a thread terminates itself or the main
 * thread is terminated, the function does not return.
*/
int uthread_terminate(int tid);


/*
 * Description: This function blocks the thread with ID tid. The thread may
 * be resumed later using uthread_resume. If no thread with ID tid exists it
 * is considered as an error. In addition, it is an error to try blocking the
 * main thread (tid == 0). If a thread blocks itself, a scheduling decision
 * should be made. Blocking a thread in BLOCKED state has no
 * effect and is not considered as an error.
 * Return value: On success, return 0. On failure, return -1.
*/
int uthread_block(int tid);


/*
 * Description: This function resumes a blocked thread with ID tid and moves
 * it to the READY state. Resuming a thread in a RUNNING or READY state
 * has no effect and is not considered as an error. If no thread with
 * ID tid exists it is considered as an error.
 * Return value: On success, return 0. On failure, return -1.
*/
int uthread_resume(int tid);


/*
 * Description: This function blocks the RUNNING thread until thread with
 * ID tid will move to RUNNING state (i.e.right after the next time that
 * thread tid will stop running, the calling thread will be resumed
 * automatically). If thread with ID tid will be terminated before RUNNING
 * again, the calling thread should move to READY state right after thread
 * tid is terminated (i.e. it won’t be blocked forever). It is considered
 * as an error if no thread with ID tid exists or if the main thread (tid==0)
 * calls this function. Immediately after the RUNNING thread transitions to
 * the BLOCKED state a scheduling decision should be made.
 * Return value: On success, return 0. On failure, return -1.
*/
int uthread_sync(int tid);


/*
 * Description: This function returns the thread ID of the calling thread.
 * Return value: The ID of the calling thread.
*/
int uthread_get_tid();


/*
 * Description: This function returns the total number of quantums that were
 * started since the library was initialized, including the current quantum.
 * Right after the call to uthread_init, the value should be 1.
 * Each time a new quantum starts, regardless of the reason, this number
 * should be increased by 1.
 * Return value: The total number of quantums.
*/
int uthread_get_total_quantums()
{
    return totalNumQuantum;
}


/*
 * Description: This function returns the number of quantums the thread with
 * ID tid was in RUNNING state. On the first time a thread runs, the function
 * should return 1. Every additional quantum that the thread starts should
 * increase this value by 1 (so if the thread with ID tid is in RUNNING state
 * when this function is called, include also the current quantum). If no
 * thread with ID tid exists it is considered as an error.
 * Return value: On success, return the number of quantums of the thread with ID tid. On failure, return -1.
*/
int uthread_get_quantums(int tid)
{

}

/* *********************************** Private Functions ************************************* */

int mainLoop()
{
    while (true)
    {
        if (timer > 0)
        {
            timer -= 1;
        }
        else if (readyThreads.size() > 0)
        {
            thread *newThread = readyThreads.front();
            readyThreads.pop_front();
            switchThreads(*newThread);
        }
    }
}

int switchThreads(thread& newThread)
{
    if (currentRunning != NULL)
    {
        // TODO pause thread
        // TODO check if it needs to go again to ready or paused or terminated thread
        // TODO save the context of the thread (using sigsetjmp)
    }
    // TODO change the state of newThread to running (using siglongjmp)
    currentRunning = &newThread;
    // set the running time for the new thread
    timer = quantumUsecs;
}