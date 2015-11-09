/****************************************************************************
 *
 * MODULE:             socket_client.c
 *
 * COMPONENT:          Utils of socket client
 *
 * REVISION:           $Revision:  0$
 *
 * DATED:              $Date: 2015-11-06 15:13:17 +0100 (Thu, 21 Oct 2015 $
 *
 * AUTHOR:             Abel
 *
 ****************************************************************************
 *
 * Copyright IOTC Project Group 2015. All rights reserved
 *
 ***************************************************************************/

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/epoll.h> 
#include <stdlib.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <stdio.h>

#include "utils.h"
#include "socket_client.h"
#include "pthread_utils.h"
/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

 

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/



/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
int flag = 0;
char SystemIp[IP_LEN];
char buf[BUFSIZE];

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

void signalHandler(int signal)
{
	DBG_vPrintf(T_TRUE, "Enter signalHandler\n");
	if(signal == SIGALRM){
		DBG_vPrintf(T_TRUE, "Enter signalHandler, signal = SIGALRM\n");
		flag = 1;
	}
}


eRetStatus getSystemIp(char* eth, char* pIP, int len)
{
    int fd;
    struct ifreq ifr_ip;
    char *tmp;
    
    if((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
		ERR_vPrintf(T_TRUE,"socket failed\n"); 
        return E_ERROR;
    }

    memset(&ifr_ip, 0, sizeof(ifr_ip));

    strncpy(ifr_ip.ifr_name, eth, strlen(eth));

    if(ioctl(fd, SIOCGIFADDR, &ifr_ip) < 0)
    {
    	close(fd);
        return E_ERROR;
    }

    close(fd);

    tmp = inet_ntoa(((struct sockaddr_in*)&(ifr_ip.ifr_addr))->sin_addr);

    strncpy(pIP, tmp, len);

    return E_OK;
}



eRetStatus ClientSockInit(ClientSock *sock, int type, int port, char *ifname)
{
		
	if((!sock)||(!ifname)){
		ERR_vPrintf(T_TRUE, "argument error\n"); 
        return E_ERROR;
	}

	sock->type = type;
	sock->port = port;

	sock->fd = socket(AF_INET, type, 0);//new socket
	if(sock->fd < 0){
		ERR_vPrintf(T_TRUE, "socket failed\n"); 
        return E_ERROR;
	}
	DBG_vPrintf(T_TRUE, "socket Success, fd = %d\n", sock->fd);

#if 0
	/*set sock unblock*/
	int flags;
	flags = fcntl(sock->fd, F_GETFL, 0);
	if(flags < 0) {
		ERR_vPrintf(T_TRUE, "fcntl get failed\n"); 
	    goto err;     
	}
	if (fcntl(sock->fd, F_SETFL, flags | O_NONBLOCK) < 0) {
        ERR_vPrintf(T_TRUE, "fcntl set failed\n"); 
        goto err;
    }
	DBG_vPrintf(T_TRUE, "fcntl Success\n");
#endif 

	/*get connect ip*/
	eRetStatus ret;
	memset(&(sock->addr), 0, IP_LEN);
	ret = getSystemIp(ifname, (char *)&(sock->addr), IP_LEN);
	if(ret != E_OK){
		ERR_vPrintf(T_TRUE, "get system ip failed\n"); 
		goto err;
	}
	DBG_vPrintf(T_TRUE, "get system ip Success\n");


	/*Connect*/
	struct sockaddr_in server;
	bzero(&server,sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(sock->port);
	//server.sin_addr.s_addr = inet_addr("127.0.0.1");
	server.sin_addr.s_addr = inet_addr((char *)&(sock->addr));
	ret = connect(sock->fd, (struct sockaddr *)&server, sizeof(server));
	while (ret < 0) {
		ERR_vPrintf(T_TRUE, "connect failed\n"); 
		if (errno == EINPROGRESS) {
			break;
		} 
		else {
			ERR_vPrintf(T_TRUE, "connect failed\n");
			goto err;
        }
    }
	DBG_vPrintf(T_TRUE, "Connect Server Success\n");
	
	return E_OK;

err:	close(sock->fd);
	return E_ERROR;
}



void ClientSockClose(ClientSock *sock){
	close(sock->fd);
	return;
}


eRetStatus MsgSend(int fd)
{
	int ret;
	//char *msg = "[{\"device_name\":\"light\",\"device_id\":1,\"device_index\":12345678},{\"device_name\":\"dimmerlight\",\"device_id\": 2,\"device_index\": 87654321}]";
	char *msg = "{"
    "\"sequence_no\": 1,"
    "\"message_type\": 2,"
    "\"description\": [{\"device_name\":\"light\",\"device_id\":1,"
	"\"device_index\":12345678},{\"device_name\":\"dimmerlight\","
	"\"device_id\": 2,\"device_index\": 87654321}]}";

	printf("Send Msg = %s\n", msg);
	ret = send(fd, msg, strlen(msg)+1, 0);
	if(ret < 0){
		perror("send error");
		return E_ERROR;
	}

	return E_OK;
	
}


void *MsgSendThreadProc(void *arg)
{
	DBG_vPrintf(T_TRUE, "Enter MsgSendThreadProc thread\n");
	Thread *pth = (Thread *)arg;
	int fd;

	signal(SIGALRM, signalHandler);  
	struct itimerval timer, old_timer;

	//timer.it_value.tv_sec = 5;
	//timer.it_value.tv_usec = 1000;
	//timer.it_interval.tv_sec = 5;
	//timer.it_interval.tv_usec = 0;

	//setitimer(ITIMER_REAL, &timer, &old_timer);

	fd = ((ClientSock *)(pth->arg))->fd;//get send fd
	
	DBG_vPrintf(T_TRUE, "Enter MsgSendThreadProc while, FD = %d\n", fd);
	
	while(T_TRUE == pth->flag)
	{
		DBG_vPrintf(T_TRUE, "in MsgSendThreadProc while\n");
		//if(1 == flag){
			//flag = 0;
			DBG_vPrintf(T_TRUE, "Enter MsgSendThreadProc msgsend\n");
			MsgSend(fd);
		//}

		sleep(3);
	}
	
	return NULL;
}



eRetStatus MsgSendThreadInit(Thread *pth, ClientSock *sock)
{
	eRetStatus ret;

	pth->proc = MsgSendThreadProc;
	pth->arg = (void *)sock;
	
	ret = DetachedThreadInit(pth);
	if(ret != E_OK){
		ERR_vPrintf(T_TRUE,"init detached thread failed!\n");
		return E_ERROR;
	}

	return E_OK;
}


int main(void)
{
	eRetStatus ret;
	ClientSock sock;
	Thread pth;

	ret = ClientSockInit(&sock, SOCK_STREAM, PORT, "eth0");
	if(ret != E_OK){
		ERR_vPrintf(T_TRUE,"ClientSockInit error!\n");
		return -1;
	}

	DBG_vPrintf(T_TRUE, "ClientSockInit success\n");

	ret = MsgSendThreadInit(&pth, &sock);
	if(ret != E_OK){
		ERR_vPrintf(T_TRUE,"MsgSendThreadInit error!\n");
		goto err1;
	}

	
	DBG_vPrintf(T_TRUE, "MsgSendThreadInit success\n");
	//epoll creat
	int epollfd;

	epollfd = epoll_create(10);

	struct epoll_event ev;

	ev.data.fd = sock.fd;
	ev.events = EPOLLIN|EPOLLERR|EPOLLHUP|EPOLLOUT|EPOLLET;

	ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, sock.fd, &ev);
	if(ret < 0){
		perror("epoll_ctl failed ");
		goto err;
	}


	int iEpollRet;
	DBG_vPrintf(T_TRUE, "enter main while\n");
	while(1){
		//DBG_vPrintf(T_TRUE, "in main while\n");
		iEpollRet = epoll_wait(epollfd, &ev, EPOLL_EVENT_NUM, -1);
		if(iEpollRet == 0){//epoll_wait or timeout
			ERR_vPrintf(T_TRUE,"socket failed\n"); 
			goto err;
		}
		else if(iEpollRet == -1){//error
			perror("epoll_wait failed ");
			if (errno == EINTR){
				//DBG_vPrintf(T_TRUE,"epoll_wait: interrupt\n");
            	continue;
			}else{
				goto err;
			}
		}else{
			//if((ev.events & EPOLLERR) || (ev.events & EPOLLHUP)){
			if(ev.events & EPOLLERR){
				printf("Socket Disconnected.. EPOLLERR\n");
				goto err;
			}
			else if(ev.events & EPOLLHUP){
				printf("Socket Disconnected.. EPOLLHUP\n");
				goto err;
			}
			else if(ev.events & EPOLLOUT){
				printf("Socket Disconnected.. EPOLLOUT\n");
			}
			else{
				//read event
				memset(buf, 0, BUFSIZE);
				recv(sock.fd, buf, BUFSIZE, 0);
				DBG_vPrintf(T_TRUE,"Recv Msg:%s\n", buf);
				continue;
			}
		}
		
	}

err:	close(epollfd);
err1:	ClientSockClose(&sock);
	return 0;
}

