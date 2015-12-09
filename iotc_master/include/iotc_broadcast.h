/****************************************************************************
 *
 * MODULE:             iotc_broadcast.h
 *
 * COMPONENT:          APP interface
 *
 * REVISION:           $Revision:  0$
 *
 * DATED:              $Date: 2015-11-09 15:13:17 +0100 (Thu, 21 Oct 2015 $
 *
 * AUTHOR:             PCT
 *
 ****************************************************************************
 *
 * Copyright panchangtao@gmail.com B.V. 2015. All rights reserved
 *
 ***************************************************************************/


#ifndef __H_IOTC_BROADCAST_H_
#define __H_IOTC_BROADCAST_H_

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
#define BROADCAST_ADDRESS "255.255.255.255"
#define BROADCAST_PORT  6789
/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
typedef enum
{
    E_BROAD_OK,
    E_BROAD_ERROR,
    E_BROAD_ERROR_MALLOC,
    E_BROAD_ERROR_PARAM,
    E_BROAD_ERROR_FORMAT,
    E_BROAD_ERROR_PTHREAD_CREATE,
    E_BROAD_ERROR_CREATESOCK,
    E_BROAD_ERROR_SETSOCK,
    E_BROAD_ERROR_BIND,
}teBroadStatus;

typedef struct _tsIotcBroadcast
{
    pthread_t                       pthIotcBroad;  
    teThreadState                   eThreadState;
    int                             iSocketFd;
    struct sockaddr_in              server_addr;  
    
}tsIotcBroadcast;

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/
teBroadStatus IotcBroadcastInit();
teBroadStatus IotcBroadcastFinished();

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
/****************************************************************************/
/***        Locate   Functions                                            ***/
/****************************************************************************/

#if defined __cplusplus
}
#endif
#endif


