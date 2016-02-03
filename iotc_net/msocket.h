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

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include "utils.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
typedef enum
{
    E_SOCKET_OK,
    E_SOCKET_NULL,
    E_SOCKET_INIT,
    E_SOCKET_SEND,
    E_SOCKET_RECV,
    E_SOCKET_CLOSE,
    E_SOCKET_ERROR,
    E_SOCKET_TIMEOUT,
    E_SOCKET_DISCONNECT,
    E_SOCKET_NO_MEM,
} temSocketStatus;

typedef struct
{
    int iSocketFd;
    int iDomain;
    union {
        struct sockaddr_in sAddr_Ipv4;
        struct sockaddr_in6 sAddr_Ipv6;
    }sAddr;
} tsmSocket;
//typedef void *(*tprThreadFunction)(void *psThreadInfoVoid);

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
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

/****************************************************************************/
/***        Locate   Functions                                            ***/
/****************************************************************************/

temSocketStatus mSocketInit(tsmSocket *psmSocket, int iPort, char *paNetAddress, bool_t isServer);
temSocketStatus mSocketFinished(tsmSocket *psmSocket);
temSocketStatus mSocketRecv(tsmSocket *psmSocket, char *paRecvMsg, uint16 u16Length);
temSocketStatus mSocketSend(tsmSocket *psmSocket, char *paSendMsg, uint16 u16Length);

#if defined __cplusplus
}
#endif

#endif /* __MTHREADS_H__ */


