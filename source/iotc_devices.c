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
#include "iotc_devices.h"

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
static teIotcStatus IotcDeviceAddService(tsIotcDevice *psIotcDevice, uint16 u16DeviceID);
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

teIotcStatus IotcDeviceAdd(char *paDeviceName, uint16 u16DeviceID, uint64 u64DeviceIndex)
{
    DBG_vPrintf(DBG_DEVICE, "IotcDeviceAdd\n");
        
    pthread_mutex_lock(&sIotcDeviceHead.mutex);
    tsIotcDevice *psIotcDeviceTemp = NULL;
    dl_list_for_each(psIotcDeviceTemp, &sIotcDeviceHead.list, tsIotcDevice, list)
    {
        if (u64DeviceIndex == psIotcDeviceTemp->u64DeviceIndex)
        {
            DBG_vPrintf(DBG_DEVICE, "The Device is Exist, %llu\n", psIotcDevice->u64DeviceIndex);
            psIotcDevice->blDeviceOnline = T_TRUE;
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
    psIotcDeviceNew->u64DeviceIndex = psIotcDevice->u64DeviceIndex;
    psIotcDeviceNew->blDeviceOnline = T_TRUE;

    if(E_IOTC_OK != IotcDeviceAddService(psIotcDeviceNew, u16DeviceID))
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
/****************************************************************************/
/***        Locate   Functions                                            ***/
/****************************************************************************/
static teIotcStatus IotcDeviceAddService(tsIotcDevice *psIotcDevice, uint16 u16DeviceID, char *paDeviceName)
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
            psIotcDevice->psDeviceServer = psDeviceSwitchLight;

            if(NULL != paDeviceName)
            {
               memcpy(psIotcDeviceNew->pcDeviceName, psDeviceName, strlen(psDeviceName));
            }
            else
            {
                memcpy(psIotcDeviceNew->pcDeviceName, "SwitchLight-%04x", u64DeviceIndex);
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




