/****************************************************************************
 *
 * MODULE:             iotc_thread.h
 *
 * COMPONENT:          connect with cc interface
 *
 * REVISION:           $Revision:  0$
 *
 * DATED:              $Date: 2015-12-12 15:13:17 +0100 (Thu, 21 Oct 2015 $
 *
 * AUTHOR:             PCT
 *
 ****************************************************************************
 *
 * Copyright panchangtao@gmail.com B.V. 2015. All rights reserved
 *
 ***************************************************************************/

#ifndef __IOTC_THREADS_H__
#define __IOTC_THREADS_H__

#include "utils.h"
/** Enumerated type of thread status's */
typedef enum
{
    E_THREAD_OK,
    E_THREAD_ERROR_FAILED,
    E_THREAD_ERROR_TIMEOUT,
    E_THREAD_ERROR_NO_MEM,
} teThreadStatus;


/** Enumerated type for thread detached / joinable states */
typedef enum
{
    E_THREAD_JOINABLE,      /**< Thread is created so that it can be waited on and joined */
    E_THREAD_DETACHED,      /**< Thread is created detached so all resources are automatically free'd at exit. */
} teThreadDetachState;


/** Structure to represent a thread */
typedef struct
{
    volatile enum
    {
        E_THREAD_STOPPED,   /**< Thread stopped */
        E_THREAD_RUNNING,   /**< Thread running */
        E_THREAD_STOPPING,  /**< Thread signaled to stop */
    } eState;               /**< Enumerated type of thread states */
    teThreadDetachState eThreadDetachState; /**< Detach state of the thread */
    pthread_t pThread_Id;           /**< Implementation specfific private structure */
    void *pvThreadData;     /**< Pointer to threads data parameter */
} tsThread;


typedef void *(*tprThreadFunction)(void *psThreadInfoVoid);

/** Function to start a thread */
teThreadStatus eThreadStart(tprThreadFunction prThreadFunction, tsThread *psThreadInfo, teThreadDetachState eDetachState);


/** Function to stop a thread 
 *  This function blocks until the specified thread exits.
 */
teThreadStatus eThreadStop(tsThread *psThreadInfo);


/** Function to be called within the thread when it is finished to clean up memory */
teThreadStatus eThreadFinish(tsThread *psThreadInfo);


/** Function to yield the CPU to another thread 
 *  \return E_THREAD_OK on success.
 */
teThreadStatus eThreadYield(void);


/** Enumerated type of thread status's */
typedef enum
{
    E_LOCK_OK,
    E_LOCK_ERROR_FAILED,
    E_LOCK_ERROR_TIMEOUT,
    E_LOCK_ERROR_NO_MEM,
} teLockStatus;

teLockStatus eLockCreate(pthread_mutex_t *psLock);

teLockStatus eLockDestroy(pthread_mutex_t *psLock);

/** Lock the data structure associated with this lock
 *  \param  psLock  Pointer to lock structure
 *  \return E_LOCK_OK if locked ok
 */
teLockStatus eLockLock(pthread_mutex_t *psLock);

/** Lock the data structure associated with this lock
 *  If possible within \param u32WaitTimeout seconds.
 *  \param  psLock  Pointer to lock structure
 *  \param  u32WaitTimeout  Number of seconds to attempt to lock the structure for
 *  \return E_LOCK_OK if locked ok, E_LOCK_ERROR_TIMEOUT if structure could not be acquired
 */
teLockStatus eMLockLockTimed(pthread_mutex_t *psLock, uint32 u32WaitTimeout);


/** Attempt to lock the data structure associated with this lock
 *  If the lock could not be acquired immediately, return E_LOCK_ERROR_FAILED.
 *  \param  psLock  Pointer to lock structure
 *  \return E_LOCK_OK if locked ok or E_LOCK_ERROR_FAILED if not
 */
teLockStatus eLockTryLock(pthread_mutex_t *psLock);


/** Unlock the data structure associated with this lock
 *  \param  psLock  Pointer to lock structure
 *  \return E_LOCK_OK if unlocked ok
 */
teLockStatus eLockUnlock(pthread_mutex_t *psLock);


typedef enum
{
    E_QUEUE_OK,
    E_QUEUE_ERROR_FAILED,
    E_QUEUE_ERROR_TIMEOUT,
    E_QUEUE_ERROR_NO_MEM,
} teQueueStatus;

typedef struct
{
    void **apvBuffer;
    uint32 u32Capacity;
    uint32 u32Size;
    uint32 u32In;
    uint32 u32Out;

    pthread_mutex_t mutex;    
    pthread_cond_t cond_space_available;
    pthread_cond_t cond_data_available;
} tsQueue;

teQueueStatus eQueueCreate(tsQueue *psQueue, uint32 u32Capacity);

teQueueStatus eQueueDestroy(tsQueue *psQueue);

teQueueStatus eQueueQueue(tsQueue *psQueue, void *pvData);

teQueueStatus eQueueDequeue(tsQueue *psQueue, void **ppvData);

teQueueStatus eQueueDequeueTimed(tsQueue *psQueue, uint32 u32WaitTimeout, void **ppvData);


/** Atomically add a 32 bit value to another.
 *  \param pu32Value        Pointer to value to update
 *  \param u32Operand       Value to add
 *  \return New value
 */
uint32 u32AtomicAdd(volatile uint32 *pu32Value, uint32 u32Operand);
/** Atomically get the value of a 32 bit value */
#define u32AtomicGet(pu32Value) u32AtomicAdd(pu32Value, 0)

#endif /* __THREADS_H__ */


