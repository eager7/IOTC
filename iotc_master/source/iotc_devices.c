/****************************************************************************
 *
 * MODULE:             iotc_device.h
 *
 * COMPONENT:          IOTC Devices interface
 *
 * REVISION:           $Revision:  0$
 *
 * DATED:              $Date: 2015-10-31 15:13:17 +0100 (Thu, 21 Oct 2015 $
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
#include <json/json.h>
#include "iotc_devices.h"
#include "socket_server.h"
/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#define DBG_DEVICE 1

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/
static teIotcStatus IotcDeviceAddService(tsIotcDevice *psIotcDevice, uint16 u16DeviceID, char *psDeviceName, uint64 u64DeviceIndex);
static teAttStatus  IotcDeviceSetDeviceAttribute(int iSocketFd, uint64 u64DeviceIndex, uint8 u8Type, tuAttributeData uAttributeData, teAtttibuteType eAtttibuteType);
/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
tsIotcDevice sIotcDeviceHead;
/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
teIotcStatus IotcDeviceInit()
{
    DBG_vPrintf(DBG_DEVICE, "IotcDeviceInit\n");
    
    memset(&sIotcDeviceHead, 0, sizeof(sIotcDeviceHead));
    pthread_mutex_init(&sIotcDeviceHead.mutex, NULL);
    dl_list_init(&sIotcDeviceHead.list);
    
    return E_IOTC_OK;
}

teIotcStatus IotcDeviceFinished()
{
    DBG_vPrintf(DBG_DEVICE, "IotcDeviceFinished\n");
    
    pthread_mutex_lock(&sIotcDeviceHead.mutex);
    tsIotcDevice *psIotcDeviceTemp1 = NULL, *psIotcDeviceTemp2 = NULL;
    dl_list_for_each_safe(psIotcDeviceTemp1, psIotcDeviceTemp2, &sIotcDeviceHead.list, tsIotcDevice, list)
    {
        dl_list_del(&psIotcDeviceTemp1->list);
        free(psIotcDeviceTemp1->psDeviceServer);
        free(psIotcDeviceTemp1);
        psIotcDeviceTemp1 = NULL;
    }    
    pthread_mutex_unlock(&sIotcDeviceHead.mutex);
    pthread_mutex_destroy(&sIotcDeviceHead.mutex);
    
    return E_IOTC_OK;
}

teIotcStatus IotcDeviceAdd(char *paDeviceName, uint16 u16DeviceID, uint64 u64DeviceIndex, int iSocketClientFd)
{
    DBG_vPrintf(DBG_DEVICE, "IotcDeviceAdd\n");
    
    pthread_mutex_lock(&sIotcDeviceHead.mutex);
    tsIotcDevice *psIotcDeviceTemp = NULL;
    dl_list_for_each(psIotcDeviceTemp, &sIotcDeviceHead.list, tsIotcDevice, list)
    {
        if (u64DeviceIndex == psIotcDeviceTemp->u64DeviceIndex)
        {
            DBG_vPrintf(DBG_DEVICE, "The Device is Exist, 0x%04llx\n", u64DeviceIndex);
            psIotcDeviceTemp->blDeviceOnline = T_TRUE;
            pthread_mutex_unlock(&sIotcDeviceHead.mutex);
            return E_IOTC_EXIST;
        }
    }
    
    tsIotcDevice *psIotcDeviceNew = NULL;
    psIotcDeviceNew = (tsIotcDevice*)malloc(sizeof(tsIotcDevice));
    if(NULL == psIotcDeviceNew)
    {
        ERR_vPrintf(T_TRUE, "Malloc New Memory Failed\n");
        pthread_mutex_unlock(&sIotcDeviceHead.mutex);
        return E_IOTC_ERROR_MALLOC;
    }
    
    psIotcDeviceNew->u16DeviceID = u16DeviceID;
    psIotcDeviceNew->u64DeviceIndex = u64DeviceIndex;
    psIotcDeviceNew->iSocketClientFd = iSocketClientFd;
    psIotcDeviceNew->blDeviceOnline = T_TRUE;

    if(E_IOTC_OK != IotcDeviceAddService(psIotcDeviceNew, u16DeviceID, paDeviceName, u64DeviceIndex))
    {
        ERR_vPrintf(T_TRUE, "Malloc New Memory Failed\n");
        pthread_mutex_unlock(&sIotcDeviceHead.mutex);
        return E_IOTC_ERROR_MALLOC;      
    }
    pthread_mutex_init(&psIotcDeviceNew->mutex, NULL);    
    dl_list_add_tail(&sIotcDeviceHead.list, &psIotcDeviceNew->list);
    pthread_mutex_unlock(&sIotcDeviceHead.mutex);
    
    return E_IOTC_OK;
}

teIotcStatus IotcDeviceRemove(tsIotcDevice *psIotcDevice)
{
    DBG_vPrintf(DBG_DEVICE, "IotcDeviceRemove\n");
    
    if (NULL == psIotcDevice)
    {
        return E_IOTC_ERROR;
    }
    
    pthread_mutex_lock(&sIotcDeviceHead.mutex);
    tsIotcDevice *psIotcDeviceTemp1 = NULL, *psIotcDeviceTemp2 = NULL;
    dl_list_for_each_safe(psIotcDeviceTemp1, psIotcDeviceTemp2, &sIotcDeviceHead.list, tsIotcDevice, list)
    {
        if (psIotcDevice->u64DeviceIndex == psIotcDeviceTemp1->u64DeviceIndex)
        {
            DBG_vPrintf(DBG_DEVICE, "The Device is Found, %llu\n", psIotcDevice->u64DeviceIndex);
            dl_list_del(&psIotcDeviceTemp1->list);
            free(psIotcDeviceTemp1->psDeviceServer);
            free(psIotcDeviceTemp1);
            psIotcDeviceTemp1 = NULL;
        }
    }
    
    pthread_mutex_unlock(&sIotcDeviceHead.mutex);

    return E_IOTC_OK;
}

teIotcStatus IotcDeviceRemoveSocket(int iSocketFd)
{
    DBG_vPrintf(DBG_DEVICE, "IotcDeviceRemoveSocket\n");
        
    pthread_mutex_lock(&sIotcDeviceHead.mutex);
    tsIotcDevice *psIotcDeviceTemp1 = NULL, *psIotcDeviceTemp2 = NULL;
    dl_list_for_each_safe(psIotcDeviceTemp1, psIotcDeviceTemp2, &sIotcDeviceHead.list, tsIotcDevice, list)
    {
        if (iSocketFd == psIotcDeviceTemp1->iSocketClientFd)
        {
            dl_list_del(&psIotcDeviceTemp1->list);
            free(psIotcDeviceTemp1->psDeviceServer);
            free(psIotcDeviceTemp1);
            psIotcDeviceTemp1 = NULL;
        }
    }
    
    pthread_mutex_unlock(&sIotcDeviceHead.mutex);

    return E_IOTC_OK;
}

/****************************************************************************/
/***        Locate   Functions                                            ***/
/****************************************************************************/
static teIotcStatus IotcDeviceAddService(tsIotcDevice *psIotcDevice, uint16 u16DeviceID, char *psDeviceName, uint64 u64DeviceIndex)
{
    DBG_vPrintf(DBG_DEVICE, "IotcDeviceAddService\n");    
    if (NULL == psIotcDevice)
    {
        return E_IOTC_ERROR;
    }

    switch(u16DeviceID)
    {
        case(E_DEVICE_SWITCH_LIGHT):
        {
            tsDeviceSwitchLight *psDeviceSwitchLight = (tsDeviceSwitchLight*)malloc(sizeof(tsDeviceSwitchLight));
            if(NULL == psDeviceSwitchLight)
            {
                return E_IOTC_ERROR;
            }    
            psDeviceSwitchLight->prDeviceSetDeviceAttribute = IotcDeviceSetDeviceAttribute;
            psIotcDevice->psDeviceServer = psDeviceSwitchLight;

            if(NULL != psDeviceName)
            {
               memcpy(psIotcDevice->pcDeviceName, psDeviceName, strlen(psDeviceName));
            }
            else
            {
                memcpy(psIotcDevice->pcDeviceName, "SwitchLight-%04x", u64DeviceIndex);
            }
        }
        break;
        case(E_DEVICE_DIMMER_LIGHT):
        {
            tsDeviceDimmerLight *psDeviceDimmerLight = (tsDeviceDimmerLight*)malloc(sizeof(tsDeviceDimmerLight));
            if(NULL == psDeviceDimmerLight)
            {
                return E_IOTC_ERROR;
            }    
            psIotcDevice->psDeviceServer = psDeviceDimmerLight;

            if(NULL != psDeviceName)
            {
               memcpy(psIotcDevice->pcDeviceName, psDeviceName, strlen(psDeviceName));
            }
            else
            {
                memcpy(psIotcDevice->pcDeviceName, "DimmerLight-%04x", u64DeviceIndex);
            }
        }
        break;    
        case(E_DEVICE_COLOR_LIGHT):
        {
            tsDeviceColorLight *psDeviceColorLight = (tsDeviceColorLight*)malloc(sizeof(tsDeviceColorLight));
            if(NULL == psDeviceColorLight)
            {
                return E_IOTC_ERROR;
            }    
            psIotcDevice->psDeviceServer = psDeviceColorLight;

            if(NULL != psDeviceName)
            {
               memcpy(psIotcDevice->pcDeviceName, psDeviceName, strlen(psDeviceName));
            }
            else
            {
                memcpy(psIotcDevice->pcDeviceName, "ColorLight-%04x", u64DeviceIndex);
            }
        }
        break;      
        case(E_DEVICE_SMART_PLUG):
        {
            tsDeviceSmartPlug *psDeviceSmartPlug = (tsDeviceSmartPlug*)malloc(sizeof(tsDeviceSmartPlug));
            if(NULL == psDeviceSmartPlug)
            {
                return E_IOTC_ERROR;
            }    
            psIotcDevice->psDeviceServer = psDeviceSmartPlug;

            if(NULL != psDeviceName)
            {
               memcpy(psIotcDevice->pcDeviceName, psDeviceName, strlen(psDeviceName));
            }
            else
            {
                memcpy(psIotcDevice->pcDeviceName, "SmartPlug-%04x", u64DeviceIndex);
            }
        }
        break;    
        case(E_DEVICE_SENSOR_LIGHT):
        {
            tsDeviceSensorLight *psDeviceSensorLight = (tsDeviceSensorLight*)malloc(sizeof(tsDeviceSensorLight));
            if(NULL == psDeviceSensorLight)
            {
                return E_IOTC_ERROR;
            }    
            psIotcDevice->psDeviceServer = psDeviceSensorLight;

            if(NULL != psDeviceName)
            {
               memcpy(psIotcDevice->pcDeviceName, psDeviceName, strlen(psDeviceName));
            }
            else
            {
                memcpy(psIotcDevice->pcDeviceName, "SensorLight-%04x", u64DeviceIndex);
            }
        }
        break;   
        case(E_DEVICE_SENSOR_TEMPHUMI):
        {
            tsDeviceSensorTempHumi *psDeviceSensorTempHumi = (tsDeviceSensorTempHumi*)malloc(sizeof(tsDeviceSensorTempHumi));
            if(NULL == psDeviceSensorTempHumi)
            {
                return E_IOTC_ERROR;
            }    
            psIotcDevice->psDeviceServer = psDeviceSensorTempHumi;

            if(NULL != psDeviceName)
            {
               memcpy(psIotcDevice->pcDeviceName, psDeviceName, strlen(psDeviceName));
            }
            else
            {
                memcpy(psIotcDevice->pcDeviceName, "SensorTempHumi-%04x", u64DeviceIndex);
            }
        }
        break;   
        case(E_DEVICE_SENSOR_BINARY):
        {
            tsDeviceSensorBinary *psDeviceSensorBinary = (tsDeviceSensorBinary*)malloc(sizeof(tsDeviceSensorBinary));
            if(NULL == psDeviceSensorBinary)
            {
                return E_IOTC_ERROR;
            }    
            psIotcDevice->psDeviceServer = psDeviceSensorBinary;

            if(NULL != psDeviceName)
            {
               memcpy(psIotcDevice->pcDeviceName, psDeviceName, strlen(psDeviceName));
            }
            else
            {
                memcpy(psIotcDevice->pcDeviceName, "SensorBinary-%04x", u64DeviceIndex);
            }
        }
        break;  
        default:
        {

        }
        break;
    }

    return E_IOTC_OK;
}


static teAttStatus IotcDeviceSetDeviceAttribute(int iSocketFd, uint64 u64DeviceIndex, uint8 u8Type, tuAttributeData uAttributeData, teAtttibuteType eAtttibuteType)
{
    switch (eAtttibuteType)
    {
        case(E_ATTRIBUTE_ONOFF):
        {
            json_object *psJsonMessage = json_object_new_object();
            json_object_object_add(psJsonMessage, paKeyDeviceOnOff, json_object_new_int(uAttributeData.u8Data));
            //struct timeval sNow;
            //gettimeofday(&sNow, NULL);
            //srand((unsigned)time(NULL));
            //int iSequeneNo = rand()%10000; //Scattered all requests by used rand
            SocketClientSendMessage(iSocketFd, (char*)json_object_get_string(psJsonMessage), strlen(json_object_get_string(psJsonMessage)));
        }
        break;
        case(E_ATTRIBUTE_LEVEL):
        {

        }
        break;
        case(E_ATTRIBUTE_COLOR):
        {

        }
        break;
    }
    return E_ATT_OK;
}


