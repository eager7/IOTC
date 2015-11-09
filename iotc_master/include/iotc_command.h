
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
    E_COMMAND_DEVICES_JION                  = 0x3001,
    E_COMMAND_REPORT_DEVICES_LIST           = 0x3002,
    E_COMMAND_RESPONSE_DEVICES_VERSION      = 0x3003,
    
    E_COMMAND_DEVICES_ATTRIBUTE_REPORT      = 0x3004,

    E_COMMAND_DEVICES_DEFAULT_RESPONSE      = 0x3005,
    E_COMMAND_DEVICES_SEARCH_RESPONSE      = 0x3006,
    E_COMMAND_DEVICES_LEAVE_REPORT      = 0x3007,
    E_COMMAND_SOCKET_DISCONNECT = 0x3008,

















    

    
}teIotcMessageType;

typedef enum
{
    E_IOTC_EVENT_APP                        = 0x0001,
    E_IOTC_EVENT_DEVICE                     = 0x0002,
    E_IOTC_EVENT_EXIT                       = 0x0003,
}teIotcEvnetType;
/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
#define paKeySequenceNo     "sequence_no"
#define paKeyEventType      "event_type"
#define paKeyMessageType    "message_type"
#define paKeyDescription    "description"
#define paKeyDeviceName     "device_name"
#define paKeyDeviceId       "device_id"
#define paKeyDeviceIndex    "device_index"
#define paKeyDeviceOnOff    "onoff"
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
