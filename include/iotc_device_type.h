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

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/


/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
typedef enum _teIotcDeviceType
{
	E_DEVICE_SWITCH_LIGHT 		= 0x0001,
	E_DEVICE_DIMMER_LIGHT		= 0x0002,
	E_DEVICE_COLOR_LIGHT		= 0x0003,

	E_DEVICE_SMART_PLUG			= 0x0004,
	E_DEVICE_SENSOR_LIGHT		= 0x0005,
	E_DEVICE_SENSOR_TEMPHUMI	= 0x0006,
	E_DEVICE_SENSOR_BINARY		= 0x0007,
}teIotcDeviceType;

typedef struct _tsDeviceSwitchLight
{
	bool_t 	OnOff;	
	uint8 	u8PowerValue;
}tsDeviceSwitchLight;

typedef struct _tsDeviceDimmerLight
{
	bool_t 	OnOff;	
	uint8 	u8Level;
	uint8 	u8PowerValue;
}tsDeviceDimmerLight;

typedef struct _tsDeviceColorLight
{
	bool_t 	OnOff;	
	uint8   u8Level;
	uint32	u32HsvValue;
	uint8 	u8PowerValue;	
}tsDeviceColorLight;

typedef struct _tsDeviceSmartPlug
{
	bool_t 	OnOff;	
	uint8 	u8PowerValue;	
}tsDeviceSmartPlug;

typedef struct _tsDeviceSensorLight
{
	uint16 	u16LightValue;	
	uint8 	u8PowerValue;		
}tsDeviceSensorLight;

typedef struct _tsDeviceSensorTempHumi
{
	bool_t 	OnOff;	
	uint8 	u8PowerValue;
}tsDeviceSensorTempHumi;

typedef struct _tsDeviceSensorBinary
{
	bool_t 	Status;	
	uint8 	u8PowerValue;
}tsDeviceSwitchLight;

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
