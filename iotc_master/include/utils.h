/****************************************************************************
 *
 * MODULE:             utils.h
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
#define UI_BLACK(x)     "\e[30;1m"x"\e[0m"
#define UI_RED(x)       "\e[31;1m"x"\e[0m"
#define UI_GREEN(x)     "\e[32;1m"x"\e[0m"
#define UI_YELLOW(x)    "\e[33;1m"x"\e[0m"
#define UI_BLUE(x)      "\e[34;1m"x"\e[0m"
#define UI_PURPLE(x)    "\e[35;1m"x"\e[0m"
#define UI_CYAN(x)      "\e[36;1m"x"\e[0m"
#define UI_WHITE(x)     "\e[37;1m"x"\e[0m"

#define DBG_vPrintf(a,b,ARGS...)    do {  if (a) {printf(UI_WHITE    ("[IOT_%d]") b, __LINE__, ## ARGS);} } while(0)
#define BLACK_vPrintf(a,b,ARGS...)  do {  if (a) {printf(UI_BLACK    ("[IOT_%d]") b, __LINE__, ## ARGS);} } while(0)
#define RED_vPrintf(a,b,ARGS...)    do {  if (a) {printf(UI_RED      ("[IOT_%d]") b, __LINE__, ## ARGS);} } while(0)
#define GREEN_vPrintf(a,b,ARGS...)  do {  if (a) {printf(UI_GREEN    ("[IOT_%d]") b, __LINE__, ## ARGS);} } while(0)
#define YELLOW_vPrintf(a,b,ARGS...) do {  if (a) {printf(UI_YELLOW   ("[IOT_%d]") b, __LINE__, ## ARGS);} } while(0)
#define BLUE_vPrintf(a,b,ARGS...)   do {  if (a) {printf(UI_BLUE     ("[IOT_%d]") b, __LINE__, ## ARGS);} } while(0)
#define PURPLE_vPrintf(a,b,ARGS...) do {  if (a) {printf(UI_PURPLE   ("[IOT_%d]") b, __LINE__, ## ARGS);} } while(0)
#define CYAN_vPrintf(a,b,ARGS...)   do {  if (a) {printf(UI_CYAN     ("[IOT_%d]") b, __LINE__, ## ARGS);} } while(0)
#define WHITE_vPrintf(a,b,ARGS...)  do {  if (a) {printf(UI_WHITE    ("[IOT_%d]") b, __LINE__, ## ARGS);} } while(0)

#define ERR_vPrintf RED_vPrintf

#define THREAD_SIGNAL SIGUSR1

#define MIBF 256
#define MDBF 1024 
#define MXBF 2048

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

typedef volatile enum
{
    E_THREAD_STOPPED, 
    E_THREAD_RUNNING,  
    E_THREAD_STOPPING, 
}teThreadState;  

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
void UtilsRegisterSignalHandler();
void thread_signal_handler(int sig);

/****************************************************************************/
/***        Locate   Functions                                            ***/
/****************************************************************************/

#if defined __cplusplus
}
#endif
#endif
