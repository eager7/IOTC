/****************************************************************************
 *
 * MODULE:             iotc_application.h
 *
 * COMPONENT:          APP interface
 *
 * REVISION:           $Revision:  0$
 *
 * DATED:              $Date: 2015-11-08 15:13:17 +0100 (Thu, 21 Oct 2015 $
 *
 * AUTHOR:             PCT
 *
 ****************************************************************************
 *
 * Copyright panchangtao@gmail.com B.V. 2015. All rights reserved
 *
 ***************************************************************************/


#ifndef __H_IOTC_APPLICATION_H_
#define __H_IOTC_APPLICATION_H_

#if defined __cplusplus
extern "C"{
#endif
/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include "utils.h"
#include "socket_server.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#define THREAD_APP_SIGNAL SIGUSR2

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
typedef enum
{
    E_APPLICATION_OK,
    E_APPLICATION_ERROR,
    E_APPLICATION_ERROR_MALLOC,
    E_APPLICATION_ERROR_PARAM,
    E_APPLICATION_ERROR_FORMAT,
    E_APPLICATION_ERROR_PTHREAD_CREATE
}teAppStatus;

typedef struct _tsIotcApplication
{
    pthread_t                       pthIotcApp;  
    teThreadState                   eThreadState;
    //pthread_mutex_t                 mutex;
}tsIotcApplication;

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/
teAppStatus IotcApplicationkInit();
teAppStatus IotcApplicationFinished();

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

