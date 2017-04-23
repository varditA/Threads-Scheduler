#include <list>
#include <cstdio>
#include <iostream>
#include <setjmp.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include <map>
#include <stdlib.h>
#include "uthreads.h"

/* **************************** start of black box code *************************** */

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

/* **************************** end of black box code *************************** */

using namespace std;

/* map with current threads
 * set with the feed indexes
 * if the set is empty:
 *  check if the max in the map is smaller than MAX_THREAD_NUM */

typedef struct
{
    int id;
    int quantumsNum;
    bool blocked;
    char *state; //TODO what is this for?
    char stack[STACK_SIZE];
    sigjmp_buf env;
} thread;

int quantumUsecs;                   /* the length of a quantum in micro-seconds */
int totalNumQuantum;                /* the total number of quantum */
int threadsNum;                     /* how many threads exist now */
int maxThreadIndex;
int timer;                          /* how many ms are left for this thread to run */

int currentRunning;                 /* the thread that running right now */
list<thread *> readyThreads;        /* a list of pointers to READY threads */
map<int, thread *> allThreads;

int getThreadNum();                 /* find the next available number for a new thread */
void switchThreads(thread* newThread);/* switches between threads */



/* *********************************** Private Functions ************************************* */

void timer_handler(int sig)
{
    while (!readyThreads.empty())
    {
        thread *newThread = readyThreads.front();
        readyThreads.pop_front();
        switchThreads(newThread);
        return;
    }

    //TODO: error if no thread is ready?

}

int mainThread(void)
{
    struct sigaction sa;
    struct itimerval timer;

    // Install timer_handler as the signal handler for SIGVTALRM.
    sa.sa_handler = &timer_handler;
    if (sigaction(SIGVTALRM, &sa,NULL) < 0) {
        printf("sigaction error.");
    }

    // Configure the timer to expire after 1 sec... */
    timer.it_value.tv_sec = 0;		// first time interval, seconds part
    timer.it_value.tv_usec = quantumUsecs;		// first time interval, microseconds part

    // configure the timer to expire every 3 sec after that.
    timer.it_interval.tv_sec = 0;	// following time intervals, seconds part
    timer.it_interval.tv_usec = quantumUsecs;	// following time intervals, microseconds part

    // Start a virtual timer. It counts down whenever this process is executing.
    if (setitimer (ITIMER_VIRTUAL, &timer, NULL)) {
        printf("setitimer error.");
    }

    for(;;) {}
}

void switchThreads(thread *newThread)
{
    if (currentRunning != 0)
    {
        readyThreads.push_back(newThread);

    }

    int ret_val = sigsetjmp(newThread->env,1);
    printf("SWITCH: ret_val=%d\n", ret_val);
    if (ret_val == 1) {
        cout << "in";
        totalNumQuantum++;
        return;
    }
    currentRunning = newThread->id;
    siglongjmp(newThread->env,1);

    /*



    totalNumQuantum += 1; //TODO: this will add a quantom even if the process terminated before
    // TODO time. is that what needs to be done?
    if (currentRunning != NULL)
    {
        // TODO need to check if it needs to go again to ready or paused or terminated thread?
        // TODO save the context of the thread (using sigsetjmp) (not trivial)
    }
    currentRunning = newThread->id;

    // set the running time for the new thread
    timer = quantumUsecs; //TODO: need to change to the operating system's timer

    siglongjmp(newThread->env, 1); //jump to execute the new thread
     */

//    return 0;
}

/*
 * Finds the lowest vacant thread num.
 * if there is no such number, returns -1
 */
int getThreadNum()
{

    for (int i = 1; i <= MAX_THREAD_NUM; i++) //TODO: not very efficient, is there a better way?
    {
        if (allThreads.find(i) == allThreads.end())
        {
            return i;
        }
    }
    // in case that there is no vacant thread num

    return -1;
}

/* *********************************** Public Functions ************************************* */

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
    currentRunning = 0; //TODO: or -1?
    totalNumQuantum = 0;
    timer = 0;
    mainThread();
//    allThreads = new map      //  TODO: init map & list
//    readyThreads = *(new list<thread *>);
    // todo create the main thread
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
    // Creating a new thread
    address_t sp, pc;

    int threadNum = getThreadNum();
    if (threadNum == -1)
    {
        fprintf(stderr,"thread library error: Can't create more threads.\n");
        return -1;
    }

    thread *newThread = new thread; //TODO exception?
    newThread->id = threadNum;
    newThread->quantumsNum = 0;
    newThread->blocked = false;

    sp = (address_t)newThread->stack + STACK_SIZE - sizeof(address_t);
    pc = (address_t)*f;
    sigsetjmp(newThread->env, 1);
    (newThread->env->__jmpbuf)[JB_SP] = translate_address(sp);
    (newThread->env->__jmpbuf)[JB_PC] = translate_address(pc);
    sigemptyset(&(newThread->env)->__saved_mask); //todo: do we need to save the mask?

    // Add the new thread into the thread list and ready list
    allThreads.insert(pair<int, thread *>(threadNum, newThread));
    readyThreads.push_back(newThread);
    threadsNum++;

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
int uthread_terminate(int tid)
{
    if (tid == 0)
    {
        _exit(0);
        return 0;
    }

    if (allThreads.find(tid) != allThreads.end())
    {
        /*
   if (currentRunning == tid)
   {
//        switchThreads(readyThreads.front());
       readyThreads.pop_front();
   }

   for (thread* i : readyThreads) // TODO this takes O(n) time - is there a better way?
   {
       if (i->id == tid)
       {
           readyThreads.remove(i);
       }
   }


    */
        thread *threadToDelete = allThreads.at(tid);
        allThreads.erase(tid);
        delete(threadToDelete); //TODO should we erase from the map & the list?
        threadsNum--;

        return 0;
    }

    return -1;
}


/*
 * Description: This function blocks the thread with ID tid. The thread may
 * be resumed later using uthread_resume. If no thread with ID tid exists it
 * is considered as an error. In addition, it is an error to try blocking the
 * main thread (tid == 0). If a thread blocks itself, a scheduling decision
 * should be made. Blocking a thread in BLOCKED state has no
 * effect and is not considered as an error.
 * Return value: On success, return 0. On failure, return -1.
*/
int uthread_block(int tid)
{
    if (tid == 0)
    {
        //TODO ERROR
    }
    if (allThreads.find(tid) != allThreads.end())
    {
        if (allThreads.at(tid)-> blocked)
        {
            return 0;
        }

        //TODO CHECK IF BLOCKING ITSELF and use switch threats
//        readyThreads.remove_if (thread->tid);
//        readyThreads.remove(thread->id==tid);
        allThreads.at(tid)-> blocked = true;

    }

    return -1;
}



/*
 * Description: This function resumes a blocked thread with ID tid and moves
 * it to the READY state. Resuming a thread in a RUNNING or READY state
 * has no effect and is not considered as an error. If no thread with
 * ID tid exists it is considered as an error.
 * Return value: On success, return 0. On failure, return -1.
*/
int uthread_resume(int tid)
{
    if (allThreads.find(tid) != allThreads.end())
    {
        if (allThreads.at(tid)-> blocked)
        {
            allThreads.at(tid)->blocked = false;
            readyThreads.push_back(allThreads.at(tid));
        }
        return 0;
    }

    return -1;
}


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
int uthread_sync(int tid);  //TODO implement


/*
 * Description: This function returns the thread ID of the calling thread.
 * Return value: The ID of the calling thread.
*/
int uthread_get_tid()
{
    return currentRunning; //TODO: right?
}


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
    if (allThreads.find(tid) != allThreads.end())
    {
        return allThreads.at(tid)->quantumsNum;
    }
    return -1;

}

