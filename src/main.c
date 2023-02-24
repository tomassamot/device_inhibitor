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
int program_is_killed = 0;

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
        syslog(LOG_ERR, "(TUYA) Error: Failed to connect, stopping program...\n");
        return 1;
    }

    struct sysinfo info;
    char message[200];
    ret = 0;
    while(ret == 0 && program_is_killed == 0){
        sleep(3);
        if(sysinfo(&info) != 0){
            syslog(LOG_ERR, "(TUYA) Error: Failed to get system information\n");
            sprintf(message, "{\"free_ram\":%d,\"total_ram\":%d}", 0,0);
        }
        else{
            sprintf(message, "{\"free_ram\":%ld,\"total_ram\":%ld}", info.freeram, info.totalram);
        }
        ret = tuya_loop(message);
    }

    tuya_disconnect();

    return(0);
}
static void handle_kill(int signum)
{
    // deallocation goes here

    program_is_killed = 1;
}