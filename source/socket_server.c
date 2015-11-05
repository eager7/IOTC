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
static void SocketCallBackListenerClear();
static void SocketClientListFree();
static void *SocketClientDataHandleThread(void *arg);

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
static tsSocketServer sSocketServer;
static tsSocketClient sSocketClientHead;


/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
teSocketStatus SocketServerInit(int iPort, char *psNetAddress)
{
    BLUE_vPrintf(DBG_SOCK, "SocketServerInit\n");

    signal(SIGPIPE, SIG_IGN);//ingnore signal interference
    memset(&sSocketServer, 0, sizeof(sSocketServer));
    pthread_mutex_init(&sSocketServer.sSocketCallbacks.mutex, NULL);
    dl_list_init(&sSocketServer.sSocketCallbacks.sCallListHead.list);

    memset(&sSocketClientHead, 0, sizeof(sSocketClientHead));
    dl_list_init(&sSocketClientHead.list);

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
    SocketCallBackListenerClear();
    pthread_mutex_destroy(&sSocketServer.sSocketCallbacks.mutex);

    SocketClientListFree();
    BLUE_vPrintf(DBG_SOCK, " SocketServerFinished %s\n", (char*)psThread_Result);

    return E_SOCK_OK;
}


teSocketStatus SocektClientSendMessage(tsSocketClient *psSocketCliet, char *psMessage, int iMessageLen)
{
    DBG_vPrintf(DBG_SOCK, "SocektClientSendMessage\n");
    if ((NULL == psSocketCliet) || (NULL == psMessage))
    {
        ERR_vPrintf(T_TRUE, "The paramter is error\n");
        return E_SOCK_INVALID_PARAMETER;
    }

    if(-1 == send(psSocketCliet->iSocketFd, psMessage, iMessageLen, 0))
    {
        ERR_vPrintf(T_TRUE,"send failed, %s\n", strerror(errno));  
        return E_SOCK_ERROR;
    }   

    return E_SOCK_OK;
}

teSocketStatus SocektClientWaitMessage(tsSocketClient *psSocketCliet, char *psMessage, uint32 u32WaitTimeoutms)
{
    DBG_vPrintf(DBG_SOCK, "SocektClientWaitMessage\n");
    if ((NULL == psSocketCliet) || (NULL == psMessage))
    {
        ERR_vPrintf(T_TRUE, "The paramter is error\n");
        return E_SOCK_INVALID_PARAMETER;
    }

    teSocketStatus eSocketStatus;
    pthread_mutex_lock(&psSocketCliet->mutex_cond);
    struct timeval sNow;
    struct timespec sTimeout;
    memset(&sNow, 0, sizeof(struct timeval));
    gettimeofday(&sNow, NULL);
    sTimeout.tv_sec = sNow.tv_sec + (u32WaitTimeoutms/1000);
    sTimeout.tv_nsec = (sNow.tv_usec + ((u32WaitTimeoutms % 1000) * 1000)) * 1000;//ns
    if (sTimeout.tv_nsec > 1000000000)
    {
        sTimeout.tv_sec++;
        sTimeout.tv_nsec -= 1000000000;
    }
    switch (pthread_cond_timedwait(&psSocketCliet->cond_message_receive, &psSocketCliet->mutex_cond, &sTimeout))
    {
        case (E_WAIT_OK):
            DBG_vPrintf(DBG_SOCK, "Got message type\n");
            pthread_mutex_lock(&psSocketCliet->mutex);
            //Copy Data
            memcpy(psMessage, psSocketCliet->csClientData, psSocketCliet->iSocketDataLen);
            pthread_mutex_unlock(&psSocketCliet->mutex);
            eSocketStatus = E_SOCK_OK;
            break;
        case (E_WAIT_TIMEOUT):
            ERR_vPrintf(T_TRUE, "Timed out for wait message \n");
            eSocketStatus = E_SOCK_NO_MESSAGE;
            break;
        
        default:
            DBG_vPrintf(DBG_SOCK, "Reset queue for next user\n");
            eSocketStatus = E_SOCK_ERROR;
            break;
    }    
    pthread_mutex_unlock(&psSocketCliet->mutex_cond);
    return E_SOCK_OK;
}

teSocketStatus SocketCallBackListenerAdd(uint16 u16MessageType, tprSocketMessageCallback prSocketMessageCallback)
{
    DBG_vPrintf(DBG_SOCK, "SocketCallBackListerAdd\n");

    pthread_mutex_lock(&sSocketServer.sSocketCallbacks.mutex);
    //TODO:Add Listenser
    tsSocketCallbackEntry *psSocketCallbackEntryNew = (tsSocketCallbackEntry*)malloc(sizeof(tsSocketCallbackEntry));
    if(NULL == psSocketCallbackEntryNew)
    {
        ERR_vPrintf(T_TRUE, "Can't malloc memory %s\n", strerror(errno));
        return E_SOCK_ERROR;
    }
    psSocketCallbackEntryNew->u16Type = u16MessageType;
    psSocketCallbackEntryNew->prCallback = prSocketMessageCallback;
    dl_list_add_tail(&sSocketServer.sSocketCallbacks.sCallListHead.list, &psSocketCallbackEntryNew->list);
    pthread_mutex_unlock(&sSocketServer.sSocketCallbacks.mutex);
    
    return E_SOCK_OK;
}


/****************************************************************************/
/***        Local    Functions                                            ***/
/****************************************************************************/
static teSocketStatus SocketInitSocket(int iPort, char *psNetAddress)
{
    struct sockaddr_in server_addr;  
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
        return E_SOCK_ERROR_BIND;
    }

    if(-1 == listen(sSocketServer.iSocketFd, SOCKET_LISTEN_NUM))
    {
        ERR_vPrintf(T_TRUE,"listen socket failed, %s\n", strerror(errno));  
        return E_SOCK_ERROR_LISTEN;
    }
    return E_SOCK_OK;
}

static void SocketCallBackListenerClear()
{
    DBG_vPrintf(DBG_SOCK, "SocketCallBackListerAdd\n");

    pthread_mutex_lock(&sSocketServer.sSocketCallbacks.mutex);
    //TODO:Remove Listenser
    tsSocketCallbackEntry *psSocketCallbackEntry1, *psSocketCallbackEntry2 = NULL;
    dl_list_for_each_safe(psSocketCallbackEntry1, psSocketCallbackEntry2, &sSocketClientHead.list, tsSocketCallbackEntry, list)
    {
        dl_list_del(&psSocketCallbackEntry1->list);
        free(psSocketCallbackEntry1);
        psSocketCallbackEntry1 = NULL;
    }
    pthread_mutex_unlock(&sSocketServer.sSocketCallbacks.mutex);
}

static void SocketClientListFree()
{
    tsSocketClient *psSocketClientTemp1, *psSocketClientTemp2 = NULL;
    dl_list_for_each_safe(psSocketClientTemp1, psSocketClientTemp2, &sSocketClientHead.list, tsSocketClient, list)
    {
        dl_list_del(&psSocketClientTemp1->list);
        pthread_mutex_destroy(&psSocketClientTemp1->mutex);
        pthread_mutex_destroy(&psSocketClientTemp1->mutex_cond);
        free(psSocketClientTemp1);
        psSocketClientTemp1 = NULL;
    }
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
    pthread_mutex_init(&psSocketClientNew->mutex, NULL);
    pthread_mutex_init(&psSocketClientNew->mutex_cond, NULL);
    pthread_cond_init(&psSocketClientNew->cond_message_receive, NULL);
    dl_list_add_tail(&sSocketClientHead.list, &psSocketClientNew->list);
    return psSocketClientNew;
}

static void SocketClientDestory(tsSocketClient *psSocketClient)
{
    close(psSocketClient->iSocketFd);
    dl_list_del(&psSocketClient->list);
    pthread_mutex_destroy(&psSocketClient->mutex);
    pthread_mutex_destroy(&psSocketClient->mutex_cond);
    pthread_cond_destroy(&psSocketClient->cond_message_receive);
    free(psSocketClient);
}

static teSocketStatus SocketServerHandleRecvMessage(int iSocketFd, tsSocketClient *psSocketClient)
{
    BLUE_vPrintf(DBG_SOCK, "SocketServerHandleRecvMessage\n");
    
    uint16 u16MessageType = 0;
    json_object *psJsonRecvMessage = NULL, *psJsonTemp = NULL;
    if (NULL != (psJsonRecvMessage = json_tokener_parse(psSocketClient->csClientData)))
    {
        if (NULL != (psJsonTemp = json_object_object_get(psJsonRecvMessage, paSequenceNo)))
        {
            if (NULL != (psJsonTemp = json_object_object_get(psJsonRecvMessage, paMessageType)))
            {
                u16MessageType = json_object_get_int(psJsonTemp);
            }
            else
            {
                ERR_vPrintf(T_TRUE, "Json Format Error\n");
                json_object_put(psJsonRecvMessage);
                return E_SOCK_ERROR_FORMAT;
            }
        }
        else
        {
            ERR_vPrintf(T_TRUE, "Json Format Error\n");
            json_object_put(psJsonRecvMessage);
            return E_SOCK_ERROR_FORMAT;
        }
    }
    else
    {
        ERR_vPrintf(T_TRUE, "Json Format Error\n");
        return E_SOCK_ERROR_FORMAT;
    }

    uint8 u8Handle = 0;
    tsSocketCallbackEntry *psSocketCallbackEntryTemp = NULL;
    dl_list_for_each(psSocketCallbackEntryTemp, &sSocketServer.sSocketCallbacks.sCallListHead.list, tsSocketCallbackEntry, list)
    {
        if (psSocketCallbackEntryTemp->u16Type == u16MessageType)
        {
            if (NULL != psSocketCallbackEntryTemp->prCallback)
            {                                           
                psSocketCallbackEntryTemp->prCallback(&iSocketFd, psJsonRecvMessage, psSocketClient->iSocketDataLen); 
                u8Handle = 1;
            }
        }
    }
    if (0 == u8Handle)
    {
        pthread_cond_broadcast(&psSocketClient->cond_message_receive); 
    }
    json_object_put(psJsonRecvMessage);
    
    return E_SOCK_OK;
}

static void *SocketClientDataHandleThread(void *arg)
{
    BLUE_vPrintf(DBG_SOCK, "SocketClientDataHandleThread\n");

    tsSocketClient *psSocketClientThread = (tsSocketClient*)arg;
    
    SocketServerHandleRecvMessage(psSocketClientThread->iSocketFd, psSocketClientThread);
    free(psSocketClientThread);
    
    DBG_vPrintf(DBG_SOCK, "Exit SocketClientDataHandleThread\n");
    pthread_exit("exit");
}

static void ThreadSignalHandler(int sig)
{
    BLUE_vPrintf(DBG_SOCK, "ThreadSignalHandler Used To Interrupt System Call\n");
}

static void *SocketServerHandleThread(void *arg)
{
    BLUE_vPrintf(DBG_SOCK, "SocketServerHandleThread\n");
    sSocketServer.eThreadState = E_THREAD_RUNNING;
    signal(THREAD_SIGNAL, ThreadSignalHandler);

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
                                                                psSocketClientNew->iSocketFd, dl_list_len(&sSocketClientHead.list));
                            
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
                        if(0 == dl_list_len(&sSocketClientHead.list))
                        {
                            ERR_vPrintf(T_TRUE, "There is no Client in List\n");
                            continue;
                        }
                        
                        tsSocketClient *psSocketClientTemp1, *psSocketClientTemp2 = NULL;
                        dl_list_for_each_safe(psSocketClientTemp1, psSocketClientTemp2, &sSocketClientHead.list, tsSocketClient, list)
                        {
                            if(EpollEventList[n].data.fd == psSocketClientTemp1->iSocketFd)
                            {
                                /***********----------------RecvMessage-----------------************/                                
                                BLUE_vPrintf(DBG_SOCK, "Socket Client[%d] Begin Recv Data...\n", psSocketClientTemp1->iSocketFd);
                                pthread_mutex_lock(&psSocketClientTemp1->mutex);
                                psSocketClientTemp1->iSocketDataLen = recv(psSocketClientTemp1->iSocketFd, 
                                    psSocketClientTemp1->csClientData, sizeof(psSocketClientTemp1->csClientData), 0);
                                pthread_mutex_unlock(&psSocketClientTemp1->mutex);
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

                                    //SocketServerHandleRecvMessage(psSocketClientTemp1);
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
    tsSocketClient *psSocketClientTemp1, *psSocketClientTemp2 = NULL;
    dl_list_for_each_safe(psSocketClientTemp1, psSocketClientTemp2, &sSocketClientHead.list, tsSocketClient, list)
    {
        dl_list_del(&psSocketClientTemp1->list);        
        free(psSocketClientTemp1);
        psSocketClientTemp1 = NULL;
    }
    
    DBG_vPrintf(DBG_SOCK, "Exit SocketServerHandleThread\n");
    pthread_exit("Get Killed Signal");
}



