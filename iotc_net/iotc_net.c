/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <syslog.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "utils.h"
#include "msocket.h"
/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/
static void print_net_usage_exit(char *argv[]);
static void iotc_net_init(int argc, char *argv[]);
static void daemonize_init(const char *cmd);



/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/
char const *Version = "v0.1";
uint8 verbosity = 0;


/****************************************************************************/
/***        Local    Functions                                            ***/
/****************************************************************************/
int main(int argc, char *argv[])
{
    printf("this is iotc_net daemon\n");

    iotc_net_init(argc, argv);

    while(1)
    {
        printf("loop..\n");
        sleep(1);
    }
    return 0;
}

static void vQuitSignalHandler (int sig)
{
    DBG_vPrintf(verbosity, "Got signal %d\n", sig); 
    exit(0);
    return;
}

static void iotc_net_init(int argc, char *argv[])
{
    signed char opt = 0;
    int option_index = 0;
    int daemonize = 0;
    static struct option long_options[] =
    {
        {"help",                    no_argument,        NULL, 'h'},
        {"verbosity",               required_argument,  NULL, 'v'},
        {"deamon",                  no_argument,        NULL, 'f'},
        { NULL, 0, NULL, 0}
    };

    while ((opt = getopt_long(argc, argv, "hfv:", long_options, &option_index)) != -1)
    {
         switch (opt) 
        {
            case 'h':
                print_net_usage_exit(argv);
            break;
            case 'f':
                daemonize = 0;
            break;
            case 'v':
                verbosity = atoi(optarg);
            break;
            default:
            print_net_usage_exit(argv);  
        }
    }
    if(daemonize)
    {
        daemonize_init("iotc net");
    }
    signal(SIGTERM, vQuitSignalHandler);/* Install signal handlers */
    signal(SIGINT,  vQuitSignalHandler);
    return;
}

static void daemonize_init(const char *cmd)
{
    int i, fd0, fd1, fd2;
    pid_t pid;
    struct rlimit rl;
    struct sigaction sa;
    umask(0);
    if (getrlimit(RLIMIT_NOFILE, &rl) < 0)
    {
        ERR_vPrintf(T_TRUE,"%s: can't get file limit", cmd);
        exit(-1);
    }
    if ((pid = fork()) < 0)
    {
        ERR_vPrintf(T_TRUE,"%s: can't fork, exit(-1)\n", cmd);
        exit(-1);
    }
    else if (pid != 0) /* parent */
    {
        DBG_vPrintf(T_TRUE,"This is Parent Program, exit(0)\n");
        exit(0);
    }
    setsid();
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGHUP, &sa, NULL) < 0)
    {
        ERR_vPrintf(T_TRUE,"%s: can't ignore SIGHUP", cmd);
        exit(-1);
    }
    if ((pid = fork()) < 0)
    {
        ERR_vPrintf(T_TRUE,"%s: can't fork, exit(-1)\n", cmd);
        exit(-1);
    }
    else if (pid != 0) /* parent */
    {
        DBG_vPrintf(T_TRUE,"This is Parent Program, exit(0)\n");
        exit(0);
    }

    if (chdir("/") < 0)
    {
        ERR_vPrintf(T_TRUE,"%s: can,t change directory to /", cmd);
        exit(-1);
    }

    if (rl.rlim_max == RLIM_INFINITY)
    {
        rl.rlim_max = 1024;
    }
    for (i = 0; i < rl.rlim_max; i++)
    {
        close(i);
    }

    fd0 = open("/dev/null", O_RDWR);
    fd1 = dup(0);
    fd2 = dup(0);

    openlog(cmd, LOG_CONS, LOG_DAEMON);
    if (fd0 != 0 || fd1 != 1 || fd2 != 2) 
    {
        ERR_vPrintf(T_TRUE, "unexpected file descriptors %d %d %d",fd0, fd1, fd2);
        exit(1);
    }
}

static void print_net_usage_exit(char *argv[])
{
    fprintf(stderr, "\t******************************************************\n");
    fprintf(stderr, "\t*         iotc net   Version: %s            *\n", Version);
    fprintf(stderr, "\t******************************************************\n");
    fprintf(stderr, "\t************************Release***********************\n");
    
    fprintf(stderr, "Usage: %s\n", argv[0]);
    fprintf(stderr, "  Arguments:\n");
    fprintf(stderr, "  Options:\n");
    fprintf(stderr, "    -h --help                              Print this help.\n");
    fprintf(stderr, "    -v --verbosity     <verbosity>         Verbosity level. Increases amount of debug information. Default off.\n");
    fprintf(stderr, "    -f --daemon        <front or daemon>   Is front or daemon, Default daenom\n");
    exit(0);
}