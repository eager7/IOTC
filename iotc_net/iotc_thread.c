/****************************************************************************
 *
 * MODULE:             libJIP
 *
 * COMPONENT:          Thread.c
 *
 * REVISION:           $Revision: 53151 $
 *
 * DATED:              $Date: 2013-04-09 16:14:34 +0100 (Tue, 09 Apr 2013) $
 *
 * AUTHOR:             Matt Redfearn
 *
 ****************************************************************************
 *
 * This software is owned by NXP B.V. and/or its supplier and is protected
 * under applicable copyright laws. All rights are reserved. We grant You,
 * and any third parties, a license to use this software solely and
 * exclusively on NXP products [NXP Microcontrollers such as JN5148, JN5142, JN5139]. 
 * You, and any third parties must reproduce the copyright and warranty notice
 * and any other legend of ownership on each copy or partial copy of the 
 * software.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.

 * Copyright NXP B.V. 2012. All rights reserved
 *
 ***************************************************************************/

#include <pthread.h>
#include <sched.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <sys/time.h>

#include <Threads.h>

#define DBG_THREADS 0
#define DBG_LOCKS   0
#define DBG_QUEUE   0

#define THREAD_SIGNAL SIGUSR1

/************************** Threads Functionality ****************************/

/** Structure representing an OS independant thread */
typedef struct
{
    pthread_t thread;
    tprThreadFunction   prThreadFunction;
} tsThreadPrivate;


/** Signal handler to receive THREAD_SIGNAL.
 *  This is just used to interrupt system calls such as recv() and sleep().
 */
static void thread_signal_handler(int sig)
{
    DBG_vPrintf(DBG_THREADS, "Signal %d received\n", sig);
}


teThreadStatus eThreadStart(tprThreadFunction prThreadFunction, tsThread *psThreadInfo, teThreadDetachState eDetachState)
{
    
    psThreadInfo->eState = E_THREAD_STOPPED;
    psThreadInfo->eThreadDetachState = eDetachState;

    static int iFirstTime = 1;
    if (iFirstTime)
    {
        /* Set up sigmask to receive configured signal in the main thread. 
        * All created threads also get this signal mask, so all threads
        * get the signal. But we can use pthread_signal to direct it at one.
        */
        struct sigaction sa;
        sa.sa_handler = thread_signal_handler;
        sa.sa_flags = 0;
        sigemptyset(&sa.sa_mask);

        if (sigaction(THREAD_SIGNAL, &sa, NULL) == -1) 
        {
            perror("sigaction");
        }
        else
        {
            DBG_vPrintf(DBG_THREADS, "Signal action registered\n\r");
            iFirstTime = 0;
        }
    }
    
    if (pthread_create(&psThreadInfo->pThread_Id, NULL, prThreadFunction, psThreadInfo))
    {
        perror("Could not start thread");
        return E_THREAD_ERROR_FAILED;
    }

    if (eDetachState == E_THREAD_DETACHED)
    {
        DBG_vPrintf(DBG_THREADS, "Detach Thread %p\n", psThreadInfo);
        if (pthread_detach(psThreadInfo->pThread_Id))
        {
            perror("pthread_detach()");
            return E_THREAD_ERROR_FAILED;
        }
    }
    return  E_THREAD_OK;
}


teThreadStatus eThreadStop(tsThread *psThreadInfo)
{
    DBG_vPrintf(DBG_THREADS, "Stopping Thread %p\n", psThreadInfo);
    
    psThreadInfo->eState = E_THREAD_STOPPING;
    
    if (0 != psThreadInfo->pThread_Id)
    {
    
        /* Send signal to the thread to kick it out of any system call it was in */
        pthread_kill(psThreadInfo->pThread_Id, THREAD_SIGNAL);
        DBG_vPrintf(DBG_THREADS, "Signaled Thread %p\n", psThreadInfo);
        
        if (psThreadInfo->eThreadDetachState == E_THREAD_JOINABLE)
        {
            /* Thread is joinable */
            if (pthread_join(psThreadInfo->pThread_Id, NULL))
            {
                perror("Could not join thread");
                return E_THREAD_ERROR_FAILED;
            }
        }
        else
        {
            DBG_vPrintf(DBG_THREADS, "Cannot join detached thread %p\n", psThreadInfo);
            return E_THREAD_ERROR_FAILED;
        }
    }
    
    DBG_vPrintf(DBG_THREADS, "Stopped Thread %p\n", psThreadInfo);
    psThreadInfo->eState = E_THREAD_STOPPED;
    return  E_THREAD_OK;
}


teThreadStatus eThreadFinish(tsThread *psThreadInfo)
{
    psThreadInfo->eState = E_THREAD_STOPPED;
    
    DBG_vPrintf(DBG_THREADS, "Finish Thread %p\n", psThreadInfo);
    
    /* Cleanup function is called when pthread quits */
    pthread_exit(NULL);
    return E_THREAD_OK; /* Control won't get here */
}


teThreadStatus eThreadYield(void)
{
    sched_yield();
    return E_THREAD_OK;
}


/************************** Lock Functionality *******************************/
teLockStatus eLockCreate(pthread_mutex_t *psLock)
{
    /* Create a recursive mutex, as we need to allow the same thread to lock mutexes a number of times */
    pthread_mutexattr_t     attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);

    if (pthread_mutex_init(psLock, &attr) != 0)
    {
        DBG_vPrintf(DBG_LOCKS, "Error initialising mutex\n");
        return E_LOCK_ERROR_FAILED;
    }

    DBG_vPrintf(DBG_LOCKS, "Lock Create: %p\n", psLock);
    return E_LOCK_OK;
}


teLockStatus eLockDestroy(pthread_mutex_t *psLock)
{
    pthread_mutex_destroy(psLock);
    DBG_vPrintf(DBG_LOCKS, "Lock Destroy: %p\n", psLock);
    return E_LOCK_OK;
}


teLockStatus eLockLock(pthread_mutex_t *psLock)
{
    DBG_vPrintf(DBG_LOCKS, "Thread 0x%lx locking: %p\n", pthread_self(), psLock);
    pthread_mutex_lock(psLock);
    DBG_vPrintf(DBG_LOCKS, "Thread 0x%lx locked: %p\n", pthread_self(), psLock);
    
    return E_LOCK_OK;
}


teLockStatus eLockLockTimed(pthread_mutex_t *psLock, uint32 u32WaitTimeout)
{    
    struct timeval sNow;
    struct timespec sTimeout;
    
    gettimeofday(&sNow, NULL);
    sTimeout.tv_sec = sNow.tv_sec + u32WaitTimeout;
    sTimeout.tv_nsec = sNow.tv_usec * 1000;
    
    DBG_vPrintf(DBG_LOCKS, "Thread 0x%lx time locking: %p\n", pthread_self(), psLock);

    switch (pthread_mutex_timedlock(psLock, &sTimeout))
    {
        case (0):
            DBG_vPrintf(DBG_LOCKS, "Thread 0x%lx: time locked: %p\n", pthread_self(), psLock);
            return E_LOCK_OK;
            break;
            
        case (ETIMEDOUT):
            DBG_vPrintf(DBG_LOCKS, "Thread 0x%lx: time out locking: %p\n", pthread_self(), psLock);
            return E_LOCK_ERROR_TIMEOUT;
            break;

        default:
            DBG_vPrintf(DBG_LOCKS, "Thread 0x%lx: error locking: %p\n", pthread_self(), psLock);
            return E_LOCK_ERROR_FAILED;
            break;
    }
}

teLockStatus eLockTryLock(pthread_mutex_t *psLock)
{
    DBG_vPrintf(DBG_LOCKS, "Thread 0x%lx try locking: %p\n", pthread_self(), psLock);
    if (pthread_mutex_trylock(psLock) != 0)
    {
        DBG_vPrintf(DBG_LOCKS, "Thread 0x%lx could not lock: %p\n", pthread_self(), psLock);
        return E_LOCK_ERROR_FAILED;
    }
    DBG_vPrintf(DBG_LOCKS, "Thread 0x%lx locked: %p\n", pthread_self(), psLock);
    
    return E_LOCK_OK;
}


teLockStatus eLockUnlock(pthread_mutex_t *psLock)
{    
    DBG_vPrintf(DBG_LOCKS, "Thread 0x%lx unlocking: %p\n", pthread_self(), psLock);
    pthread_mutex_unlock(psLock);
    DBG_vPrintf(DBG_LOCKS, "Thread 0x%lx unlocked: %p\n", pthread_self(), psLock);
    return E_LOCK_OK;
}


/************************** Queue Functionality ******************************/
teQueueStatus eQueueCreate(tsQueue *psQueue, uint32 u32Capacity)
{
    psQueue->apvBuffer = malloc(sizeof(void *) * u32Capacity);
    
    if (!psQueue->apvBuffer)
    {
        return E_QUEUE_ERROR_NO_MEM;
    }
    
    psQueue->u32Capacity = u32Capacity;
    psQueue->u32Size = 0;
    psQueue->u32In = 0;
    psQueue->u32Out = 0;
    
    pthread_mutex_init(&psQueue->mutex, NULL);
    pthread_cond_init(&psQueue->cond_space_available, NULL);
    pthread_cond_init(&psQueue->cond_data_available, NULL);
    
    return E_QUEUE_OK;
}


teQueueStatus eQueueDestroy(tsQueue *psQueue)
{
    if (NULL == psQueue->apvBuffer)
    {
        return E_QUEUE_ERROR_FAILED;
    }
    free(psQueue->apvBuffer);
    
    pthread_mutex_destroy(&psQueue->mutex);
    pthread_cond_destroy(&psQueue->cond_space_available);
    pthread_cond_destroy(&psQueue->cond_data_available);

    return E_QUEUE_OK;
}


teQueueStatus eQueueQueue(tsQueue *psQueue, void *pvData)
{
    pthread_mutex_lock(&psQueue->mutex);
    while (psQueue->u32Size == psQueue->u32Capacity)
        pthread_cond_wait(&psQueue->cond_space_available, &psQueue->mutex);
    psQueue->apvBuffer[psQueue->u32In] = pvData;
    ++psQueue->u32Size;
    
    psQueue->u32In = (psQueue->u32In+1) % psQueue->u32Capacity;
    
    pthread_mutex_unlock(&psQueue->mutex);
    pthread_cond_broadcast(&psQueue->cond_data_available);
    return E_QUEUE_OK;
}


teQueueStatus eQueueDequeue(tsQueue *psQueue, void **ppvData)
{
    pthread_mutex_lock(&psQueue->mutex);
    while (psQueue->u32Size == 0)
        pthread_cond_wait(&psQueue->cond_data_available, &psQueue->mutex);
    
    *ppvData = psQueue->apvBuffer[psQueue->u32Out];
    --psQueue->u32Size;
    
    psQueue->u32Out = (psQueue->u32Out + 1) % psQueue->u32Capacity;
    pthread_mutex_unlock(&psQueue->mutex);
    pthread_cond_broadcast(&psQueue->cond_space_available);
    return E_QUEUE_OK;
}


teQueueStatus eQueueDequeueTimed(tsQueue *psQueue, uint32 u32WaitTimeout, void **ppvData)
{
    pthread_mutex_lock(&psQueue->mutex);
    while (psQueue->u32Size == 0)
    {
        struct timeval sNow;
        struct timespec sTimeout;
        
        memset(&sNow, 0, sizeof(struct timeval));
        gettimeofday(&sNow, NULL);
        sTimeout.tv_sec = sNow.tv_sec + (u32WaitTimeout/1000);
        sTimeout.tv_nsec = (sNow.tv_usec + ((u32WaitTimeout % 1000) * 1000)) * 1000;
        if (sTimeout.tv_nsec > 1000000000)
        {
            sTimeout.tv_sec++;
            sTimeout.tv_nsec -= 1000000000;
        }
        DBG_vPrintf(DBG_QUEUE, "Dequeue timed: now    %lu s, %lu ns\n", sNow.tv_sec, sNow.tv_usec * 1000);
        DBG_vPrintf(DBG_QUEUE, "Dequeue timed: until  %lu s, %lu ns\n", sTimeout.tv_sec, sTimeout.tv_nsec);

        switch (pthread_cond_timedwait(&psQueue->cond_data_available, &psQueue->mutex, &sTimeout))
        {
            case (0):
                break;
            
            case (ETIMEDOUT):
                pthread_mutex_unlock(&psQueue->mutex);
                return E_QUEUE_ERROR_TIMEOUT;
                break;
            
            default:
                pthread_mutex_unlock(&psQueue->mutex);
                return E_QUEUE_ERROR_FAILED;
        }
    }
    
    *ppvData = psQueue->apvBuffer[psQueue->u32Out];
    --psQueue->u32Size;
    
    psQueue->u32Out = (psQueue->u32Out + 1) % psQueue->u32Capacity;
    pthread_mutex_unlock(&psQueue->mutex);
    pthread_cond_broadcast(&psQueue->cond_space_available);
    return E_QUEUE_OK;
}



uint32 u32AtomicAdd(volatile uint32 *pu32Value, uint32 u32Operand)
{
#if defined(_MSC_VER)
        return add_value + InterlockedExchangeAdd(pu32Value, u32Operand);
#else
        return __sync_add_and_fetch (pu32Value, u32Operand);
#endif
}

