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

#ifndef __MTHREADS_H__
#define __MTHREADS_H__

#if defined __cplusplus
extern "C"{
#endif
/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include "utils.h"
/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
typedef enum
{
    E_THREAD_OK,
    E_THREAD_ERROR_FAILED,
    E_THREAD_ERROR_TIMEOUT,
    E_THREAD_ERROR_NO_MEM,
} teThreadStatus;

typedef enum
{
    E_THREAD_JOINABLE,
    E_THREAD_DETACHED,
} teThreadDetachState;

typedef struct
{
    volatile enum
    {
        E_THREAD_STOPPED,
        E_THREAD_RUNNING,
        E_THREAD_STOPPING,
    } eState;
    teThreadDetachState eThreadDetachState;
    pthread_t pThread_Id;
    void *pvThreadData;
} tsThread;

typedef void *(*tprThreadFunction)(void *psThreadInfoVoid);
/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
teThreadStatus mThreadStart(tprThreadFunction prThreadFunction, tsThread *psThreadInfo, teThreadDetachState eDetachState);
teThreadStatus mThreadStop(tsThread *psThreadInfo);
teThreadStatus mThreadFinish(tsThread *psThreadInfo);
teThreadStatus mThreadYield(void);

typedef enum
{
    E_LOCK_OK,
    E_LOCK_ERROR_FAILED,
    E_LOCK_ERROR_TIMEOUT,
    E_LOCK_ERROR_NO_MEM,
} teLockStatus;

teLockStatus mLockCreate(pthread_mutex_t *psLock);
teLockStatus mLockDestroy(pthread_mutex_t *psLock);
teLockStatus mLockLock(pthread_mutex_t *psLock);
teLockStatus mMLockLockTimed(pthread_mutex_t *psLock, uint32 u32WaitTimeout);
teLockStatus mLockTryLock(pthread_mutex_t *psLock);
teLockStatus mLockUnlock(pthread_mutex_t *psLock);

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
    uint32 u32Length;
    uint32 u32Size;
    uint32 u32Front;
    uint32 u32Rear;

    pthread_mutex_t mutex;    
    pthread_cond_t cond_space_available;
    pthread_cond_t cond_data_available;
} tsQueue;

teQueueStatus mQueueCreate(tsQueue *psQueue, uint32 u32BufferSize, uint32 u32Length); 
teQueueStatus mQueueDestroy(tsQueue *psQueue);
teQueueStatus mQueueEnqueue(tsQueue *psQueue, void *pvData, uint32 u32Length);
teQueueStatus mQueueDequeue(tsQueue *psQueue, void **ppvData);
teQueueStatus mQueueDequeueTimed(tsQueue *psQueue, uint32 u32WaitTimeMil, void **ppvData);


/** Atomically add a 32 bit value to another.
 *  \param pu32Value        Pointer to value to update
 *  \param u32Operand       Value to add
 *  \return New value
 */
uint32 u32AtomicAdd(volatile uint32 *pu32Value, uint32 u32Operand);

/** Atomically get the value of a 32 bit value */
#define u32AtomicGet(pu32Value) u32AtomicAdd(pu32Value, 0)

#if defined __cplusplus
}
#endif

#endif /* __MTHREADS_H__ */


