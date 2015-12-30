/****************************************************************************
 *
 * MODULE:             iotc_sock_server.c
 *
 * COMPONENT:          connect with cc interface
 *
 * REVISION:           $Revision:  0$
 *
 * DATED:              $Date: 2015-12-12 15:13:17 +0100 (Thu, 21 Oct 2015 $
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <unistd.h> //usleep
#include <sys/un.h>
#include "iotc_sock_server.h"
/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#define NUM_LISTEN 5
#define checkSocketError(s,r) do{if(0!=r){ERR_vPrintf(1,"[%d]err:%s\n", __LINE__, strerror(errno));close(s);return -1;}}while(0)
/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/
extern uint8 verbosity;
/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
int IOTCSockServerInit(char *sockAddr, int port, uint8 isnet)
{
    BLUE_vPrintf(verbosity, "IOTCSockServerInit\n");

    int iSocketFd = -1;
    if (isnet){
        checkError((iSocketFd = socket(AF_INET,SOCK_STREAM,0)));

        struct sockaddr_in addr_server;
        addr_server.sin_port = port;
        addr_server.sin_family = AF_INET;
        if(NULL == sockAddr){
            addr_server.sin_addr.s_addr = htons(INADDR_ANY);
        } else {
            addr_server.sin_addr.s_addr = inet_addr(sockAddr);
        }
        checkSocketError(iSocketFd, bind(iSocketFd, (struct sockaddr*)&addr_server, sizeof(addr_server)));
        checkSocketError(iSocketFd, listen(iSocketFd, NUM_LISTEN));
    } else {
        checkNull(sockAddr);
        checkError((iSocketFd = socket(AF_UNIX,SOCK_STREAM,0)));

        struct sockaddr_un addr_server;
        addr_server.sun_family = AF_UNIX;
        strncpy(addr_server.sun_path, sockAddr, sizeof(addr_server.sun_path) - 1);
        unlink(sockAddr);
        checkSocketError(iSocketFd, bind(iSocketFd, (struct sockaddr*)&addr_server, sizeof(addr_server)));
        checkSocketError(iSocketFd, listen(iSocketFd, NUM_LISTEN));
    }
    return iSocketFd;
}


