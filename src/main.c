#include <stdlib.h>
#include <sys/types.h>
#include <syslog.h>
#include <unistd.h>

#include "argpfuncs.h"
#include "tuyafuncs.h"
#include "becomedaemon.h"

int main(int argc, char** argv)
{
    struct arguments *arguments;
    pid_t pid = getpid();
    pid_t ppid = getppid();
    int ret;

    arguments->product_id = (char*) malloc(sizeof(char)*30);
    arguments->device_id = (char*) malloc(sizeof(char)*30);
    arguments->device_secret = (char*) malloc(sizeof(char)*30);
    arguments->config_file_path = (char*) malloc(sizeof(char)*60);

    start_parser(argc, argv, &arguments);

    /*ret = become_daemon();
    if(ret){
        syslog(LOG_USER | LOG_ERR, "error starting");
        closelog();
        return EXIT_FAILURE;
    }*/

    openlog("mydaemonlog", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL0);

    syslog(LOG_INFO, "(TUYA) Daemon started");
    
    ret = start_tuya_connection(arguments->product_id, arguments->device_id, arguments->device_secret);

    free(arguments->product_id);
    free(arguments->device_id);
    free(arguments->device_secret);
    free(arguments->config_file_path);
    closelog();

    exit(ret);
}