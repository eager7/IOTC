/****************************************************************************
 *
 * MODULE:             iotc_multicast.h
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


#ifndef __H_IOTC_MULTICAST_H_
#define __H_IOTC_MULTICAST_H_

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
#define MULTICAST_ADDRESS "239.227.227.227"
#define MULTICAST_PORT  7789
/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
typedef enum
{
    E_MULTI_OK,
    E_MULTI_ERROR,
    E_MULTI_ERROR_MALLOC,
    E_MULTI_ERROR_PARAM,
    E_MULTI_ERROR_FORMAT,
    E_MULTI_ERROR_PTHREAD_CREATE,
    E_MULTI_ERROR_CREATESOCK,
    E_MULTI_ERROR_SETSOCK,
    E_MULTI_ERROR_BIND,
}teMultiStatus;

typedef struct _tsIotcMulticast
{
    pthread_t                       pthIotcMulti;  
    teThreadState                   eThreadState;
    int                             iSocketFd;
    struct ip_mreq                  multi_addr;
    struct sockaddr_in              server_addr;  
    
    //pthread_mutex_t                 mutex;
}tsIotcMulticast;

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/
teMultiStatus IotcMulticastInit();
teMultiStatus IotcMulticastFinished();

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

