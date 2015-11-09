/****************************************************************************
 *
 * MODULE:             IOTC - Internet of things control daemon
 *
 * COMPONENT:          multicast client
 *
 * REVISION:           $Revision: 01 $
 *
 * DATED:              $Date: 2015-11-09 11:13:17 +0100 (Fri, 30 Oct 2015) $
 *
 * AUTHOR:             PCT
 *
 ****************************************************************************
 *
 * Copyright IOTC B.V. 2015. All rights reserved
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

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#define DBG_MAIN 1

#define MULTICAST_ADDRESS "227.227.227.227"
#define MULTICAST_PORT  7789
/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/
volatile sig_atomic_t bRunning = 1;/** Main loop running flag */


/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

/****************************************************************************/
/***        Locate   Functions                                            ***/
/****************************************************************************/
int main(int argc, char *argv[])
{
    printf("Start IOTC Multicast Client Program...\n");

    signal(SIGPIPE, SIG_IGN);//ingnore signal interference

    int iSocketFd = 0;
    if(-1 == (iSocketFd = socket(AF_INET, SOCK_DGRAM, 0)))
    {
        ERR_vPrintf(T_TRUE, "socket create error %s\n", strerror(errno));
        return -1;
    }
    
    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family          = AF_INET;  
    client_addr.sin_addr.s_addr     = inet_addr(MULTICAST_ADDRESS);  
    client_addr.sin_port            = htons(MULTICAST_PORT);

    while(bRunning)
    {
        PURPLE_vPrintf(DBG_MAIN, "Main Loop...\n");

        int iRet = 0;
        const char *paIpRequest = "This is a request";
        char paRecvBuffer[MDBF] = {0};
        DBG_vPrintf(DBG_MAIN, "Send Data : %s\n", paIpRequest);
        if((iRet = sendto(iSocketFd, paIpRequest, strlen(paIpRequest), 0, 
                    (struct sockaddr*)&client_addr, sizeof(client_addr))) < 0)
        {
            ERR_vPrintf(T_TRUE, "Send Data Error!\n");
        }

        int iAddrLen = sizeof(client_addr);
        if((iRet = recvfrom(iSocketFd, paRecvBuffer, MDBF, 0, 
                    (struct sockaddr*)&client_addr,(socklen_t*)&iAddrLen)) < 0)
        {
            ERR_vPrintf(T_TRUE, "Recvfrom Data Error!\n");
        }
        DBG_vPrintf(DBG_MAIN, "Recv Data : %s\n", paRecvBuffer);
        
        sleep(3);//dispatch thread
    }

    return 0;
}

int MulticastClientInit()
{
    return 0;
}
