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

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include "utils.h"
#include "iotc_application.h"
#include <json/json.h>

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#define DBG_APP 1

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/
static void *IotcApplicationHandleThread(void *arg);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
static tsIotcApplication sIotcApplication;
static const char *paThreadExit = "{\"sequence_no\": 1,\"message_type\": 4,\"event_type\":3,\"description\": \"\"}";
/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
teAppStatus IotcApplicationkInit()
{
    DBG_vPrintf(DBG_APP, "IotcApplicationkInit\n");

    memset(&sIotcApplication, 0, sizeof(tsIotcApplication));
    BLUE_vPrintf(DBG_APP, "pthread_create\n");
    if(0 != pthread_create(&sIotcApplication.pthIotcApp, NULL, IotcApplicationHandleThread, NULL))
    {
        ERR_vPrintf(T_TRUE,"pthread_create failed, %s\n", strerror(errno));  
        return E_APPLICATION_ERROR_PTHREAD_CREATE;
    }
    
    return E_APPLICATION_OK;
}

teAppStatus IotcApplicationFinished()
{
    DBG_vPrintf(DBG_APP, "IotcNetworkFinished\n");

    sIotcApplication.eThreadState = E_THREAD_STOPPED;

    pthread_mutex_lock(&sSocketEventQuene.mutex);
    while(!sSocketEventQuene.flag_space)
    {
        pthread_cond_wait(&sSocketEventQuene.cond_space_available, &sSocketEventQuene.mutex);
    }
    sSocketEventQuene.flag_data = T_TRUE;
    sprintf(sSocketEventQuene.sSocketClient.csClientData, "%s", paThreadExit);
    pthread_mutex_unlock(&sSocketEventQuene.mutex);
    pthread_cond_broadcast(&sSocketEventQuene.cond_data_available); 

    pthread_kill(sIotcApplication.pthIotcApp, THREAD_SIGNAL);
    void *psThread_Result = NULL;
    if(0 != pthread_join(sIotcApplication.pthIotcApp, &psThread_Result))
    {
        ERR_vPrintf(T_TRUE,"phread_join socket failed, %s\n", strerror(errno));  
        return E_APPLICATION_ERROR;
    }
    
    return E_APPLICATION_OK;
}

/****************************************************************************/
/***        Locate   Functions                                            ***/
/****************************************************************************/
static void *IotcApplicationHandleThread(void *arg)
{
    BLUE_vPrintf(DBG_APP, "IotcApplicationHandleThread\n");
    sIotcApplication.eThreadState = E_THREAD_RUNNING;

    while(sIotcApplication.eThreadState)
    {
        sched_yield();
        pthread_mutex_lock(&sSocketEventQuene.mutex);
        while (!sSocketEventQuene.flag_data)
        {
            GREEN_vPrintf(DBG_APP, "pthread_cond_waiting ...\n");
            pthread_cond_wait(&sSocketEventQuene.cond_data_available, &sSocketEventQuene.mutex);
            GREEN_vPrintf(DBG_APP, "pthread_cond_wait wakeup ...\n");
        }
        DBG_vPrintf(DBG_APP, "The Data Recv is %s\n", sSocketEventQuene.sSocketClient.csClientData);

        uint16 u16EventType = 0;
        json_object *psJsonRecvMessage = NULL, *psJsonTemp = NULL;
        if (NULL != (psJsonRecvMessage = json_tokener_parse(sSocketEventQuene.sSocketClient.csClientData)))
        {
            if (NULL != (psJsonTemp = json_object_object_get(psJsonRecvMessage, paKeyEventType)))
            {
                u16EventType = json_object_get_int(psJsonTemp);
                DBG_vPrintf(DBG_APP, "This Event is %d Event\n", u16EventType);
            }
            else
            {
                ERR_vPrintf(T_TRUE, "Json Format Error\n");
                json_object_put(psJsonRecvMessage);
                sSocketEventQuene.flag_data  = T_FALSE;
                sSocketEventQuene.flag_space = T_TRUE;
                pthread_mutex_unlock(&sSocketEventQuene.mutex);
                continue;
            }
        }
        else
        {
            ERR_vPrintf(T_TRUE, "Json Format Error\n");
            sSocketEventQuene.flag_data  = T_FALSE;
            sSocketEventQuene.flag_space = T_TRUE;
            pthread_mutex_unlock(&sSocketEventQuene.mutex);
            continue;
        }
        
        if(E_IOTC_EVENT_EXIT == u16EventType)
        {
            pthread_mutex_unlock(&sSocketEventQuene.mutex);
            json_object_put(psJsonRecvMessage);
            goto done;
        }

        if(E_IOTC_EVENT_APP != u16EventType)
        {
            DBG_vPrintf(DBG_APP, "This Event is not A App Event, Push it Again\n");
            pthread_mutex_unlock(&sSocketEventQuene.mutex);
            json_object_put(psJsonRecvMessage);
            
            continue;
        }
        
        sSocketEventQuene.flag_data  = T_FALSE;
        sSocketEventQuene.flag_space = T_TRUE;
        int iSocketFd = sSocketEventQuene.sSocketClient.iSocketFd;
        memset(&sSocketEventQuene.sSocketClient, 0, sizeof(tsSocketClient));
        pthread_mutex_unlock(&sSocketEventQuene.mutex);      
        pthread_cond_broadcast(&sSocketEventQuene.cond_space_available); 
        
        //IotcNetWorkHandleRecvMessage(iSocketFd, psJsonRecvMessage);
        
        json_object_put(psJsonRecvMessage);
        sleep(0);
    }

done:
    DBG_vPrintf(DBG_APP, "Exit IotcApplicationHandleThread\n");
    pthread_exit("Get Killed Signal");
}


/****************************************************************************/
/***        End           file                                            ***/
/****************************************************************************/


