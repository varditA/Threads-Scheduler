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
#include <vector>
#include <algorithm>
#include <set>
#include "uthreads.h"

/* **************************** start of black box code ********************* */

#ifdef __x86_64__
/* code for 64 bit Intel arch */

typedef unsigned long address_t;

#define JB_SP 6
#define JB_PC 7
#define TIME_CONVERT 1000000

//error msg

#define INVALID_INPUT_MSG "thread library error: invalid input.\n"
#define BLOCK_ERROR_MSG "system error: can't block the signal.\n"
#define UNBLOCK_ERROR_MSG "system error: can't unblock the signal.\n"
#define MAX_THREADS "thread library error: Can't create more threads.\n"
#define EMPTY_SIGNAL_SET_MSG "system call error: can't empty the signal set.\n"
#define ADDING_SIGNAL_MSG "system call error: can't add to the signals set.\n"
#define REMOVE_MSG "thread library error: can't remove from the ready list.\n"
#define BLOCK_THREAD_MSG "thread library error: main thread can't be blocked.\n"
#define NO_THREAD_MSG "thread library error: thread's id isn't existed.\n"
#define ALLOCATION_MSG "system call error: can't allocate memory.\n"
#define SWITCH_MSG "thread library error: can't switch threads.\n"


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

/* **************************** end of black box code ********************** */

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
    char stack[STACK_SIZE];
    vector<int> dependencies;
    int blockingThread;
    sigjmp_buf env;
} thread;

int quantumUsecs;            /* the length of a quantum in micro-seconds */
int totalNumQuantum;         /* the total number of quantum */
int threadsNum;              /* how many threads exist now */

itimerval timer;             /* how many ms are left for this thread to run */

sigset_t signalsSet;

int currentRunning;          /* the thread that running right now */
list<thread *> readyThreads; /* a list of pointers to READY threads */
map<int, thread *> allThreads;

int getThreadNum();        /* find the next available number for a new thread */
void switchThreads(thread * newThread);/* switches between threads */
int resumeDependencies(thread *trd);
void startTime();
void stopTime();



/* *********************************** Private Functions ******************* */

/*
 * if the thread is running, it switch it with the next ready thread without
 * adding it to the ready list again.
 * else, it removes the thread from the ready list.
 */
int stopThread(int tid)
{
    if (allThreads.find(tid) != allThreads.end()) {
        if (currentRunning != tid) {
            std::list<thread *>::iterator it = readyThreads.begin();
            while (it != readyThreads.end()) {
                if ((*it)->id == tid) {
                    readyThreads.remove(*it++);

                } else {
                    ++it;
                }
            }
        } else {
            if (readyThreads.size() > 0)
            {
                thread *newThread = readyThreads.front();
                readyThreads.pop_front();
                switchThreads(newThread);
            } else
            {
                return -1;
            }

        }
        return 0;
    }
    return 0;
}

/*
 * the function that calls the switch function every time the timer activates
 * it.
 */
void timer_handler(int sig)
{
    readyThreads.push_back(allThreads.at(currentRunning));
    thread *newThread = readyThreads.front();
    readyThreads.pop_front();
    switchThreads(newThread);
}

/*
 * set the timer's signal and its first values.
 */
int initTimer()
{
    struct sigaction sa;

    // Install timer_handler as the signal handler for SIGVTALRM.
    sa.sa_handler = &timer_handler;

    if (sigaction(SIGVTALRM, &sa,NULL) < 0) {
        fprintf(stderr,"system error: can't change the default signal.\n");
        _exit(1);
    }
    startTime();

    // Start a virtual timer. It counts down whenever this process is executing.
    if (setitimer (ITIMER_VIRTUAL, &timer, NULL)) {
        fprintf(stderr,"system error: can't start the timer.\n");
        _exit(1);
    }

    return 0;
}

/*
 * set the timer's default values.
 */
void startTime()
{
    // first time interval, seconds part
    timer.it_value.tv_sec = quantumUsecs/TIME_CONVERT;
    // first time interval, microseconds part
    timer.it_value.tv_usec = quantumUsecs%TIME_CONVERT;

    // following time intervals, seconds part
    timer.it_interval.tv_sec = quantumUsecs/TIME_CONVERT;
    // following time intervals, microseconds part
    timer.it_interval.tv_usec = quantumUsecs%TIME_CONVERT;

}

/*
 * reset the timer.
 */
void stopTime()
{
    // first time interval, seconds part
    timer.it_value.tv_sec = 0;
    // first time interval, microseconds part
    timer.it_value.tv_usec = 0;

    // following time intervals, seconds part
    timer.it_interval.tv_sec = 0;
    // following time intervals, microseconds part
    timer.it_interval.tv_usec = 0;
}

/*
 * switch between the running thread and the next in the ready list.
 */
void switchThreads(thread *newThread)
{
    stopTime();
    thread * oldThread = allThreads.at(currentRunning);
    int ret_val = sigsetjmp(oldThread->env,1);

    if (ret_val != 0) {
        return;
    }

    newThread->quantumsNum++;
    resumeDependencies(newThread);
    totalNumQuantum++;
    currentRunning = newThread->id;
    startTime();
    siglongjmp(newThread->env,1);
}

/*
 * Finds the lowest vacant thread num.
 * if there is no such number, returns -1
 */
int getThreadNum()
{

    for (int i = 1; i < MAX_THREAD_NUM; i++)
    {
        if (allThreads.find(i) == allThreads.end())
        {
            return i;
        }
    }


    return -1;
}

/*
 * released the threads that were blocked by trd thread.
 */
int resumeDependencies(thread *trd)
{
    for_each(trd->dependencies.begin(), trd->dependencies.end(), [](int a)
    {
        thread * current = allThreads.at(a);
        current->blockingThread = -1;
        if (!current->blocked)
        {
            readyThreads.push_back(current);
        }
    });
    trd->dependencies.clear();
    return 0;
}


/* *********************************** Public Functions ******************** */

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
        fprintf(stderr,INVALID_INPUT_MSG);
        return -1;
    }
    currentRunning = 0;
    totalNumQuantum = 1;
    quantumUsecs = quantum_usecs;
    if (sigemptyset(&signalsSet) != 0)
    {
        fprintf(stderr, EMPTY_SIGNAL_SET_MSG);
        _exit(1);
    }
    if (sigaddset(&signalsSet, SIGVTALRM) != 0)
    {
        fprintf(stderr, ADDING_SIGNAL_MSG);
        _exit(1);
    }

    try {
        thread *newThread = new thread;
        newThread->id = 0;
        newThread->quantumsNum = 1;
        newThread->blocked = false;
        newThread->blockingThread = -1;

        if (sigemptyset(&(newThread->env)->__saved_mask) == -1)
        {
            fprintf(stderr,EMPTY_SIGNAL_SET_MSG);
        };

//     Add the new thread into the thread list and ready list
        allThreads.insert(pair<int, thread *>(0, newThread));
        initTimer();
        return 0;
    }
    catch (std::bad_alloc& ba)
    {
        fprintf(stderr,ALLOCATION_MSG);
        _exit(1);
    }
    return -1;

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
    if (sigprocmask(SIG_BLOCK, &signalsSet, NULL) < 0) {
        fprintf(stderr, BLOCK_ERROR_MSG);
        _exit(1);
    }

    // Creating a new thread
    address_t sp, pc;

    int threadNum = getThreadNum();
    if (threadNum == -1)
    {
        fprintf(stderr,MAX_THREADS);
        if (sigprocmask(SIG_UNBLOCK, &signalsSet, NULL) < 0) {
            fprintf(stderr,UNBLOCK_ERROR_MSG);
            _exit(1);
        }
        return -1;
    }

    try {
        thread *newThread = new thread;
        newThread->id = threadNum;
        newThread->quantumsNum = 0;
        newThread->blocked = false;
        newThread->blockingThread = -1;

        sp = (address_t)newThread->stack + STACK_SIZE - sizeof(address_t);
        pc = (address_t)f;
        sigsetjmp(newThread->env, 1);
        (newThread->env->__jmpbuf)[JB_SP] = translate_address(sp);
        (newThread->env->__jmpbuf)[JB_PC] = translate_address(pc);
        if (sigemptyset(&(newThread->env)->__saved_mask) == -1)
        {
            fprintf(stderr,EMPTY_SIGNAL_SET_MSG);
            _exit(1);
        };
        // Add the new thread into the thread list and ready list
        allThreads.insert(pair<int, thread *>(threadNum, newThread));
        readyThreads.push_back(newThread);
        threadsNum++;

        if (sigprocmask(SIG_UNBLOCK, &signalsSet, NULL) < 0) {
            fprintf(stderr,UNBLOCK_ERROR_MSG);
            _exit(1);
        }

        return threadNum;

    }
    catch (std::bad_alloc& ba)
    {
        fprintf(stderr,ALLOCATION_MSG);

        if (sigprocmask(SIG_UNBLOCK, &signalsSet, NULL) < 0) {
            fprintf(stderr,UNBLOCK_ERROR_MSG);
            _exit(1);
        }

        return -1;
    }

    return -1;
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
    if (sigprocmask(SIG_BLOCK, &signalsSet, NULL) < 0) {
        fprintf(stderr, BLOCK_ERROR_MSG);
        _exit(1);
    }
    if (tid == 0)
    {
        auto itr = allThreads.begin();
        while (itr != allThreads.end())
        {
                thread* keyCopy = itr->second;
                itr = allThreads.erase(itr);
                delete keyCopy;
        }
        _exit(0);
    }

    if (allThreads.find(tid) != allThreads.end())
    {
        if (stopThread(tid) != 0)
        {
            fprintf(stderr,REMOVE_MSG);
            return -1;
        }

        thread *threadToDelete = allThreads.at(tid);
        resumeDependencies(threadToDelete);
        allThreads.erase(tid);


        delete(threadToDelete);
        threadsNum--;

        if (sigprocmask(SIG_UNBLOCK, &signalsSet, NULL) < 0) {
            fprintf(stderr,UNBLOCK_ERROR_MSG);
            _exit(1);
        }
        return 0;
    }

    if (sigprocmask(SIG_UNBLOCK, &signalsSet, NULL) < 0) {
        fprintf(stderr,UNBLOCK_ERROR_MSG);
        _exit(1);
    }

    fprintf(stderr,NO_THREAD_MSG);
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
    if (sigprocmask(SIG_BLOCK, &signalsSet, NULL) < 0) {
        fprintf(stderr, BLOCK_ERROR_MSG);
        _exit(1);
                     }

    if (tid == 0)
    {
        fprintf(stderr, BLOCK_THREAD_MSG);
        return -1;
    }
    if (allThreads.find(tid) != allThreads.end())
    {

        if (!allThreads.at(tid)-> blocked)
        {
            allThreads.at(tid)-> blocked = true;
        }

        if (stopThread(tid) != 0)
        {
            fprintf(stderr,REMOVE_MSG);
            return -1;
        }
        if (sigprocmask(SIG_UNBLOCK, &signalsSet, NULL) < 0) {
            fprintf(stderr,UNBLOCK_ERROR_MSG);
            _exit(1);
        }
        return 0;
    }

    if (sigprocmask(SIG_UNBLOCK, &signalsSet, NULL) < 0) {
        fprintf(stderr,UNBLOCK_ERROR_MSG);
        _exit(1);
    }

    fprintf(stderr,NO_THREAD_MSG);
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
    if (sigprocmask(SIG_BLOCK, &signalsSet, NULL) < 0) {
        fprintf(stderr, BLOCK_ERROR_MSG);
        _exit(1);
    }

    if (allThreads.find(tid) != allThreads.end())
    {
        thread * current = allThreads.at(tid);
        if (current-> blocked)
        {
            allThreads.at(tid)->blocked = false;
            if (current->blockingThread == -1)
            {
                readyThreads.push_back(current);
            }
        }

        if (sigprocmask(SIG_UNBLOCK, &signalsSet, NULL) < 0) {
            fprintf(stderr,UNBLOCK_ERROR_MSG);
            _exit(1);
        }

        return 0;
    }

    if (sigprocmask(SIG_UNBLOCK, &signalsSet, NULL) < 0) {
        fprintf(stderr,UNBLOCK_ERROR_MSG);
        _exit(1);
    }

    fprintf(stderr,NO_THREAD_MSG);
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
int uthread_sync(int tid)
{
    if (sigprocmask(SIG_BLOCK, &signalsSet, NULL) < 0) {
        fprintf(stderr, BLOCK_ERROR_MSG);
        _exit(1);
    }


    if (allThreads.find(tid) != allThreads.end() && tid != currentRunning
        && currentRunning != 0) {
        thread *callingThread = allThreads.at(tid);
        callingThread->dependencies.push_back(currentRunning);
        allThreads.at(currentRunning)->blockingThread = tid;
        if (stopThread(currentRunning) != 0)
        {
            fprintf(stderr, SWITCH_MSG);
            return -1;
        }

        if (sigprocmask(SIG_UNBLOCK, &signalsSet, NULL) < 0) {
            fprintf(stderr,UNBLOCK_ERROR_MSG);
            _exit(1);
        }
        return 0;
    }

    if (sigprocmask(SIG_UNBLOCK, &signalsSet, NULL) < 0) {
        fprintf(stderr,UNBLOCK_ERROR_MSG);
        _exit(1);
    }

    if (tid == currentRunning)
    {
        fprintf(stderr, "thread library system: thread can't call sync function"
                "with itself.\n");
    } else
    {
        fprintf(stderr,NO_THREAD_MSG);

    }
    return -1;
}


/*
 * Description: This function returns the thread ID of the calling thread.
 * Return value: The ID of the calling thread.
*/
int uthread_get_tid()
{
    return currentRunning;
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
 * Return value: On success, return the number of quantums of the thread with
 * ID tid. On failure, return -1.
*/
int uthread_get_quantums(int tid)
{
    if (allThreads.find(tid) != allThreads.end())
    {
        return allThreads.at(tid)->quantumsNum;
    }

    fprintf(stderr,NO_THREAD_MSG);
    return -1;

}

