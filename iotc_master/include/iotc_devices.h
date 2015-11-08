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


#ifndef __H_IOTC_DEVICES_H_
#define __H_IOTC_DEVICES_H_

#if defined __cplusplus
extern "C"{
#endif
/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include <pthread.h>
#include "utils.h"
#include "list.h"
#include "iotc_device_type.h"
#include "socket_server.h"
/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/


/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
typedef enum
{
    E_IOTC_OK,
    E_IOTC_ERROR,
    E_IOTC_EXIST,
    E_IOTC_ERROR_MALLOC,
    E_IOTC_ERROR_PARAM,
}teIotcStatus;

typedef struct _tsIotc_Device
{
    pthread_mutex_t mutex;
    char            pcDeviceName[MXBF];
    uint16          u16DeviceID;
    bool_t          blDeviceOnline;
    
    uint64          u64DeviceIndex;
    void            *psDeviceServer;

    int             iSocketClientFd;

    struct dl_list  list;
}tsIotcDevice;

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/
extern tsIotcDevice sIotcDeviceHead;
/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
teIotcStatus IotcDeviceInit();
teIotcStatus IotcDeviceFinished();
teIotcStatus IotcDeviceAdd(char *paDeviceName, uint16 u16DeviceID, uint64 u64DeviceIndex, int iSocketClientFd);
teIotcStatus IotcDeviceRemove(tsIotcDevice *psIotcDevice);
teIotcStatus IotcDeviceRemoveSocket(int iSocketFd);



/****************************************************************************/
/***        Locate   Functions                                            ***/
/****************************************************************************/

#if defined __cplusplus
}
#endif
#endif
