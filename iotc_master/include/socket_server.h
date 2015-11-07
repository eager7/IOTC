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
#include <signal.h>
/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#define SOCKET_LISTEN_NUM 20
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
    E_SOCK_DISCONNECT,
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

typedef struct _tSocketServer
{
    int                             iSocketFd;
    char                            *psNetAddress;
    uint8                           u8NumConnClient;
    pthread_t                       pthSocketServer;
    teThreadState                   eThreadState;
    pthread_mutex_t                 mutex;
}tsSocketServer;

typedef struct _tSocektClient
{
    int                             iSocketFd;
    struct sockaddr_in              addrclient;
    int                             iSocketDataLen;
    char                            csClientData[MXBF];
    struct dl_list                  list;
}tsSocketClient;

typedef struct _tsSocketClientList
{
    tsSocketClient                  sSocketClient;
    pthread_mutex_t                 mutex;
}tsSocketClientHead;

typedef struct _tsSocketData
{
    int     iSocketFd;
    int     iSocketDataLen;
    char    paSocketData[MXBF];
}tsSocketData;

typedef struct _tsSocketEvent
{
    teIotcEvnetType eSocketCondEvent;
    union 
    {
        tsSocketData sSocketData;

    }uCondData;
    
}tsSocketEvent;

typedef struct _tsSocketEventQuene
{
    volatile sig_atomic_t           flag;
    pthread_mutex_t                 mutex;
    pthread_cond_t                  cond_data_recv;

    tsSocketEvent                   sSocketEvent;
}tsSocketEventQuene;
/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/
extern tsSocketEventQuene sSocketEventQuene;

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
teSocketStatus SocketServerInit(int iPort, char *psNetAddress);
teSocketStatus SocketServerFinished();
teSocketStatus SocketClientSendMessage(int iSocketFd, char *psMessage, int iMessageLen);
teSocketStatus SocketClientWaitMessage(int iSocketFd, char *psMessage, uint32 u32WaitTimeoutms);


#if defined __cplusplus
}
#endif
#endif

