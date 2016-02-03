/****************************************************************************
 *
 * MODULE:             Zigbee - JIP daemon
 *
 * COMPONENT:          Utils interface
 *
 * REVISION:           $Revision: 43420 $
 *
 * DATED:              $Date: 2015-10-01 15:13:17 +0100 (Mon, 18 Jun 2012) $
 *
 * AUTHOR:             PCT
 *
 ****************************************************************************
 *
 * Copyright Tonly B.V. 2015. All rights reserved
 *
 ***************************************************************************/

#ifndef __H_UTILS_H_
#define __H_UTILS_H_

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
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <malloc.h>
#include <syslog.h>

#define UI_BLACK(x)     "\e[30;1m"x"\e[0m"
#define UI_RED(x)       "\e[31;1m"x"\e[0m"
#define UI_GREEN(x)     "\e[32;1m"x"\e[0m"
#define UI_YELLOW(x)    "\e[33;1m"x"\e[0m"
#define UI_BLUE(x)      "\e[34;1m"x"\e[0m"
#define UI_PURPLE(x)    "\e[35;1m"x"\e[0m"
#define UI_CYAN(x)      "\e[36;1m"x"\e[0m"
#define UI_WHITE(x)     "\e[37;1m"x"\e[0m"

#define DBG_vPrintf(a,b,ARGS...)  do {  if (a) {printf(UI_BLUE	("[IOT_%d]") b, __LINE__, ## ARGS);} \
    /*else {syslog(LOG_DEBUG, "[IOT_%d]" b, __LINE__, ## ARGS);}*/} while(0)
#define INF_vPrintf(a,b,ARGS...)  do {  if (a) {printf(UI_YELLOW("[IOT_%d]") b, __LINE__, ## ARGS);} \
    /*else {syslog(LOG_INFO,  "[IOT_%d]" b, __LINE__, ## ARGS);}*/} while(0)
#define NOT_vPrintf(a,b,ARGS...)  do {  if (a) {printf(UI_GREEN	("[IOT_%d]") b, __LINE__, ## ARGS);} \
    /*else {syslog(LOG_NOTICE,"[IOT_%d]" b, __LINE__, ## ARGS);}*/} while(0)
#define WAR_vPrintf(a,b,ARGS...)  do {  if (a) {printf(UI_PURPLE("[IOT_%d]") b, __LINE__, ## ARGS);} \
    /*else {syslog(LOG_WARNING, "[IOT_%d]" b, __LINE__, ## ARGS);}*/} while(0)
#define ERR_vPrintf(a,b,ARGS...)  do {  if (a) {printf(UI_RED	("[IOT_%s:%d]") b, __FILE__, __LINE__, ## ARGS);} \
    /*syslog(LOG_ERR,  "[IOT_%s:%d]" b, __FILE__, __LINE__, ## ARGS);*/} while(0)

#define MIBF 256
#define MDBF 1024 
#define MXBF 2048

#define CheckError(value,comp,ret) do{if(value != comp){ERR_vPrintf(T_TRUE, "err:%s\n", strerror(errno));return ret;}}while(0)
#define CheckNull(x,y) do{if(x == NULL){return y;}}while(0)

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
typedef unsigned char       uint8;
typedef unsigned short      uint16; 
typedef unsigned int        uint32;
typedef unsigned long long  uint64;

typedef enum
{
    T_FALSE = 0,
    T_TRUE  = 1,
}bool_t;

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

#endif /*__H_UTILS_H_*/
