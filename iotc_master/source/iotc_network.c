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
static void IotcNetworkHandleSocketDisconnect(void *psUser, void *pvMessage, uint16 u16Length);
/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
static tsIotcNetwork sIotcNetwork;

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
    pthread_kill(sIotcNetwork.pthIotcNetwork, THREAD_NETWORK_SIGNAL);
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

static void IotcNetworkHandleSocketDisconnect(void *psUser, void *pvMessage, uint16 u16Length)
{
    DBG_vPrintf(DBG_NETWORK, "IotcNetworkHandleSocketDisconnect\n");

    if(NULL == psUser)
    {
        ERR_vPrintf(T_TRUE, "The paramer is error\n");
        return;
    }
    //int iSocketClientFd = *((int*)psUser);

}

static teNetworkStatus IotcNetWorkHandleRecvMessage(int iSocketFd, char *paMessage)
{
    uint16 u16MessageType = 0;
    json_object *psJsonRecvMessage = NULL, *psJsonTemp = NULL;
    if (NULL != (psJsonRecvMessage = json_tokener_parse(paMessage)))
    {
        if (NULL != (psJsonTemp = json_object_object_get(psJsonRecvMessage, paKeySequenceNo)))
        {
            if (NULL != (psJsonTemp = json_object_object_get(psJsonRecvMessage, paKeyMessageType)))
            {
                u16MessageType = json_object_get_int(psJsonTemp);
            }
            else
            {
                ERR_vPrintf(T_TRUE, "Json Format Error\n");
                json_object_put(psJsonRecvMessage);
                return E_NETWORK_ERROR_FORMAT;
            }
        }
        else
        {
            ERR_vPrintf(T_TRUE, "Json Format Error\n");
            json_object_put(psJsonRecvMessage);
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
        case(E_COMMAND_DEVICES_REPORT):
        {
            IotcNetworkHandleDevicesReport(iSocketFd, psJsonRecvMessage);
        }
        break;
        default:
            break;
    }
    return E_NETWORK_OK;
}

static void ThreadSignalHandler(int sig)
{
    BLUE_vPrintf(DBG_NETWORK, "ThreadSignalHandler Used To Interrupt System Call\n");
    pthread_exit(&sIotcNetwork.pthIotcNetwork);
    pthread_mutex_unlock(&sSocketEventQuene.mutex);
}

static void *IotcNetworkHandleThread(void *arg)
{
    BLUE_vPrintf(DBG_NETWORK, "IotcNetworkHandleThread\n");
    sIotcNetwork.eThreadState = E_THREAD_RUNNING;
    signal(THREAD_NETWORK_SIGNAL, ThreadSignalHandler);

    while(sIotcNetwork.eThreadState)
    {
        pthread_mutex_lock(&sSocketEventQuene.mutex);
        while (!sSocketEventQuene.flag)
        {
            GREEN_vPrintf(DBG_NETWORK, "pthread_cond_waiting ...\n");
            pthread_cond_wait(&sSocketEventQuene.cond_data_recv, &sSocketEventQuene.mutex);
        }
        pthread_mutex_unlock(&sSocketEventQuene.mutex);

        GREEN_vPrintf(DBG_NETWORK, "pthread_cond_waited ...\n");
        if(E_IOTC_EVENT_DEVICE != sSocketEventQuene.sSocketEvent.eSocketCondEvent)
        {
            DBG_vPrintf(DBG_NETWORK, "This Event is not A Device Event, Push it Again\n");
            pthread_cond_broadcast(&sSocketEventQuene.cond_data_recv); 
            continue;
        }
        
        int iSocketFd = sSocketEventQuene.sSocketEvent.uCondData.sSocketData.iSocketFd;
        char *paEventBuffer = (char*)malloc(sSocketEventQuene.sSocketEvent.uCondData.sSocketData.iSocketDataLen);
        if(NULL == paEventBuffer)
        {
            ERR_vPrintf(T_TRUE, "Can't Malloc Memory\n");
            goto done;
        }
        IotcNetWorkHandleRecvMessage(iSocketFd, paEventBuffer);
        free(paEventBuffer);
        
        sleep(0);
    }

done:    
    DBG_vPrintf(DBG_NETWORK, "Exit SocketServerHandleThread\n");
    pthread_exit("Get Killed Signal");
}


/****************************************************************************/
/***        End           file                                            ***/
/****************************************************************************/

