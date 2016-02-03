/****************************************************************************
 *
 * MODULE:             socket lib interface
 *
 * COMPONENT:          msocket.h
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

#ifndef __MSOCKET_H__
#define __MSOCKET_H__

#if defined __cplusplus
extern "C"{
#endif

#include "utils.h"

typedef enum
{
    E_SOCKET_OK,
    E_SOCKET_ERROR_FAILED,
    E_SOCKET_ERROR_TIMEOUT,
    E_SOCKET_ERROR_NO_MEM,
} teSocketStatus;

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


#if defined __cplusplus
}
#endif

#endif /* __MTHREADS_H__ */


