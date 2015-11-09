
/****************************************************************************
 *
 * MODULE:             command.h
 *
 * COMPONENT:          Utils interface
 *
 * REVISION:           $Revision:  0$
 *
 * DATED:              $Date: 2015-11-04 15:13:17 +0100 (Thu, 21 Oct 2015 $
 *
 * AUTHOR:             PCT
 *
 ****************************************************************************
 *
 * Copyright panchangtao@gmail.com B.V. 2015. All rights reserved
 *
 ***************************************************************************/


#ifndef __H_IOTC_COMMAND_H_
#define __H_IOTC_COMMAND_H_

#if defined __cplusplus
extern "C"{
#endif
/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/


/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
typedef enum
{
    E_COMMAND_DEVICES_JION                  = 0x1001,
    E_COMMAND_DEFAULT_RESPONSE              = 0x8001,
    E_COMMAND_REPORT_DEVICES_LIST           = 0x1002,

    E_COMMAND_REQUEST_VIRSION               = 0x8003,
    E_COMMAND_RESPONSE_VERSION              = 0x1003,
    
    E_COMMAND_REPORT_DEVICE_ATTRIBUTE       = 0x1004,
    E_COMMAND_REPORT_DEVICE_LEAVE           = 0x1005,

    E_COMMAND_REQUEST_DEVICE_ATTRIBUTE      = 0x8006,
    E_COMMAND_RESPONSE_DEVICE_ATTRIBUTE     = 0x1006,

    E_COMMAND_REQUEST_DEVICES_SEARCH        = 0x8007,
    E_COMMAND_RESPONSE_DEVICES_SEARCH       = 0x1007,

    E_COMMAND_SET_DEVICE_ONOFF              = 0x8008,
    E_COMMAND_SET_DEVICE_BRIGHTNESS         = 0x8009,
    E_COMMAND_SET_DEVICE_COLOR              = 0x800A,

    //internel command   
    E_COMMAND_SOCKET_DISCONNECT = 0x3001,   
}teIotcMessageType;

typedef enum
{
    E_IOTC_EVENT_APP                        = 0x0001,
    E_IOTC_EVENT_DEVICE                     = 0x0002,
    E_IOTC_EVENT_EXIT                       = 0x0003,
}teIotcEvnetType;

typedef enum _teIotcDeviceType
{
    E_DEVICE_SWITCH_LIGHT                   = 0x0001,
    E_DEVICE_DIMMER_LIGHT                   = 0x0002,
    E_DEVICE_COLOR_LIGHT                    = 0x0003,

    E_DEVICE_SMART_PLUG                     = 0x0004,
    E_DEVICE_SENSOR_LIGHT                   = 0x0005,
    E_DEVICE_SENSOR_TEMPHUMI                = 0x0006,
    E_DEVICE_SENSOR_BINARY                  = 0x0007,
}teIotcDeviceType;

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
#define paKeySequenceNo         "sequence_no"
#define paKeyEventType          "event_type"
#define paKeyMessageType        "message_type"
#define paKeyDescription        "description"
#define paKeyDeviceName         "device_name"
#define paKeyDeviceId           "device_id"
#define paKeyDeviceIndex        "device_index"
#define paKeyDeviceOnOff        "onoff"
#define paKeyDeviceBrightness   "brightness"
#define paKeyDeviceColor        "color"
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
