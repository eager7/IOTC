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
    E_SOCKET_ERROR_FAILED,
    E_SOCKET_ERROR_TIMEOUT,
    E_SOCKET_ERROR_NO_MEM,
    E_SOCKET_NULL,
} temSocketStatus;

typedef struct
{
    int iSocketFd;
    struct sockaddr_in sAddr;
} tsmSocket;
//typedef void *(*tprThreadFunction)(void *psThreadInfoVoid);

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#define mSocketCheckError(x,y) do{if(x != E_SOCKET_OK){return y;}}while(0)
#define mSocketCheckNull(x,y) do{if(x == NULL){return y;}}while(0)
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

#if defined __cplusplus
}
#endif

#endif /* __MTHREADS_H__ */


