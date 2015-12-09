/****************************************************************************
 *
 * MODULE:             iotc_broadcast.c
 *
 * COMPONENT:          APP interface
 *
 * REVISION:           $Revision:  0$
 *
 * DATED:              $Date: 2015-11-08 15:13:17 +0100 (Thu, 21 Oct 2015 $
 *
 * AUTHOR:             PCT
 *
 ****************************************************************************
 *
 * Copyright panchangtao@gmail.com B.V. 2015. All rights reserved
 *
 ***************************************************************************/

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/epoll.h> 
#include <stdlib.h>
#include <sys/time.h>
#include <net/if.h>
#include <netinet/in.h>
#include <linux/sockios.h>

#include "utils.h"
#include "iotc_broadcast.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#define DBG_BROAD 1

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/
static void *tfIotcBroadcastThread(void *arg);
static teBroadStatus IotcBroadcastSocketInit(int iPort, char *paBroadAddress);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
static tsIotcBroadcast sIotcBroadcast;
/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
teBroadStatus IotcBroadcastInit()
{
    DBG_vPrintf(DBG_BROAD, "IotcMulticastInit\n");

    memset(&sIotcBroadcast, 0, sizeof(tsIotcBroadcast));
    BLUE_vPrintf(DBG_BROAD, "pthread_create\n");

    if(E_BROAD_OK != IotcBroadcastSocketInit(BROADCAST_PORT, BROADCAST_ADDRESS))
    {
        ERR_vPrintf(T_TRUE,"IotcBroadcastSocketInit failed\n");  
        return E_BROAD_ERROR;    
    }
        
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if(0 != pthread_create(&sIotcBroadcast.pthIotcBroad, &attr, tfIotcBroadcastThread, NULL))
    {
        ERR_vPrintf(T_TRUE,"pthread_create failed, %s\n", strerror(errno));  
        return E_BROAD_ERROR_PTHREAD_CREATE;
    }
    
    return E_BROAD_OK;
}

teBroadStatus IotcBroadcastFinished()
{
    DBG_vPrintf(DBG_BROAD, "IotcMulticastFinished\n");

    sIotcBroadcast.eThreadState = E_THREAD_STOPPED;

    return E_BROAD_OK;
}

/****************************************************************************/
/***        Locate   Functions                                            ***/
/****************************************************************************/
static teBroadStatus IotcBroadcastSocketInit(int iPort, char *paNetAddress)
{
    DBG_vPrintf(DBG_BROAD, "IotcBroadcastSocketInit\n");
    if(NULL == paNetAddress)
    {
        ERR_vPrintf(T_TRUE, "The Param is Error\n");
        return E_BROAD_ERROR_PARAM;
    }
    
    if(-1 == (sIotcBroadcast.iSocketFd = socket(AF_INET, SOCK_DGRAM, 0)))
    {
        ERR_vPrintf(T_TRUE, "socket create error %s\n", strerror(errno));
        return E_BROAD_ERROR_CREATESOCK;
    }
    
    int on = 1;  /*SO_REUSEADDR port can used twice by program */
    if((setsockopt(sIotcBroadcast.iSocketFd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)))<0) 
    {  
        ERR_vPrintf(T_TRUE,"setsockopt failed, %s\n", strerror(errno));  
        close(sIotcBroadcast.iSocketFd);
        return E_BROAD_ERROR_SETSOCK;
    }  

    bzero(&sIotcBroadcast.server_addr,sizeof(sIotcBroadcast.server_addr));  
    sIotcBroadcast.server_addr.sin_family=AF_INET;  
    sIotcBroadcast.server_addr.sin_addr.s_addr=htonl(INADDR_ANY);  
    sIotcBroadcast.server_addr.sin_port=htons(iPort);  
    
    if(-1 == bind(sIotcBroadcast.iSocketFd, 
                    (struct sockaddr*)&sIotcBroadcast.server_addr, sizeof(sIotcBroadcast.server_addr)))
    {
        ERR_vPrintf(T_TRUE,"bind socket failed, %s\n", strerror(errno));  
        close(sIotcBroadcast.iSocketFd);
        return E_BROAD_ERROR_BIND;
    }

    BLUE_vPrintf(DBG_BROAD, "Create Socket Fd %d\n", sIotcBroadcast.iSocketFd);
    return E_BROAD_OK;
}

static void *tfIotcBroadcastThread(void *arg)
{
    BLUE_vPrintf(DBG_BROAD, "tfIotcMulticastThread\n");
    sIotcBroadcast.eThreadState = E_THREAD_RUNNING;

    int  iRecvLen = 0;
    char paRecvBuffer[MDBF] = {0};
    int  iAddrLen = sizeof(sIotcBroadcast.server_addr);
    while(sIotcBroadcast.eThreadState)
    {
        sched_yield();
        if((iRecvLen = recvfrom(sIotcBroadcast.iSocketFd, paRecvBuffer, MDBF, 0, 
                    (struct sockaddr*)&sIotcBroadcast.server_addr,(socklen_t*)&iAddrLen)) < 0)
        {
            ERR_vPrintf(T_TRUE, "Recvfrom Data Error!%s\n", strerror(errno));
            usleep(5);
            continue;
        }
        BLUE_vPrintf(DBG_BROAD, "Recv Data[%d]: %s\n", iRecvLen, paRecvBuffer);
        const char *paResponse = "This is Server";
        if(sendto(sIotcBroadcast.iSocketFd, paResponse, strlen(paResponse), 0, 
                    (struct sockaddr*)&sIotcBroadcast.server_addr, sizeof(sIotcBroadcast.server_addr)) < 0)
        {
            ERR_vPrintf(T_TRUE, "Send Data Error!\n");
        }
        
        sleep(0);
    }

    DBG_vPrintf(DBG_BROAD, "Exit tfIotcMulticastThread\n");
    pthread_exit("exit");
}


/****************************************************************************/
/***        End           file                                            ***/
/****************************************************************************/



