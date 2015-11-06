/****************************************************************************
 *
 * MODULE:             socket_client.c
 *
 * COMPONENT:          Utils interface
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
#include "list.h"
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
char SystemIp[16];
char buf[BUFSIZE];
//pthread_mutex_t	mutex; 

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

void signalHandler(int signal)
{
	if(signal == SIGALRM){
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
		ERR_vPrintf("argument error\n"); 
        return E_ERROR;
	}

	sock->type = type;
	sock->port = port;

	sock->fd = socket(AF_INET, type, 0);//new socket
	if(sock->fd < 0){
		ERR_vPrintf("socket failed\n"); 
        return E_ERROR;
	}
	DEB_vPrintf("socket Success\n");

	/*set sock unblock*/
	int flags;
	flags = fcntl(sockfd, F_GETFL, 0);
	if(flags < 0) {
		ERR_vPrintf("fcntl get failed\n"); 
	    goto err;     
	}
	if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) < 0) {
        ERR_vPrintf("fcntl set failed\n"); 
        goto err;
    }
	DEB_vPrintf("fcntl Success\n");

	/*get connect ip*/
	eRetStatus ret;
	memset(&(sock->addr), 0, IP_LEN);
	ret = getSystemIp(ifname, &(sock->addr), IP_LEN);
	if(ret != E_OK){
		ERR_vPrintf("get system ip failed\n"); 
		goto err;
	}
	DEB_vPrintf("Connect Server Success\n");


	/*Connect*/
	struct sockaddr_in server;
	bzero(&server,sizeof(server));
	server.sin_family = sock->type;
	server.sin_port = htons(sock->port);
	server.sin_addr.s_addr = inet_addr(&(sock->addr));
	ret = connect(sockfd, (struct sockaddr *)&server, sizeof(server));
	while (ret < 0) {
		if (errno == EINPROGRESS) {
			break;
		} 
		else {
			ERR_vPrintf("connect failed\n"); 
			goto err;
        }
    }
	DEB_vPrintf("Connect Server Success\n");
	
	
	return E_OK;

err:	close(sock->fd);
	return E_ERROR;
}




eRetStatus send_msg(int fd)
{
	int ret;
	//char *msg = "[{\"device_name\":\"light\",\"device_id\":1,\"device_index\":12345678},{\"device_name\":\"dimmerlight\",\"device_id\": 2,\"device_index\": 87654321}]";
	char *msg = "{"
    "\"sequence_no\": 1,"
    "\"message_type\": 1,"
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


int main(void)
{
	ClientSock sock;

	eRetStatus ret;
	ret = ClientSockInit(&sock, AF_INET, PORT, "eth0.2");
	if(ret != E_OK){
		ERR_vPrintf(T_TRUE,"socket send error!\n");
		return -1;
	}

	//creat msg send thread
	

	ret = send_msg(sock.fd);
	if(E_OK != ret)
	{
		ERR_vPrintf(T_TRUE,"socket send error!\n"); 
		return -1;
	}

	//creat timer
	//pthread_mutex_init(&mutex, NULL);
	signal(SIGALRM, signalHandler);  
	struct itimerval timer, old_timer;

	timer.it_value.tv_sec = 10;
	timer.it_value.tv_usec = 1000;
	timer.it_interval.tv_sec = 5;
	timer.it_interval.tv_usec = 0;

	//setitimer(ITIMER_REAL, &timer, &old_timer);

	int epollfd;

	epollfd = epoll_create(10);

	struct epoll_event ev;

	ev.data.fd = sockfd;
	ev.data.fd = EPOLLIN|EPOLLERR|EPOLLHUP;

	ret = epoll_ctl(epollfd,EPOLL_CTL_ADD, sockfd, &ev);
	if(ret < 0){
		perror("epoll_ctl failed ");
		goto done;
	}

	
	int iEpollRet;
	printf("Enter epoll Wait...\n");	
	while(1){
		iEpollRet = epoll_wait(epollfd, &ev, EPOLL_EVENT_NUM, -1);
		if(iEpollRet == 0){//epoll_wait or timeout
			ERR_vPrintf(T_TRUE,"socket failed\n"); 
			goto done;
		}
		else if(iEpollRet == -1){//error
			perror("epoll_wait failed ");
			//if(errno == EAGAIN)
			goto done;
		}else{
			if((ev.events & EPOLLERR) || (ev.events & EPOLLHUP)){
				//close socket
				printf("Socket Disconnected.. C\n");
				goto done;
			}
			else{
				//read event
				memset(buf, 0, BUFSIZE);
				recv(sockfd, buf, BUFSIZE, 0);
				BLUE_vPrintf(T_TRUE,"Recv Msg:%s\n", buf);
				//continue;
			}
		}

		//send event
		
	}
done:

	close(epollfd);
	close(sockfd);
	return 0;
}




int main(void)
{
	//init sock

	//creat sock client


	//creat timer

	//epoll creat

	//
}

