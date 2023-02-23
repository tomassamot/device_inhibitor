#include <stdlib.h>
#include <sys/types.h>
#include <syslog.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#include "argpfuncs.h"
#include "tuyafuncs.h"
#include "becomedaemon.h"

static void handle_kill(int signum);


struct arguments arguments = { {[0 ... 29] = '\0'}, {[0 ... 29] = '\0'}, {[0 ... 29] = '\0'}, {[0 ... 1023] = '\0'} };

int main(int argc, char** argv)
{
    int ret;

    start_parser(argc, argv, &arguments);

    ret = become_daemon();
    if(ret){
        syslog(LOG_USER | LOG_ERR, "(TUYA) Error starting daemon");
        closelog();
        return EXIT_FAILURE;
    }

    openlog("mydaemonlog", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL0);

    syslog(LOG_INFO, "(TUYA) Daemon started");

    signal(SIGKILL, handle_kill);
    signal(SIGTERM, handle_kill);
    signal(SIGQUIT, handle_kill);
    
    ret = start_tuya_connection(arguments.product_id, arguments.device_id, arguments.device_secret);

    closelog();

    exit(ret);
}
static void handle_kill(int signum)
{
    // deallocation goes here
    
    syslog(LOG_INFO, "(TUYA) Request to kill detected, stopping program...");
    closelog();

    if(signum == 8){ // SIGKILL
        signal(SIGKILL, SIG_DFL);
        raise(SIGKILL);
    }
    else if(signum == 15){ // SIGTERM
        signal(SIGTERM, SIG_DFL);
        raise(SIGTERM);
    }
    else{
        signal(SIGINT, SIG_DFL);
        raise(SIGINT);
    }
}