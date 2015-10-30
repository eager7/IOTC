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
#include <libdaemon/daemon.h>

#include "utils.h"
#include "socket_server.h"

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
	int daemonize = 1;
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
	daemon_log_ident = daemon_ident_from_argv0(argv[0]);/* Log everything into syslog */

    if (daemonize)
    {
        /* Prepare for return value passing from the initialization procedure of the daemon process */
        if (daemon_retval_init() < 0) {
            daemon_log(LOG_ERR, "Failed to create pipe.");
            return 1;
        }

		pid_t pid;
        if ((pid = daemon_fork()) < 0)/* Do the fork */
        {          
            daemon_log(LOG_ERR, "Failed to fork() daemon process.");/* Exit on error */
            daemon_retval_done();
            return 1;
        } 
        else if (pid)/* The parent */
        { 
            int ret;
            /* Wait for 20 seconds for the return value passed from the daemon process */
            if ((ret = daemon_retval_wait(20)) < 0)
            {
                daemon_log(LOG_ERR, "Could not recieve return value from daemon process: %s", strerror(errno));
                return 255;
            }
			
            if (ret == 0)
            {
                daemon_log(LOG_INFO, "Daemon process started.");
            }
            else
            {
                daemon_log(LOG_ERR, "Daemon returned %i.", ret);
            }
            return ret;
        } 
        else/* The daemon */
        { 
            if (daemon_close_all(-1) < 0)/* Close FDs */
            {
                daemon_log(LOG_ERR, "Failed to close all file descriptors: %s", strerror(errno));
                daemon_retval_send(1);/* Send the error condition to the parent process */
                goto finish;
            }
            daemon_log_use = DAEMON_LOG_SYSLOG;
            daemon_retval_send(0);/* Send OK to parent process */
            daemon_log(LOG_INFO, "Daemon started");
        }
    }
    else/* Running foreground - set verbosity */
    {
        if ((verbosity != LOG_INFO) && (verbosity != LOG_NOTICE))
        {
            if (verbosity > LOG_DEBUG)/*LOG_DEBUG = 7*/
            {
                daemon_set_verbosity(LOG_DEBUG);
            }
            else
            {
                daemon_set_verbosity(verbosity);
            }
        }
    }
	    
    signal(SIGTERM, vQuitSignalHandler);/* Install signal handlers */
    signal(SIGINT,  vQuitSignalHandler);		
	
	if ((SocketServerInit(PORT_SOCKET, NULL) != E_SOCK_OK))
	{
		ERR_vPrintf(T_TRUE, "Init compents failed \n");
		goto finish;
	}

	while(bRunning)
	{
		//Printf Device List
		sleep(1);//dispatch thread
	}
	
	SocketServerFinished();
finish:
    if (daemonize)
        daemon_log(LOG_INFO, "Daemon process exiting");  
    else
        daemon_log(LOG_INFO, "Exiting");

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
    SocketServerFinished();
	exit(0);
    return;
}
