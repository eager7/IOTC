/****************************************************************************
 *
 * MODULE:             socket_client.c
 *
 * COMPONENT:          Utils interface
 *
 * REVISION:           $Revision:  0$
 *
 * DATED:              $Date: 2015-10-21 15:13:17 +0100 (Thu, 21 Oct 2015 $
 *
 * AUTHOR:             Abel
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


/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#define PORT 7788
#define BUFSIZE	2048
#define EPOLL_EVENT_NUM  10
 
#define UI_BLUE(x)      "\e[34;1m"x"\e[0m"
#define UI_RED(x)       "\e[31;1m"x"\e[0m"

typedef enum
{
	E_OK = 0,
	E_ERROR,
}eRetStatus;

typedef enum
{
    T_FALSE = 0,
    T_TRUE  = 1,
}bool_t;


#define BLUE_vPrintf(a,b,ARGS...)   do {  if (a) {printf(UI_BLUE     ("[PCT_%d]") b, __LINE__, ## ARGS);} } while(0)
#define RED_vPrintf(a,b,ARGS...)    do {  if (a) {printf(UI_RED      ("[PCT_%d]") b, __LINE__, ## ARGS);} } while(0)
#define ERR_vPrintf RED_vPrintf


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
        return E_ERROR;
    }

    close(fd);

    tmp = inet_ntoa(((struct sockaddr_in*)&(ifr_ip.ifr_addr))->sin_addr);

    strncpy(pIP, tmp, len);

    return E_OK;
}


eRetStatus send_msg(int fd)
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


int main(void)
{
	printf("Hello..\n");
	int sockfd, num;    /* files descriptors */
	struct sockaddr_in server;
	int ret;

	if((sockfd=socket(AF_INET,SOCK_STREAM, 0))==-1)
	{
		ERR_vPrintf(T_TRUE,"socket failed, %s\n", strerror(errno)); 
		return -1;
	}

	int flags = fcntl(sockfd, F_GETFL, 0);
	if(flags < 0) {
	    return -1;      
	}
	if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) < 0) {
        perror("set socket O_NONBLOCK fail.\n");
        return -1;
    }

	bzero(&server,sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(PORT);
	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	ret = connect(sockfd, (struct sockaddr *)&server, sizeof(server));
	while (ret < 0) {
		if (errno == EINPROGRESS) {
			break;
		} 
		else {
			perror("connect remote server fail.\n");
            return -1;
        }
    }
	printf("Connect Server Success\n");
	ret = send_msg(sockfd);
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



