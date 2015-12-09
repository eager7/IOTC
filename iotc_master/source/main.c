/****************************************************************************
 *
 * MODULE:             IOTC - Internet of things control daemon
 *
 * COMPONENT:          main
 *
 * REVISION:           $Revision: 01 $
 *
 * DATED:              $Date: 2015-10-30 11:13:17 +0100 (Fri, 30 Oct 2015) $
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
#include <stdio.h>
#include <getopt.h>
#include <signal.h>
#include <syslog.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "utils.h"
#include "socket_server.h"
#include "iotc_network.h"
#include "iotc_devices.h"
#include "iotc_application.h"
#include "iotc_multicast.h"
#include "iotc_broadcast.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#define DBG_MAIN 1
#define PORT_SOCKET 7788
/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/
const char *Version = "0.1";
int verbosity = 0;
volatile sig_atomic_t bRunning = 1;/** Main loop running flag */


/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/
static void vQuitSignalHandler (int sig);
static void print_usage_exit(char *argv[]);
static void daemonize_init(const char *cmd);

/****************************************************************************/
/***        Locate   Functions                                            ***/
/****************************************************************************/
int main(int argc, char *argv[])
{
    printf("Start IOTC Daemon Program...\n");

    static struct option long_options[] =
    {
        {"help",                    no_argument,        NULL, 'h'},
        {"verbosity",               required_argument,  NULL, 'v'},
        {"foreground",              no_argument,        NULL, 'f'},
        { NULL, 0, NULL, 0}
    };

    signed char opt = 0;
    int option_index = 0;
    int daemonize = 0;
    while ((opt = getopt_long(argc, argv, "s:hfv:B:I:P:m:nc:p:6:", long_options, &option_index)) != -1) 
    {
        switch (opt) 
        {
            case 'h':
                print_usage_exit(argv);
            break;
            case 'f':
                daemonize = 0;
            break;
            case 'v':
                verbosity = atoi(optarg);
            break;
            case 0:
            break;
            default: /* '?' */
                print_usage_exit(argv);
        }
    }

    if (daemonize)
    {
        daemonize_init("iotc");
    }
    
    signal(SIGTERM, vQuitSignalHandler);/* Install signal handlers */
    signal(SIGINT,  vQuitSignalHandler);
    UtilsRegisterSignalHandler();

    if ((SocketServerInit(PORT_SOCKET, NULL) != E_SOCK_OK) || 
        (IotcNetworkInit() != E_NETWORK_OK) || 
        (IotcApplicationkInit() != E_APPLICATION_OK) || 
        (IotcBroadcastInit() != E_BROAD_OK)
        /*(IotcMulticastInit() != E_MULTI_OK)*/)
    {
        ERR_vPrintf(T_TRUE, "Init compents failed \n");
        goto finish;
    }

    while(bRunning)
    {
        //Printf Device List
        PURPLE_vPrintf(DBG_MAIN, "Display The Devices List...\n");
        pthread_mutex_lock(&sIotcDeviceHead.mutex);
        tsIotcDevice *psIotcDeviceTemp = NULL;
        dl_list_for_each(psIotcDeviceTemp, &sIotcDeviceHead.list, tsIotcDevice, list)
        {
            DBG_vPrintf(DBG_MAIN, "The Device Index is 0x%016llx, socket is %d\n", 
                    psIotcDeviceTemp->u64DeviceIndex, psIotcDeviceTemp->iSocketClientFd);
        }
        pthread_mutex_unlock(&sIotcDeviceHead.mutex);
        
        sleep(3);//dispatch thread
    }

    SocketServerFinished();
    IotcNetworkFinished();
    IotcApplicationFinished();
    finish:
    if (daemonize)
        syslog(LOG_INFO, "Daemon process exiting");  
    else
        syslog(LOG_INFO, "Exiting");

    return 0;
}

static void print_usage_exit(char *argv[])
{
    fprintf(stderr, "******************************************************\n");
    fprintf(stderr, "*   Internet of Things Control Daemon Version: %s    *\n", Version);
    fprintf(stderr, "******************************************************\n");
    
    fprintf(stderr, "\nUsage: %s\n", argv[0]);
    fprintf(stderr, "Arguments:\n");
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "    -h --help                              Print this help.\n");
    fprintf(stderr, "    -v --verbosity     <verbosity>         Verbosity level. Increases amount of debug information. Default off.\n");
    fprintf(stderr, "    -f --foreground                        Do not detatch daemon process, run in foreground.\n");
    exit(0);
}

static void vQuitSignalHandler (int sig)
{
    DBG_vPrintf(DBG_MAIN, "Got signal %d\n", sig); 
    bRunning = 0;
    return;
}

static void daemonize_init(const char *cmd)
{
    int i, fd0, fd1, fd2;
    pid_t pid;
    struct rlimit rl;
    struct sigaction sa;
    /*
    * Clear file creation mask.
    */
    umask(0);
    /*
    * Get maximum number of file descriptors.
    */
    if (getrlimit(RLIMIT_NOFILE, &rl) < 0)
    {
        printf("%s: can't get file limit", cmd);
        exit(-1);
    }
    /*
    * Become a session leader to lose controlling TTY.
    */
    if ((pid = fork()) < 0)
    {
        printf("%s: can't fork", cmd);
        exit(-1);
    }
    else if (pid != 0) /* parent */
    {
        exit(0);
    }

    setsid();
    /*
    * Ensure future opens won't allocate controlling TTYs.
    */
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGHUP, &sa, NULL) < 0)
    {
        printf("%s: can't ignore SIGHUP", cmd);
        exit(-1);
    }
    if ((pid = fork()) < 0)
    {
        printf("%s: can't fork", cmd);
        exit(-1);
    }
    else if (pid != 0) /* parent */
    {
        exit(0);
    }
    /*
    * Change the current working directory to the root so
    * we won't prevent file systems from being unmounted.
    */
    if (chdir("/") < 0)
    {
        printf("%s: can,t change directory to /", cmd);
        exit(-1);
    }
    /*
    * Close all open file descriptors.
    */
    if (rl.rlim_max == RLIM_INFINITY)
    {
        rl.rlim_max = 1024;
    }
    for (i = 0; i < rl.rlim_max; i++)
    {
        close(i);
    }
    /*
    * Attach file descriptors 0, 1, and 2 to /dev/null.
    */
    fd0 = open("/dev/null", O_RDWR);
    fd1 = dup(0);
    fd2 = dup(0);
    /*
    * Initialize the log file.
    */
    openlog(cmd, LOG_CONS, LOG_DAEMON);
    if (fd0 != 0 || fd1 != 1 || fd2 != 2) 
    {
        syslog(LOG_ERR, "unexpected file descriptors %d %d %d",fd0, fd1, fd2);
        exit(1);
    }
}

