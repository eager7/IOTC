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
static void IotcNetworkHandleDevicesReport(void *psUser, void *pvMessage, uint16 u16Length);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
teNetworkStatus IotcNetworkInit()
{
    DBG_vPrintf(DBG_NETWORK, "IotcNetworkInit\n");

    IotcDeviceInit();    
    SocketCallBackListenerAdd(E_COMMAND_DEVICES_REPORT, IotcNetworkHandleDevicesReport);    
    return E_NETWORK_OK;
}

teNetworkStatus IotcNetworkFinished()
{
    DBG_vPrintf(DBG_NETWORK, "IotcNetworkFinished\n");

    IotcDeviceFinished();    
    
    return E_NETWORK_OK;
}

/****************************************************************************/
/***        Locate   Functions                                            ***/
/****************************************************************************/
static void IotcNetworkHandleDevicesReport(void *psUser, void *pvMessage, uint16 u16Length)
{
    DBG_vPrintf(DBG_NETWORK, "IotcNetworkHandleDevicesReport\n");

    if(NULL == pvMessage)
    {
        ERR_vPrintf(T_TRUE, "The paramer is error\n");
        return;
    }
    json_object *psJsonMessage = (struct json_object*)pvMessage;
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
            IotcDeviceAdd(char *paDeviceName, uint16 u16DeviceID, uint64 u64DeviceIndex, tsSocketClient *psSocketClient)
            {

            }
        }
    }
}

/****************************************************************************/
/***        End           file                                            ***/
/****************************************************************************/

