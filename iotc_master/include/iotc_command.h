
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
    E_COMMAND_DEVICES_SEARCH                = 0x0001,
    E_COMMAND_DEVICES_REPORT                = 0x0002,
    E_COMMAND_SET_ONOFF                     = 0x0003,
    E_COMMAND_SOCKET_DISCONNECT             = 0x0004,
}teIotcCommand;

typedef enum
{
    E_IOTC_EVENT_DEVICE,
    E_IOTC_EVENT_APP,
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
