/****************************************************************************
 *
 * MODULE:             iotc_application.h
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
#include "iotc_multicast.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#define DBG_MUL 1

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/
static void *tfIotcMulticastThread(void *arg);
static teMultiStatus IotcMulticastSocketInit(int iPort, char *paMulAddress);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
static tsIotcMulticast sIotcMulticast;
/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
teMultiStatus IotcMulticastInit()
{
    DBG_vPrintf(DBG_MUL, "IotcApplicationkInit\n");

    memset(&sIotcMulticast, 0, sizeof(tsIotcMulticast));
    BLUE_vPrintf(DBG_MUL, "pthread_create\n");

    IotcMulticastSocketInit(MULTICAST_PORT, MULTICAST_ADDRESS);
        
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if(0 != pthread_create(&sIotcMulticast.pthIotcMulti, &attr, tfIotcMulticastThread, NULL))
    {
        ERR_vPrintf(T_TRUE,"pthread_create failed, %s\n", strerror(errno));  
        return E_MULTI_ERROR_PTHREAD_CREATE;
    }
    
    return E_MULTI_OK;
}

teMultiStatus IotcMulticastFinished()
{
    DBG_vPrintf(DBG_MUL, "IotcMulticastFinished\n");

    sIotcMulticast.eThreadState = E_THREAD_STOPPED;

    return E_MULTI_OK;
}

/****************************************************************************/
/***        Locate   Functions                                            ***/
/****************************************************************************/
static teMultiStatus IotcMulticastSocketInit(int iPort, char *paMulAddress)
{
    DBG_vPrintf(DBG_MUL, "IotcMulticastSocketInit\n");
    if(NULL == paMulAddress)
    {
        ERR_vPrintf(T_TRUE, "The Param is Error\n");
        return E_MULTI_ERROR_PARAM;
    }
    
    if(-1 == (sIotcMulticast.iSocketFd = socket(AF_INET, SOCK_DGRAM, 0)))
    {
        ERR_vPrintf(T_TRUE, "socket create error %s\n", strerror(errno));
        return E_MULTI_ERROR_CREATESOCK;
    }
    
    int on = 1;  /*SO_REUSEADDR port can used twice by program */
    if((setsockopt(sIotcMulticast.iSocketFd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)))<0) 
    {  
        ERR_vPrintf(T_TRUE,"setsockopt failed, %s\n", strerror(errno));  
        close(sIotcMulticast.iSocketFd);
        return E_MULTI_ERROR_SETSOCK;
    }  

    sIotcMulticast.server_addr.sin_family = AF_INET;  
    sIotcMulticast.server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    sIotcMulticast.server_addr.sin_port = htons(iPort);
    if(-1 == bind(sIotcMulticast.iSocketFd, 
                    (struct sockaddr*)&sIotcMulticast.server_addr, sizeof(sIotcMulticast.server_addr)))
    {
        ERR_vPrintf(T_TRUE,"bind socket failed, %s\n", strerror(errno));  
        close(sIotcMulticast.iSocketFd);
        return E_MULTI_ERROR_BIND;
    }

    sIotcMulticast.multi_addr.imr_multiaddr.s_addr = inet_addr(paMulAddress);
    sIotcMulticast.multi_addr.imr_interface.s_addr = htonl(INADDR_ANY);
    if(setsockopt(sIotcMulticast.iSocketFd, IPPROTO_IP, IP_ADD_MEMBERSHIP, 
            (char *)&sIotcMulticast.multi_addr, sizeof(sIotcMulticast.multi_addr)) < 0)
    {
        ERR_vPrintf(T_TRUE,"setsockopt failed, %s\n", strerror(errno));  
        close(sIotcMulticast.iSocketFd);
        return E_MULTI_ERROR_SETSOCK;
    }


    return E_MULTI_OK;
}

static void *tfIotcMulticastThread(void *arg)
{
    BLUE_vPrintf(DBG_MUL, "tfIotcMulticastThread\n");
    sIotcMulticast.eThreadState = E_THREAD_RUNNING;

    int  iRecvLen = 0;
    char paRecvBuffer[MDBF] = {0};
    int  iAddrLen = sizeof(sIotcMulticast.server_addr);
    while(sIotcMulticast.eThreadState)
    {
        sched_yield();
        if((iRecvLen = recvfrom(sIotcMulticast.iSocketFd, paRecvBuffer, MDBF, 0, 
                    (struct sockaddr*)&sIotcMulticast.server_addr,(socklen_t*)&iAddrLen)) < 0)
        {
            ERR_vPrintf(T_TRUE, "Recvfrom Data Error!\n");
        }
        BLUE_vPrintf(DBG_MUL, "Recv Data: %s\n", paRecvBuffer);
        const char *paResponse = "This is Server";
        if(sendto(sIotcMulticast.iSocketFd, paResponse, strlen(paResponse), 0, 
                    (struct sockaddr*)&sIotcMulticast.server_addr, sizeof(sIotcMulticast.server_addr)) < 0)
        {
            ERR_vPrintf(T_TRUE, "Send Data Error!\n");
        }
        
        sleep(0);
    }

    DBG_vPrintf(DBG_MUL, "Exit tfIotcMulticastThread\n");
    pthread_exit("exit");
}


/****************************************************************************/
/***        End           file                                            ***/
/****************************************************************************/


