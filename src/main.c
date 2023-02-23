#include <stdlib.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <syslog.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#include "argpfuncs.h"
#include "tuyafuncs.h"
#include "becomedaemon.h"

static void handle_kill(int signum);


struct arguments arguments = { {[0 ... 29] = '\0'}, {[0 ... 29] = '\0'}, {[0 ... 29] = '\0'}, {[0 ... 1023] = '\0'}, 0 };

int main(int argc, char** argv)
{
    int ret;

    start_parser(argc, argv, &arguments);
    printf("Starting...\n");

    openlog("mylog", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL0);
    
    if(arguments.daemon != 0){
        ret = become_daemon();
        if(ret){
            syslog(LOG_USER | LOG_ERR, "(TUYA) Error starting daemon");
            closelog();
            return EXIT_FAILURE;
        }
        syslog(LOG_INFO, "(TUYA) Daemon started");
    }
    else{
        syslog(LOG_INFO, "(TUYA) Program started");
    }

    signal(SIGKILL, handle_kill);
    signal(SIGTERM, handle_kill);
    signal(SIGINT, handle_kill);
    signal(SIGUSR1, handle_kill);

    
    ret = tuya_connect(arguments.product_id, arguments.device_id, arguments.device_secret);
    if(ret != 0){
        printf("Error: Failed to connect, stopping program...\n");
        return 1;
    }

    struct sysinfo info;
    char message[200];
    ret = 0;
    while(ret == 0){
        sleep(3);
        if(sysinfo(&info) != 0){
            printf("Error: Failed to get system information\n");
            sprintf(message, "{\"free_ram\":%d,\"total_ram\":%d}", 0,0);
        }
        else{
            sprintf(message, "{\"free_ram\":%ld,\"total_ram\":%ld}", info.freeram, info.totalram);
        }
        ret = tuya_loop(message);
    }

    tuya_disconnect();

    exit(ret);
}
static void handle_kill(int signum)
{
    // deallocation goes here


    if(signum == 8){ // SIGKILL (before tuya conncetion)
        signal(SIGKILL, SIG_DFL);
        raise(SIGKILL);
    }
    else if(signum == 15){ // SIGTERM (before tuya conncetion)
        signal(SIGTERM, SIG_DFL);
        raise(SIGTERM);
    }
    else if(signum == 2){ // SIGINT (before tuya conncetion)
        signal(SIGINT, SIG_DFL);
        raise(SIGINT);
    }
}