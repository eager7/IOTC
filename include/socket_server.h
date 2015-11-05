/****************************************************************************
 *
 * MODULE:             socket_server.h
 *
 * COMPONENT:          Utils interface
 *
 * REVISION:           $Revision:  0$
 *
 * DATED:              $Date: 2015-10-21 15:13:17 +0100 (Thu, 21 Oct 2015 $
 *
 * AUTHOR:             PCT
 *
 ****************************************************************************
 *
 * Copyright panchangtao@gmail.com B.V. 2015. All rights reserved
 *
 ***************************************************************************/


#ifndef __H_SOCKET_SERVER_H_
#define __H_SOCKET_SERVER_H_

#if defined __cplusplus
extern "C"{
#endif
/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include <stdio.h>
#include <pthread.h>
#include "utils.h"
#include <netinet/in.h>
#include "list.h"
#include "iotc_command.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#define SOCKET_LISTEN_NUM 10
#define THREAD_SIGNAL SIGUSR1
#define EPOLL_EVENT_NUM 10
/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
typedef enum
{
    E_SOCK_OK = 0,
    E_SOCK_ERROR,
    E_SOCK_INVALID_PARAMETER,
    E_SOCK_ERROR_CREATESOCK,
    E_SOCK_ERROR_SETSOCK,
    E_SOCK_ERROR_BIND,
    E_SOCK_ERROR_LISTEN, 
    E_SOCK_ERROR_PTHREAD_CREATE, 
    E_SOCK_ERROR_JOIN, 
    E_SOCK_ERROR_FORMAT, 
    E_SOCK_NO_MESSAGE,
    E_SOCK_NO_MEMORY,
}teSocketStatus;

typedef enum
{
    E_EPOLL_OK = 1,
    E_EPOLL_TIMEOUT = 0,
    E_EPOLL_ERROR = -1,
}teSelectResult;

typedef enum
{
    E_WAIT_OK = 0,
    E_WAIT_TIMEOUT = ETIMEDOUT,
}teCondWaitResult;

typedef volatile enum
{
    E_THREAD_STOPPED, 
    E_THREAD_RUNNING,  
    E_THREAD_STOPPING, 
}teThreadState;  

typedef void (*tprSocketMessageCallback)(void *psUser, void *pvMessage, uint16 u16Length);

typedef struct _tsSocketCallbackEntry
{
    uint16                          u16Type;       
    tprSocketMessageCallback        prCallback;    
    struct dl_list                  list;
}tsSocketCallbackEntry;

typedef struct _tsSocketCallbacks
{
    pthread_mutex_t                 mutex;
    tsSocketCallbackEntry           sCallListHead;
}tsSocketCallbacks;

typedef struct _tSocketServer
{
    int                             iSocketFd;
    char                            *psNetAddress;
    uint8                           u8NumConnClient;
    pthread_t                       pthSocketServer;
    teThreadState                   eThreadState;
    pthread_mutex_t                 mutex;
    tsSocketCallbacks               sSocketCallbacks;
}tsSocketServer;

typedef struct _tSocektClient
{
    int                             iSocketFd;
    struct sockaddr_in              addrclient;
    pthread_mutex_t                 mutex;              /*Lock The Data*/
    pthread_mutex_t                 mutex_cond;         /*Lock The Cond*/
    pthread_cond_t                  cond_message_receive;
    int                             iSocketDataLen;
    char                            csClientData[MXBF];
    struct dl_list                  list;
}tsSocketClient;

typedef struct _tsSocketCondQuene
{
    
}teSocketCondQuene;
/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
teSocketStatus SocketServerInit(int iPort, char *psNetAddress);
teSocketStatus SocketServerFinished();
teSocketStatus SocektClientSendMessage(int iSocketFd, char *psMessage, int iMessageLen);
teSocketStatus SocektClientWaitMessage(int iSocketFd, char *psMessage, uint32 u32WaitTimeoutms);
teSocketStatus SocketCallBackListenerAdd(uint16 u16MessageType, tprSocketMessageCallback prSocketMessageCallback);


#if defined __cplusplus
}
#endif
#endif

