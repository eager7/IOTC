/****************************************************************************
 *
 * MODULE:             iotc_network.h
 *
 * COMPONENT:          device manager
 *
 * REVISION:           $Revision:  0$
 *
 * DATED:              $Date: 2015-11-05 15:13:17 +0100 (Thu, 21 Oct 2015 $
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
#include "iotc_network.h"
#include "iotc_devices.h"
#include "socket_server.h"
#include <json/json.h>

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#define DBG_NETWORK 1

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/
static void *IotcNetworkHandleThread(void *arg);


static teNetworkStatus IotcNetworkHandleDevicesReport(int iSocketFd, json_object *psJsonMessage);
static teNetworkStatus IotcNetworkHandleSocketDisconnect(int iSocketFd, json_object *psJsonMessage);
/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
static tsIotcNetwork sIotcNetwork;
static const char *paThreadExit = "{\"sequence_no\": 1,\"message_type\": 4,\"event_type\":3,\"description\": \"\"}";

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
teNetworkStatus IotcNetworkInit()
{
    DBG_vPrintf(DBG_NETWORK, "IotcNetworkInit\n");

    IotcDeviceInit();    

    memset(&sIotcNetwork, 0, sizeof(tsIotcNetwork));
    BLUE_vPrintf(DBG_NETWORK, "pthread_create\n");
    if(0 != pthread_create(&sIotcNetwork.pthIotcNetwork, NULL, IotcNetworkHandleThread, NULL))
    {
        ERR_vPrintf(T_TRUE,"pthread_create failed, %s\n", strerror(errno));  
        return E_SOCK_ERROR_PTHREAD_CREATE;
    }

    return E_NETWORK_OK;
}

teNetworkStatus IotcNetworkFinished()
{
    DBG_vPrintf(DBG_NETWORK, "IotcNetworkFinished\n");
    IotcDeviceFinished(); 
    
    sIotcNetwork.eThreadState = E_THREAD_STOPPED;
    DBG_vPrintf(DBG_NETWORK, "pthread_kill\n");
    
    pthread_mutex_lock(&sSocketEventQuene.mutex);
    while(!sSocketEventQuene.flag_space)
    {
        pthread_cond_wait(&sSocketEventQuene.cond_space_available, &sSocketEventQuene.mutex);
    }
    sSocketEventQuene.flag_data = T_TRUE;
    sprintf(sSocketEventQuene.sSocketClient.csClientData, "%s", paThreadExit);
    pthread_mutex_unlock(&sSocketEventQuene.mutex);
    pthread_cond_broadcast(&sSocketEventQuene.cond_data_available); 

    pthread_kill(sIotcNetwork.pthIotcNetwork, THREAD_SIGNAL);
    void *psThread_Result = NULL;
    if(0 != pthread_join(sIotcNetwork.pthIotcNetwork, &psThread_Result))
    {
        ERR_vPrintf(T_TRUE,"phread_join socket failed, %s\n", strerror(errno));  
        return E_SOCK_ERROR_JOIN;
    }
    
    return E_NETWORK_OK;
}

/****************************************************************************/
/***        Locate   Functions                                            ***/
/****************************************************************************/
static teNetworkStatus IotcNetworkHandleDevicesReport(int iSocketFd, json_object *psJsonMessage)
{
    DBG_vPrintf(DBG_NETWORK, "IotcNetworkHandleDevicesReport\n");

    if(NULL == psJsonMessage)
    {
        ERR_vPrintf(T_TRUE, "The paramer is error\n");
        return E_NETWORK_ERROR_PARAM;
    }

    json_object *psJsonDeviceArray = NULL;
    if(NULL != (psJsonDeviceArray = json_object_object_get(psJsonMessage, paKeyDescription)))
    {
        uint8 u8DeviceNumber = json_object_array_length(psJsonDeviceArray);
        int i = 0;
        for(i = 0; i < u8DeviceNumber; i++)
        {
            json_object *psJsonDevice = json_object_array_get_idx(psJsonDeviceArray, i);
            char paDeviceName[256];
            sprintf(paDeviceName, "%s",json_object_get_string(json_object_object_get(psJsonDevice, paKeyDeviceName)));
            uint16 u16DeviceID = json_object_get_int(json_object_object_get(psJsonDevice, paKeyDeviceId));
            uint64 u64DeviceIndex = json_object_get_int64(json_object_object_get(psJsonDevice, paKeyDeviceIndex));
            if(E_IOTC_OK != IotcDeviceAdd(paDeviceName, u16DeviceID, u64DeviceIndex, iSocketFd))
            {
                ERR_vPrintf(T_TRUE, "IotcDeviceAdd Error, Maybe exist...\n");
            }
        }
    }
    return E_NETWORK_OK;
}

static teNetworkStatus IotcNetworkHandleSocketDisconnect(int iSocketFd, json_object *psJsonMessage)
{
    DBG_vPrintf(DBG_NETWORK, "IotcNetworkHandleSocketDisconnect\n");

    if(NULL == psJsonMessage)
    {
        ERR_vPrintf(T_TRUE, "The paramer is error\n");
        return E_NETWORK_ERROR_PARAM;
    }
    IotcDeviceRemoveSocket(iSocketFd);

    return E_NETWORK_OK;
}

static teNetworkStatus IotcNetWorkHandleRecvMessage(int iSocketFd, json_object *psJsonMessage)
{
    DBG_vPrintf(DBG_NETWORK, "IotcNetWorkHandleRecvMessage\n");

    if(NULL == psJsonMessage)
    {
        ERR_vPrintf(T_TRUE, "The paramer is error\n");
    }
    
    uint16 u16MessageType = 0;
    json_object *psJsonTemp = NULL;
    if (NULL != (psJsonTemp = json_object_object_get(psJsonMessage, paKeySequenceNo)))
    {
        if (NULL != (psJsonTemp = json_object_object_get(psJsonMessage, paKeyMessageType)))
        {
            u16MessageType = json_object_get_int(psJsonTemp);
        }
        else
        {
            ERR_vPrintf(T_TRUE, "Json Format Error\n");
            return E_NETWORK_ERROR_FORMAT;
        }
    }
    else
    {
        ERR_vPrintf(T_TRUE, "Json Format Error\n");
        return E_NETWORK_ERROR_FORMAT;
    }
    
    switch(u16MessageType)
    {
        case(E_COMMAND_REPORT_DEVICES_LIST):
        {
            IotcNetworkHandleDevicesReport(iSocketFd, psJsonMessage);
        }
        break;
        case(E_COMMAND_SOCKET_DISCONNECT):
        {
            IotcNetworkHandleSocketDisconnect(iSocketFd, psJsonMessage);
        }
        break;
        default:
            break;
    }
    
    return E_NETWORK_OK;
}

static void *IotcNetworkHandleThread(void *arg)
{
    BLUE_vPrintf(DBG_NETWORK, "IotcNetworkHandleThread\n");
    sIotcNetwork.eThreadState = E_THREAD_RUNNING;
    signal(THREAD_SIGNAL, thread_signal_handler);

    while(sIotcNetwork.eThreadState)
    {
        sched_yield();
        pthread_mutex_lock(&sSocketEventQuene.mutex);
        while (!sSocketEventQuene.flag_data)
        {
            GREEN_vPrintf(DBG_NETWORK, "pthread_cond_waiting ...\n");
            pthread_cond_wait(&sSocketEventQuene.cond_data_available, &sSocketEventQuene.mutex);
            GREEN_vPrintf(DBG_NETWORK, "pthread_cond_wait wakeup ...\n");
        }
        DBG_vPrintf(DBG_NETWORK, "The Data Recv is %s\n", sSocketEventQuene.sSocketClient.csClientData);

        uint16 u16EventType = 0;
        json_object *psJsonRecvMessage = NULL, *psJsonTemp = NULL;
        if (NULL != (psJsonRecvMessage = json_tokener_parse(sSocketEventQuene.sSocketClient.csClientData)))
        {
            if (NULL != (psJsonTemp = json_object_object_get(psJsonRecvMessage, paKeyEventType)))
            {
                u16EventType = json_object_get_int(psJsonTemp);
                DBG_vPrintf(DBG_NETWORK, "This Event is %d Event\n", u16EventType);
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

        if(E_IOTC_EVENT_DEVICE != u16EventType)
        {
            DBG_vPrintf(DBG_NETWORK, "This Event is not A Device Event, Push it Again\n");
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
        
        IotcNetWorkHandleRecvMessage(iSocketFd, psJsonRecvMessage);
        
        json_object_put(psJsonRecvMessage);
        sleep(0);
    }

done:
    DBG_vPrintf(DBG_NETWORK, "Exit IotcNetworkHandleThread\n");
    pthread_exit("Get Killed Signal");
}


/****************************************************************************/
/***        End           file                                            ***/
/****************************************************************************/

