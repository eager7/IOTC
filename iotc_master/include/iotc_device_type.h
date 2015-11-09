/****************************************************************************
 *
 * MODULE:             iotc_device_type.h
 *
 * COMPONENT:          Utils interface
 *
 * REVISION:           $Revision:  0$
 *
 * DATED:              $Date: 2015-10-21 15:13:17 +0100 (Thu, 21 Oct 2015 $
 *
 * AUTHOR:             PCT
 *
 ****************************************************************************
 *
 * Copyright panchangtao@gmail.com B.V. 2015. All rights reserved
 *
 ***************************************************************************/


#ifndef __H_IOTC_DEVICE_TYPE_H_
#define __H_IOTC_DEVICE_TYPE_H_

#if defined __cplusplus
extern "C"{
#endif
/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include "utils.h"
#include "iotc_command.h"
/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/


/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
typedef enum
{
    E_ATT_OK,
    E_ATT_ERROR,
}teAttStatus;

typedef enum
{
    E_ATTRIBUTE_ONOFF,
    E_ATTRIBUTE_LEVEL,
    E_ATTRIBUTE_COLOR,
}teAtttibuteType;

typedef union
{
    uint8     u8Data;
    uint16    u16Data;
    uint32    u32Data;
    uint64    u64Data;
} tuAttributeData;

typedef teAttStatus (*tprDeviceSetDeviceAttributeCallback)(int iSocketFd, uint64 u64DeviceIndex, uint8 u8Type, tuAttributeData uAttributeData, teAtttibuteType eAtttibuteType);
typedef teAttStatus (*tprDeviceGetDeviceAttributeCallback)(int iSocketFd, uint64 u64DeviceIndex, uint8 u8Type, tuAttributeData *puAttributeData, teAtttibuteType eAtttibuteType);

typedef struct _tsDeviceSwitchLight
{
    bool_t  OnOff;
    uint8   u8PowerValue;
    tprDeviceSetDeviceAttributeCallback prDeviceSetDeviceAttribute;
    tprDeviceGetDeviceAttributeCallback prDeviceGetDeviceAttribute;
}tsDeviceSwitchLight;

typedef struct _tsDeviceDimmerLight
{
    bool_t  OnOff;
    uint8   u8Level;
    uint8   u8PowerValue;
    tprDeviceSetDeviceAttributeCallback prDeviceSetDeviceAttribute;
    tprDeviceGetDeviceAttributeCallback prDeviceGetDeviceAttribute;
}tsDeviceDimmerLight;

typedef struct _tsDeviceColorLight
{
    bool_t  OnOff;
    uint8   u8Level;
    uint32  u32HsvValue;
    uint8   u8PowerValue;
    tprDeviceSetDeviceAttributeCallback prDeviceSetDeviceAttribute;
    tprDeviceGetDeviceAttributeCallback prDeviceGetDeviceAttribute;
}tsDeviceColorLight;

typedef struct _tsDeviceSmartPlug
{
    bool_t  OnOff;
    uint8   u8PowerValue;
    tprDeviceSetDeviceAttributeCallback prDeviceSetDeviceAttribute;
    tprDeviceGetDeviceAttributeCallback prDeviceGetDeviceAttribute;
}tsDeviceSmartPlug;

typedef struct _tsDeviceSensorLight
{
    uint16  u16LightValue;
    uint8   u8PowerValue;
    tprDeviceSetDeviceAttributeCallback prDeviceSetDeviceAttribute;
    tprDeviceGetDeviceAttributeCallback prDeviceGetDeviceAttribute;
}tsDeviceSensorLight;

typedef struct _tsDeviceSensorTempHumi
{
    bool_t  OnOff;
    uint8   u8PowerValue;
    tprDeviceSetDeviceAttributeCallback prDeviceSetDeviceAttribute;
    tprDeviceGetDeviceAttributeCallback prDeviceGetDeviceAttribute;
}tsDeviceSensorTempHumi;

typedef struct _tsDeviceSensorBinary
{
    bool_t  Status;
    uint8   u8PowerValue;
    tprDeviceSetDeviceAttributeCallback prDeviceSetDeviceAttribute;
    tprDeviceGetDeviceAttributeCallback prDeviceGetDeviceAttribute;
}tsDeviceSensorBinary;

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

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
