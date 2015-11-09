/****************************************************************************
 *
 * MODULE:             socket_server.c
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

#include <json/json.h>

#include "socket_server.h"
/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#define DBG_SOCK 1 
/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/
static void *SocketServerHandleThread(void *arg);
static teSocketStatus SocketInitSocket(int iPort, char *psNetAddress);
static void SocketClientListFree();
static void *SocketClientDataHandleThread(void *arg);

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
static const char *paSocketDisconnect = "{\"sequence_no\": 1,\"message_type\": 4,\"event_type\":1,\"description\": \"\"}";
static tsSocketServer sSocketServer;
static tsSocketClientHead sSocketClientHead;

tsSocketEventQuene sSocketEventQuene;

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
teSocketStatus SocketServerInit(int iPort, char *psNetAddress)
{
    BLUE_vPrintf(DBG_SOCK, "SocketServerInit\n");
    pthread_mutexattr_t     attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);

    signal(SIGPIPE, SIG_IGN);//ingnore signal interference
    memset(&sSocketServer, 0, sizeof(tsSocketServer));

    memset(&sSocketEventQuene, 0, sizeof(tsSocketEventQuene));
    sSocketEventQuene.flag_data  =  T_FALSE;
    sSocketEventQuene.flag_space =  T_TRUE;
    pthread_mutex_init(&sSocketEventQuene.mutex, NULL);
    pthread_cond_init(&sSocketEventQuene.cond_data_available, NULL);
    pthread_cond_init(&sSocketEventQuene.cond_space_available, NULL);
    
    memset(&sSocketClientHead, 0, sizeof(tsSocketClientHead));
    pthread_mutex_init(&sSocketClientHead.mutex, &attr);
    dl_list_init(&sSocketClientHead.sSocketClient.list);

    teSocketStatus eSocketStatus;
    if(E_SOCK_OK != (eSocketStatus = SocketInitSocket(iPort, psNetAddress)))
    {
        return eSocketStatus;
    }
    
    //start accept thread
    BLUE_vPrintf(DBG_SOCK, "pthread_create\n");
    if(0 != pthread_create(&sSocketServer.pthSocketServer, NULL, SocketServerHandleThread, NULL))
    {
        ERR_vPrintf(T_TRUE,"pthread_create failed, %s\n", strerror(errno));  
        return E_SOCK_ERROR_PTHREAD_CREATE;
    }

    return E_SOCK_OK;
}

teSocketStatus SocketServerFinished()
{
    BLUE_vPrintf(DBG_SOCK, "Waiting SocketServerFinished...\n");
    
    sSocketServer.eThreadState = E_THREAD_STOPPED;
    pthread_kill(sSocketServer.pthSocketServer, THREAD_SIGNAL);
    void *psThread_Result = NULL;
    if(0 != pthread_join(sSocketServer.pthSocketServer, &psThread_Result))
    {
        ERR_vPrintf(T_TRUE,"phread_join socket failed, %s\n", strerror(errno));  
        return E_SOCK_ERROR_JOIN;
    }
    BLUE_vPrintf(DBG_SOCK, " SocketServerFinished %s\n", (char*)psThread_Result);
    
    SocketClientListFree();
    
    return E_SOCK_OK;
}


teSocketStatus SocketClientSendMessage(int iSocketFd, char *psMessage, int iMessageLen)
{
    DBG_vPrintf(DBG_SOCK, "SocektClientSendMessage\n");
    if (NULL == psMessage)
    {
        ERR_vPrintf(T_TRUE, "The paramter is error\n");
        return E_SOCK_INVALID_PARAMETER;
    }

    if(-1 == send(iSocketFd, psMessage, iMessageLen, 0))
    {
        ERR_vPrintf(T_TRUE,"send failed, %s\n", strerror(errno));  
        return E_SOCK_ERROR;
    }   

    return E_SOCK_OK;
}

/****************************************************************************/
/***        Local    Functions                                            ***/
/****************************************************************************/
static teSocketStatus SocketInitSocket(int iPort, char *psNetAddress)
{
    struct sockaddr_in server_addr;  
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;  
    if(NULL != psNetAddress)
    {
        server_addr.sin_addr.s_addr = inet_addr(psNetAddress);  /*just receive one address*/
    }
    else
    {
        server_addr.sin_addr.s_addr = htonl(INADDR_ANY);        /*receive any address*/
    }
    server_addr.sin_port = htons(iPort);

    if(-1 == (sSocketServer.iSocketFd = socket(AF_INET, SOCK_STREAM, 0)))
    {
        ERR_vPrintf(T_TRUE, "socket create error %s\n", strerror(errno));
        return E_SOCK_ERROR_CREATESOCK;
    }

    int on = 1;  /*SO_REUSEADDR port can used twice by program */
    if((setsockopt(sSocketServer.iSocketFd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)))<0) 
    {  
        ERR_vPrintf(T_TRUE,"setsockopt failed, %s\n", strerror(errno));  
        return E_SOCK_ERROR_SETSOCK;
    }  

    if(-1 == bind(sSocketServer.iSocketFd, (struct sockaddr*)&server_addr, sizeof(server_addr)))
    {
        ERR_vPrintf(T_TRUE,"bind socket failed, %s\n", strerror(errno));  
        close(sSocketServer.iSocketFd);
        return E_SOCK_ERROR_BIND;
    }

    if(-1 == listen(sSocketServer.iSocketFd, SOCKET_LISTEN_NUM))
    {
        ERR_vPrintf(T_TRUE,"listen socket failed, %s\n", strerror(errno));  
        close(sSocketServer.iSocketFd);
        return E_SOCK_ERROR_LISTEN;
    }
    return E_SOCK_OK;
}

static void SocketClientListFree()
{
    tsSocketClient *psSocketClientTemp1, *psSocketClientTemp2 = NULL;
    pthread_mutex_lock(&sSocketClientHead.mutex);
    dl_list_for_each_safe(psSocketClientTemp1, psSocketClientTemp2, &sSocketClientHead.sSocketClient.list, tsSocketClient, list)
    {
        dl_list_del(&psSocketClientTemp1->list);
        free(psSocketClientTemp1);
        psSocketClientTemp1 = NULL;
    }
    pthread_mutex_unlock(&sSocketClientHead.mutex);
}

tsSocketClient* SocketClientNew()
{
    tsSocketClient *psSocketClientNew = (tsSocketClient *)malloc(sizeof(tsSocketClient));
    if(NULL == psSocketClientNew)
    {
        ERR_vPrintf(T_TRUE,"malloc failed, %s\n", strerror(errno));                                         
        exit(1);
    }
    memset(psSocketClientNew, 0, sizeof(tsSocketClient));

    pthread_mutex_lock(&sSocketClientHead.mutex);
    dl_list_add_tail(&sSocketClientHead.sSocketClient.list, &psSocketClientNew->list);
    pthread_mutex_unlock(&sSocketClientHead.mutex);
    return psSocketClientNew;
}

static void SocketClientDestory(tsSocketClient *psSocketClient)
{
    pthread_mutex_lock(&sSocketClientHead.mutex);
    dl_list_del(&psSocketClient->list);
    pthread_mutex_unlock(&sSocketClientHead.mutex);    

    if(psSocketClient->iSocketFd > 0)
    {
        close(psSocketClient->iSocketFd);
        //Remove The Devices detch This Socket, brocast the envet to device thread
        //TODO:
        pthread_mutex_lock(&sSocketEventQuene.mutex);
        GREEN_vPrintf(DBG_SOCK, "pthread_cond_waiting ...\n");
        while(!sSocketEventQuene.flag_space)
        {
            pthread_cond_wait(&sSocketEventQuene.cond_space_available, &sSocketEventQuene.mutex);
        }
        memset(&sSocketEventQuene.sSocketClient, 0, sizeof(tsSocketClient));
        memcpy(&sSocketEventQuene.sSocketClient, psSocketClient, sizeof(tsSocketClient));
        sprintf(sSocketEventQuene.sSocketClient.csClientData, "%s", paSocketDisconnect);
        sSocketEventQuene.flag_data = T_TRUE;
        pthread_mutex_unlock(&sSocketEventQuene.mutex);
        BLUE_vPrintf(DBG_SOCK, "pthread_cond_broadcast Message\n");
        pthread_cond_broadcast(&sSocketEventQuene.cond_data_available); 





        free(psSocketClient);
    }
}

static teSocketStatus SocketServerHandleRecvMessage(tsSocketClient *psSocketClient)
{
    BLUE_vPrintf(DBG_SOCK, "SocketServerHandleRecvMessage\n");

    //Brodcast This Data to Other Thread
    pthread_mutex_lock(&sSocketEventQuene.mutex);
    GREEN_vPrintf(DBG_SOCK, "pthread_cond_waiting ...\n");
    while(!sSocketEventQuene.flag_space)
    {
        pthread_cond_wait(&sSocketEventQuene.cond_space_available, &sSocketEventQuene.mutex);
    }
    memset(&sSocketEventQuene.sSocketClient, 0, sizeof(tsSocketClient));
    memcpy(&sSocketEventQuene.sSocketClient, psSocketClient, sizeof(tsSocketClient));
    sSocketEventQuene.flag_data = T_TRUE;
    pthread_mutex_unlock(&sSocketEventQuene.mutex);
    BLUE_vPrintf(DBG_SOCK, "pthread_cond_broadcast Message\n");
    pthread_cond_broadcast(&sSocketEventQuene.cond_data_available); 

    return E_SOCK_OK;
}

static void *SocketClientDataHandleThread(void *arg)
{
    BLUE_vPrintf(DBG_SOCK, "SocketClientDataHandleThread\n");

    tsSocketClient *psSocketClientThread = (tsSocketClient*)arg;
    
    SocketServerHandleRecvMessage(psSocketClientThread);
    free(psSocketClientThread);
    
    DBG_vPrintf(DBG_SOCK, "Exit SocketClientDataHandleThread\n");
    pthread_exit("exit");
}

static void *SocketServerHandleThread(void *arg)
{
    BLUE_vPrintf(DBG_SOCK, "SocketServerHandleThread\n");
    sSocketServer.eThreadState = E_THREAD_RUNNING;
    //signal(THREAD_SIGNAL, thread_signal_handler);

    int iEpollFd = epoll_create(65535);
    if(-1 == iEpollFd)
    {
        ERR_vPrintf(T_TRUE,"epoll_create failed, %s\n", strerror(errno));  
        pthread_exit("epoll_create failed");
    }
    struct epoll_event EpollEvevt, EpollEventList[EPOLL_EVENT_NUM];
    EpollEvevt.data.fd = sSocketServer.iSocketFd;
    EpollEvevt.events = EPOLLIN;  /*read*/
    if(-1 == epoll_ctl (iEpollFd, EPOLL_CTL_ADD, sSocketServer.iSocketFd, &EpollEvevt))
    {
        ERR_vPrintf(T_TRUE,"epoll_create failed, %s\n", strerror(errno));  
        goto done;
    }
        
    while(sSocketServer.eThreadState)
    {
        DBG_vPrintf(DBG_SOCK, "\n++++++++++++++Waiting for iEpollFd Changed\n");
        int iEpollResult = epoll_wait(iEpollFd,EpollEventList,EPOLL_EVENT_NUM,-1);

        switch (iEpollResult)
        {
            case (E_EPOLL_ERROR):
            {
                ERR_vPrintf(T_TRUE,"epoll_wait failed, %s\n", strerror(errno));  
                goto done;
            }
            break;
            case (E_EPOLL_TIMEOUT):
                ERR_vPrintf(T_TRUE,"epoll_wait E_EPOLL_TIMEOUT\n");  
            break;
            default:
            {
                DBG_vPrintf(DBG_SOCK, "Epoll_wait Find %d Changed\n", iEpollResult);
                int n = 0;
                for(n = 0; n < iEpollResult; n++)
                {
                    if((EpollEventList[n].events & EPOLLERR) || (EpollEventList[n].events & EPOLLHUP))
                    {
                        ERR_vPrintf(T_TRUE,"The Fd Occured an Error, %s\n", strerror(errno));  
                        continue;
                    }
                    else if(EpollEventList[n].data.fd == sSocketServer.iSocketFd)    /*Server accept event*/
                    {
                        DBG_vPrintf(DBG_SOCK, "sSocketServer.iSocketFd Changed\n");
                        
                        tsSocketClient *psSocketClientNew = SocketClientNew();
                        int Len = sizeof(psSocketClientNew->addrclient);
                        psSocketClientNew->iSocketFd = accept(sSocketServer.iSocketFd,
                                (struct sockaddr*)&psSocketClientNew->addrclient, (socklen_t *)&Len);
                        if(-1 == psSocketClientNew->iSocketFd)
                        {
                            ERR_vPrintf(T_TRUE, "socket accept error %s\n", strerror(errno));
                            SocketClientDestory(psSocketClientNew);
                        }
                        else
                        {
                            YELLOW_vPrintf(DBG_SOCK, "A client[%d] Already Connected, The Number of Client is [%d]\n", 
                                                                psSocketClientNew->iSocketFd, dl_list_len(&sSocketClientHead.sSocketClient.list));
                            
                            sSocketServer.u8NumConnClient++;
                            EpollEvevt.data.fd = psSocketClientNew->iSocketFd;
                            EpollEvevt.events = EPOLLIN | EPOLLET;  /*read ,Ede-Triggered, close*/
                            if(-1 == epoll_ctl (iEpollFd, EPOLL_CTL_ADD, psSocketClientNew->iSocketFd, &EpollEvevt))
                            {
                                ERR_vPrintf(T_TRUE,"epoll_ctl failed, %s\n", strerror(errno));                                         
                                goto done;
                            }
                            DBG_vPrintf(DBG_SOCK, "Client Already Add Epoll_wait Fd\n");
                            break;  /*jump out for, otherwise will go accept again*/
                        }
                    }
                    else/*Client recive event or disconnect event*/
                    {
                        if(0 == dl_list_len(&sSocketClientHead.sSocketClient.list))
                        {
                            ERR_vPrintf(T_TRUE, "There is no Client in List\n");
                            continue;
                        }
                        
                        tsSocketClient *psSocketClientTemp1, *psSocketClientTemp2 = NULL;
                        dl_list_for_each_safe(psSocketClientTemp1, psSocketClientTemp2, &sSocketClientHead.sSocketClient.list, tsSocketClient, list)
                        {
                            if(EpollEventList[n].data.fd == psSocketClientTemp1->iSocketFd)
                            {
                                /***********----------------RecvMessage-----------------************/                                
                                BLUE_vPrintf(DBG_SOCK, "Socket Client[%d] Begin Recv Data...\n", psSocketClientTemp1->iSocketFd);
                                psSocketClientTemp1->iSocketDataLen = recv(psSocketClientTemp1->iSocketFd, 
                                    psSocketClientTemp1->csClientData, sizeof(psSocketClientTemp1->csClientData), 0);
                                if(-1 == psSocketClientTemp1->iSocketDataLen)
                                {
                                    ERR_vPrintf(T_TRUE, "socket recv error %s\n", strerror(errno));
                                }
                                else if(0 == psSocketClientTemp1->iSocketDataLen)   /*disconnect*/
                                {
                                    ERR_vPrintf(T_TRUE, "The Client[%d] is disconnect, Closet It\n", psSocketClientTemp1->iSocketFd);
                                    
                                    EpollEvevt.data.fd = psSocketClientTemp1->iSocketFd;
                                    EpollEvevt.events = EPOLLIN | EPOLLET | EPOLLRDHUP;  /*read ,Ede-Triggered, close*/
                                    if(-1 == epoll_ctl (iEpollFd, EPOLL_CTL_DEL, psSocketClientTemp1->iSocketFd, &EpollEvevt))
                                    {
                                        ERR_vPrintf(T_TRUE,"epoll_ctl failed, %s\n", strerror(errno));                                         
                                        goto done;
                                    }

                                    /*disconnect this socket client*/
                                    SocketClientDestory(psSocketClientTemp1);
                                    psSocketClientTemp1 = NULL;
                                    sSocketServer.u8NumConnClient --;
                                }
                                else    /*recv event*/
                                {
                                    YELLOW_vPrintf(DBG_SOCK, "Recv Data is [%d]--- %s\n", psSocketClientTemp1->iSocketFd, psSocketClientTemp1->csClientData);
                                    //SocketServerHandleRecvMessage(psSocketClientTemp1);
                                    //Start a new thread to handle data
                                    BLUE_vPrintf(DBG_SOCK, "pthread_create\n");
                                    
                                    tsSocketClient *psSocketClientThread = (tsSocketClient*)malloc(sizeof(tsSocketClient));
                                    if(NULL == psSocketClientThread)
                                    {
                                        goto done;
                                    }
                                    memcpy(psSocketClientThread, psSocketClientTemp1, sizeof(tsSocketClient));
                                    pthread_t pthreadHandle;
                                    pthread_attr_t attr;
                                    pthread_attr_init(&attr);
                                    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
                                    if(0 != pthread_create(&pthreadHandle, &attr, SocketClientDataHandleThread, psSocketClientThread))
                                    {
                                        ERR_vPrintf(T_TRUE,"pthread_create failed, %s\n", strerror(errno));  
                                        goto done;
                                    }
                                }
                                break;
                            }
                        }/*dl_list_for_each_safe*/
                    }
                }/*for*/
            }
            break;
        }
        sleep(0);
    }
    
done:    
    close(iEpollFd);
    close(sSocketServer.iSocketFd);
    SocketClientListFree();
    
    DBG_vPrintf(DBG_SOCK, "Exit SocketServerHandleThread\n");
    pthread_exit("Get Killed Signal");
}



