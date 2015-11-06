/****************************************************************************
 *
 * MODULE:             service.h
 *
 * COMPONENT:          Utils interface
 *
 * REVISION:           $Revision:  0$
 *
 * DATED:              $Date: 2015-10-21 15:13:17 +0100 (Thu, 21 Oct 2015 $
 *
 * AUTHOR:             Abel
 *
 ****************************************************************************
 *
 * Copyright IOTC B.V. 2015. All rights reserved
 *
 ***************************************************************************/


#ifndef __H_IOTC_SERVER_H_
#define __H_IOTC_SERVER_H_

#if defined __cplusplus
extern "C"{
#endif




/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include "utils.h"
#include "list.h"


/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#define 


/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
typedef enum
{
    E_IOCTP_OK = 0,
    E_IOCTP_ERROR,
    E_IOCTP_ARG_ERROR
}eIoctProtolStatus;



typedef struct _Descriptor
{
	char *url;
	char *descriptor;
	struct dl_list list;
}Descriptor, DescriptorList;



typedef struct _IotcAttribute
{
	char *name;
	char *value;
	struct dl_list list;
}IotcAttribute, IotcAttributeList;


typedef struct _IotcArgument
{
	char *name;
	char *value;
	struct dl_list list;
}IotcArgument, IotcArgumentList;


typedef struct _IotcService
{
	char *name;
	IotcArgumentList argList;
	struct dl_list list;
}IotcService, IotcServiceList;


typedef struct _IotcDevice
{
	char *name; 			//device name
	char *mName;			//Module name
	char *DevNum;
	IotcAttribute attrList;
	IotcServiceList serList;
	struct dl_list list;
	pthread_mutex_t mutex;
}IotcDevice, IotcDeviceList;




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





#if defined __cplusplus
}
#endif
#endif

