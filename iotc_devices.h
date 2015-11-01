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
}teIotcStatus;
    
    
typedef union
{
    uint8           u8Data;
    uint16          u16Data;
    uint32          u32Data;
    uint64          u64Data;
}tuDevAttributeData;

typedef struct _tsDeviceServer
{
    uint16          u16DeviceServerID;
    uint32          u32NumAttribute;
    uint16          *pu16Attributes;  
      
    uint32          u32NumCommands;
    uint8           *pau8Commands;   
}tsDeviceServer;

typedef struct _tsIotc_Device
{
    pthread_mutex_t mutex;
    char            cDeviceName[MXBF];
    uint16          u16DeviceID;
    bool_t          blDeviceOnline;
    
    uint64          u64DeviceIndex;
    
    uint16          u16NumServer;
    tsDeviceServer  *psDeviceServer;

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
teIotcStatus IotcDeviceAdd(tsIotcDevice *psIotcDevice);
teIotcStatus IotcDeviceRemove(tsIotcDevice *psIotcDevice);

teIotcStatus IotcDeviceServerAdd();
teIotcStatus IotcDeviceAttributeAdd();



/****************************************************************************/
/***        Locate   Functions                                            ***/
/****************************************************************************/

#if defined __cplusplus
}
#endif
#endif
