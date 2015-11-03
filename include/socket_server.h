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
}teSocketStatus;

typedef enum
{
    E_EPOLL_OK = 1,
    E_EPOLL_TIMEOUT = 0,
    E_EPOLL_ERROR = -1,
}teSelectResult;

typedef volatile enum
{
    E_THREAD_STOPPED,   /**< Thread stopped */
    E_THREAD_RUNNING,   /**< Thread running */
    E_THREAD_STOPPING,  /**< Thread signaled to stop */
} teState;               /**< Enumerated type of thread states */


typedef void (*tprSock_MessageCallback)(void *pvUser, uint16 u16Length, void *pvMessage);

/** Linked list structure for a callback function entry */
typedef struct _tsSock_CallbackEntry
{
    uint16                u16Type;        /**< Message type for this callback */
    tprSock_MessageCallback   prCallback;     /**< User supplied callback function for this message type */
    void                    *pvUser;        /**< User supplied data for the callback function */
    struct _tsSock_CallbackEntry *psNext;     /**< Pointer to next in linked list */
} tsSock_CallbackEntry;

typedef struct
{
    pthread_mutex_t         mutex;
    tsSock_CallbackEntry    *psListHead;
} tsCallbacks;

typedef struct _tSocketServer
{
    int iSocketFd;

    teState eState;
    pthread_t pthSocketServer;
    pthread_mutex_t mutex;
    
    uint8 u8NumConnClient;
    char *psNetAddress;

    tsCallbacks sCallbacks;
}tsSocketServer;

typedef struct _tSocektClient
{
    int iSocketFd;
    pthread_mutex_t mutex;
    pthread_mutex_t mutex_cond;
    pthread_cond_t cond_message_receive;
    struct sockaddr_in addrclient;
    int iSocketDataLen;
    char csClientData[MXBF];
    struct dl_list list;
}tsSocketClient;

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
teSocketStatus SocektServerSendMessage(int iClientFd, uint16 u16Type, uint16 u16Length, void *psMessage, uint16 *pu16SquenceNo);
teSocketStatus SocketServerWaitMessage(uint16 u16Type, uint32 u32WaitTimeout, uint16 *pu16Length, void **ppvMessage);


#if defined __cplusplus
}
#endif
#endif

