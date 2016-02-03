/****************************************************************************
 *
 * MODULE:             msocket.c
 *
 * COMPONENT:          socket interface
 *
 * REVISION:           $Revision:  0$
 *
 * DATED:              $Date: 2016-01-04 17:04:13 $
 *
 * AUTHOR:             PCT
 *
 ****************************************************************************
 *
 * Copyright panchangtao@gmail.com 2016. All rights reserved
 *
 ***************************************************************************/

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include "msocket.h"
/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#define DBG_SOCK 1
#define SOCKET_LISTEN_NUM 5
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
temSocketStatus mSocketInit(tsmSocket *psmSocket, int iPort, char *paNetAddress, bool_t isServer)
{
    DBG_vPrintf(DBG_SOCK, "mSocketInit\n");

    CheckNull(psmSocket, E_SOCKET_NULL);
    CheckNull(paNetAddress, E_SOCKET_NULL);
    signal(SIGPIPE, SIG_IGN);//ingnore signal interference
    
    psmSocket->sAddr.sAddr_Ipv4.sin_family = AF_INET;
    psmSocket->sAddr.sAddr_Ipv4.sin_port = htons(iPort);
    if(isServer){
        CheckError(inet_pton(psmSocket->iDomain, INADDR_ANY, &psmSocket->sAddr.sAddr_Ipv4.sin_addr), 1, E_SOCKET_INIT);
    } else {
        CheckError(inet_pton(psmSocket->iDomain, paNetAddress, &psmSocket->sAddr.sAddr_Ipv4.sin_addr), 1, E_SOCKET_INIT);
    }
    
    if(-1 == (psmSocket->iSocketFd = socket(AF_INET, SOCK_STREAM, 0)))
    {
        ERR_vPrintf(T_TRUE, "socket create error %s\n", strerror(errno));
        return E_SOCKET_INIT;
    }
    int on = 1;  /*SO_REUSEADDR port can used twice by program */
    CheckError(setsockopt(psmSocket->iSocketFd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)), 0, E_SOCKET_INIT);

    if(isServer){
        CheckError(bind(psmSocket->iSocketFd, (struct sockaddr*)&psmSocket->sAddr.sAddr_Ipv4, sizeof(struct sockaddr_in)), 0, E_SOCKET_INIT);
        CheckError(listen(psmSocket->iSocketFd, SOCKET_LISTEN_NUM), 0, E_SOCKET_INIT);
    }
    return E_SOCKET_OK;
}

temSocketStatus mSocketFinished(tsmSocket *psmSocket)
{
    DBG_vPrintf(DBG_SOCK, "mSocketFinished\n");
    CheckNull(psmSocket, E_SOCKET_CLOSE);
    
    CheckError(close(psmSocket->iSocketFd), 0, E_SOCKET_CLOSE);
    return E_SOCKET_OK;
}

temSocketStatus mSocketSend(tsmSocket *psmSocket, char *paSendMsg, uint16 u16Length)
{
    DBG_vPrintf(DBG_SOCK, "mSocketSend\n");
    CheckNull(psmSocket, E_SOCKET_SEND);
    CheckNull(paSendMsg, E_SOCKET_SEND);
    
    if( -1 == send(psmSocket->iSocketFd, paSendMsg, u16Length, 0)){
        ERR_vPrintf(T_TRUE, "socket send error %s\n", strerror(errno));
        return E_SOCKET_SEND;
    }
    return E_SOCKET_OK;
}

temSocketStatus mSocketRecv(tsmSocket *psmSocket, char *paRecvMsg, uint16 u16Length)
{
    DBG_vPrintf(DBG_SOCK, "mSocketSend\n");
    CheckNull(psmSocket, E_SOCKET_RECV);
    CheckNull(paRecvMsg, E_SOCKET_RECV);
    int ret = recv(psmSocket->iSocketFd, paRecvMsg, u16Length, 0);
    if( -1 == ret){
        ERR_vPrintf(T_TRUE, "socket send error %s\n", strerror(errno));
        return E_SOCKET_RECV;
    } else if(0 == ret){
        ERR_vPrintf(T_TRUE, "socket is disconnected..\n");
        return E_SOCKET_DISCONNECT;
    }
    return E_SOCKET_OK;
}

