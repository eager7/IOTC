/****************************************************************************
 *
 * MODULE:             thread lib interface
 *
 * COMPONENT:          mthreads.h
 *
 * REVISION:           $Revision: 52723 $
 *
 * DATED:              $Date: 2016-01-04 17:04:13 $
 *
 * AUTHOR:             panchangtao
 *
 ****************************************************************************
 *
 * Copyright panchangtao@gmail.com 2016. All rights reserved
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

#include <mthread.h>

#define DBG_THREADS 1
#define DBG_LOCKS   1
#define DBG_QUEUE   1

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

teThreadStatus mThreadStart(tprThreadFunction prThreadFunction, tsThread *psThreadInfo, teThreadDetachState eDetachState)
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
			ERR_vPrintf(T_TRUE, "sigaction:%s\n", strerror(errno));
		}
		else
		{
			DBG_vPrintf(DBG_THREADS, "Signal action registered\n\r");
			iFirstTime = 0;
		}
	}
    
    if (pthread_create(&psThreadInfo->pThread_Id, NULL, prThreadFunction, psThreadInfo))
    {
        ERR_vPrintf(T_TRUE, "Could not start thread:%s\n", strerror(errno));
        return E_THREAD_ERROR_FAILED;
    }

    if (eDetachState == E_THREAD_DETACHED)
    {
        DBG_vPrintf(DBG_THREADS, "Detach Thread %p\n", psThreadInfo);
        if (pthread_detach(psThreadInfo->pThread_Id))
        {
            ERR_vPrintf(T_TRUE, "pthread_detach():%s\n", strerror(errno));
            return E_THREAD_ERROR_FAILED;
        }
    }
    DBG_vPrintf(DBG_THREADS, "Create Thread %p\n", psThreadInfo);
    return  E_THREAD_OK;
}

teThreadStatus mThreadStop(tsThread *psThreadInfo)
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
                ERR_vPrintf(T_TRUE, "Could not join thread:%s\n", strerror(errno));
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

teThreadStatus mThreadFinish(tsThread *psThreadInfo)
{
    psThreadInfo->eState = E_THREAD_STOPPED;
    
    DBG_vPrintf(DBG_THREADS, "Finish Thread %p\n", psThreadInfo);
    
    /* Cleanup function is called when pthread quits */
    pthread_exit(NULL);
    return E_THREAD_OK; /* Control won't get here */
}

teThreadStatus mThreadYield(void)
{
    sched_yield();
    return E_THREAD_OK;
}

/************************** Lock Functionality *******************************/
teLockStatus mLockCreate(pthread_mutex_t *psLock)
{
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

teLockStatus mLockDestroy(pthread_mutex_t *psLock)
{
    pthread_mutex_destroy(psLock);
    DBG_vPrintf(DBG_LOCKS, "Lock Destroy: %p\n", psLock);
    return E_LOCK_OK;
}

teLockStatus mLockLock(pthread_mutex_t *psLock)
{
    DBG_vPrintf(DBG_LOCKS, "Thread 0x%lx locking: %p\n", pthread_self(), psLock);
    pthread_mutex_lock(psLock);
    DBG_vPrintf(DBG_LOCKS, "Thread 0x%lx locked: %p\n", pthread_self(), psLock);
    
    return E_LOCK_OK;
}

teLockStatus mLockLockTimed(pthread_mutex_t *psLock, uint32 u32WaitTimeout)
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

teLockStatus mLockTryLock(pthread_mutex_t *psLock)
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

teLockStatus mLockUnlock(pthread_mutex_t *psLock)
{    
    DBG_vPrintf(DBG_LOCKS, "Thread 0x%lx unlocking: %p\n", pthread_self(), psLock);
    pthread_mutex_unlock(psLock);
    DBG_vPrintf(DBG_LOCKS, "Thread 0x%lx unlocked: %p\n", pthread_self(), psLock);
    return E_LOCK_OK;
}


/*******************************************************************************
** �� �� ��  : mQueueCreate
** ��������  : ������Ϣ����
** �������  : tsQueue *psQueue  
             : uint32 u32Length  
** �� �� ֵ  : 
       
** ��    ��  : 2016��1��4��
** ��    ��  : PCT
*******************************************************************************/
teQueueStatus mQueueCreate(tsQueue *psQueue, uint32 u32bufferSize, uint32 u32Length)
{
    psQueue->apvBuffer = malloc(sizeof(void *) * u32Length);
    if (!psQueue->apvBuffer){
        return E_QUEUE_ERROR_NO_MEM;
    }
    
    int i = 0;
    for(i = 0; i < u32Length; i++){
        psQueue->apvBuffer[i] = malloc(u32bufferSize);
        if(psQueue->apvBuffer[i] == NULL){
            goto ERR;
        }
    }
    
    psQueue->u32Length = u32Length;
    psQueue->u32Size = u32bufferSize;
    psQueue->u32Front = 0;
    psQueue->u32Rear = 0;
    
    pthread_mutex_init(&psQueue->mutex, NULL);
    pthread_cond_init(&psQueue->cond_space_available, NULL);
    pthread_cond_init(&psQueue->cond_data_available, NULL);
    
    return E_QUEUE_OK;
ERR:    
    for(i = 0; i < u32Length; i++){
        if(psQueue->apvBuffer[i] != NULL){
            free(psQueue->apvBuffer[i]);
        }
    }
    free(psQueue->apvBuffer);
    
    return E_QUEUE_ERROR_NO_MEM;
}

/*******************************************************************************
** �� �� ��  : mQueueDestroy
** ��������  : ���ٴ�������Ϣ����
** �������  : tsQueue *psQueue  
** �� �� ֵ  : 
       
** ��    ��  : 2016��1��4��
** ��    ��  : PCT
*******************************************************************************/
teQueueStatus mQueueDestroy(tsQueue *psQueue)
{
    if (NULL == psQueue->apvBuffer){
        return E_QUEUE_ERROR_FAILED;
    }

    int i = 0;
    for(i = 0; i < psQueue->u32Length; i++){
        if(psQueue->apvBuffer[i] != NULL){
            free(psQueue->apvBuffer[i]);
        }
    }
    free(psQueue->apvBuffer);
    
    pthread_mutex_destroy(&psQueue->mutex);
    pthread_cond_destroy(&psQueue->cond_space_available);
    pthread_cond_destroy(&psQueue->cond_data_available);

    return E_QUEUE_OK;
}

/*******************************************************************************
** �� �� ��  : mQueueEnqueue
** ��������  : ��Ӻ���������ռ���������Ҫ�ȴ��ռ��ͷţ�Ȼ��㲥����������
               �ݿ���
** �������  : tsQueue *psQueue  
             : void *pvData      
** �� �� ֵ  : 
       
** ��    ��  : 2016��1��4��
** ��    ��  : PCT
*******************************************************************************/
teQueueStatus mQueueEnqueue(tsQueue *psQueue, void *pvData, uint32 u32Length)
{
    pthread_mutex_lock(&psQueue->mutex);
    while (((psQueue->u32Rear + 1)%psQueue->u32Length) == psQueue->u32Front)
        pthread_cond_wait(&psQueue->cond_space_available, &psQueue->mutex);
    //psQueue->apvBuffer[psQueue->u32Rear] = pvData;
    memset(psQueue->apvBuffer[psQueue->u32Rear], 0, psQueue->u32Size);
    if(u32Length < psQueue->u32Size){
        memcpy(psQueue->apvBuffer[psQueue->u32Rear], pvData, u32Length);
    }else{
        memcpy(psQueue->apvBuffer[psQueue->u32Rear], pvData, psQueue->u32Size);
    }   
    
    psQueue->u32Rear = (psQueue->u32Rear+1) % psQueue->u32Length;
    
    pthread_mutex_unlock(&psQueue->mutex);
    pthread_cond_broadcast(&psQueue->cond_data_available);
    return E_QUEUE_OK;
}

/*******************************************************************************
** �� �� ��  : mQueueDequeue
** ��������  : ���Ӻ�������Ҫ�ȴ������������ݿ��ã��������ݺ���Ҫ�㲥������
               �ռ����
** �������  : tsQueue *psQueue  
             : void **ppvData    
** �� �� ֵ  : 
       
** ��    ��  : 2016��1��4��
** ��    ��  : PCT
*******************************************************************************/
teQueueStatus mQueueDequeue(tsQueue *psQueue, void **ppvData)
{
    pthread_mutex_lock(&psQueue->mutex);
    while (psQueue->u32Front == psQueue->u32Rear)
        pthread_cond_wait(&psQueue->cond_data_available, &psQueue->mutex);
    
    *ppvData = psQueue->apvBuffer[psQueue->u32Front];
    
    psQueue->u32Front = (psQueue->u32Front + 1) % psQueue->u32Length;
    pthread_mutex_unlock(&psQueue->mutex);
    pthread_cond_broadcast(&psQueue->cond_space_available);
    return E_QUEUE_OK;
}

/*******************************************************************************
** �� �� ��  : mQueueDequeueTimed
** ��������  : ������ʱ�ȴ����ܵĳ��Ӻ������������õȴ�ʱ��Ȼ�󷵻أ�������
               ��
** �������  : tsQueue *psQueue       
             : uint32 u32WaitTimeout  
             : void **ppvData         
** �� �� ֵ  : 
       
** ��    ��  : 2016��1��4��
** ��    ��  : PCT
*******************************************************************************/
teQueueStatus mQueueDequeueTimed(tsQueue *psQueue, uint32 u32WaitTimeMil, void **ppvData)
{
    pthread_mutex_lock(&psQueue->mutex);
    while (psQueue->u32Front == psQueue->u32Rear)
    {
        struct timeval sNow;
        struct timespec sTimeout;
        
        memset(&sNow, 0, sizeof(struct timeval));
        gettimeofday(&sNow, NULL);
        sTimeout.tv_sec = sNow.tv_sec + (u32WaitTimeMil/1000);
        sTimeout.tv_nsec = (sNow.tv_usec + ((u32WaitTimeMil % 1000) * 1000)) * 1000;
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
    
    *ppvData = psQueue->apvBuffer[psQueue->u32Front];
    
    psQueue->u32Front = (psQueue->u32Front + 1) % psQueue->u32Length;
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

