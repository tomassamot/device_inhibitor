#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <syslog.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <argp.h>
#include "argpfuncs.h"
#include "tuyafuncs.h"
#include "becomedaemon.h"
#include "ubusfuncs.h"

static void board_cb(struct ubus_request *req, int type, struct blob_attr *msg);
static void handle_kill(int signum);


struct arguments arguments = { {[0 ... 29] = '\0'}, {[0 ... 29] = '\0'}, {[0 ... 29] = '\0'}, {[0 ... 1023] = '\0'}, 0 };
int program_is_killed = 0;

int rc = 0;

int main(int argc, char** argv)
{
    int ret;
    struct ubus_context *context;
    uint32_t id;

    start_parser(argc, argv, &arguments);
    printf("Starting...\n");

    openlog("device_inhibitor", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL0);
    
    if(arguments.daemon != 0){
        ret = become_daemon();
        if(ret){
            syslog(LOG_ERR, "Error starting daemon");
            closelog();
            return EXIT_FAILURE;
        }
        syslog(LOG_INFO, "Daemon started");
    }
    else{
        syslog(LOG_INFO, "Program started");
    }

    signal(SIGKILL, handle_kill);
    signal(SIGTERM, handle_kill);
    signal(SIGINT, handle_kill);
    signal(SIGUSR1, handle_kill);

    ret = tuya_connect(arguments.product_id, arguments.device_id, arguments.device_secret);
    if(ret != 0){
        return ret;
    }
    
    connect_to_ubus(&context);
    if(context == NULL){
        printf("Failed to connect to ubus\n");
        return 1;
    }
    ubus_lookup_id(context, "system", &id);

    syslog(LOG_INFO, "Beginning to send router RAM information to cloud");
    ret = 0;
    while(ret == 0 && program_is_killed == 0){
        sleep(5);
        ubus_invoke(context, id, "info", NULL, board_cb, NULL, 3000);
    }

    disconnect_from_ubus(context);
    tuya_disconnect();

    return(0);
}
static void board_cb(struct ubus_request *req, int type, struct blob_attr *msg)
{
    char message[200];
	struct blob_buf *buf = (struct blob_buf *)req->priv;
	struct blob_attr *tb[__INFO_MAX];
	struct blob_attr *memory[__MEMORY_MAX];

	blobmsg_parse(info_policy, __INFO_MAX, tb, blob_data(msg), blob_len(msg));

	if (!tb[MEMORY_DATA]) {
		syslog(LOG_WARNING, "No memory data received");
        rc--;
		return;
	}

	blobmsg_parse(memory_policy, __MEMORY_MAX, memory, blobmsg_data(tb[MEMORY_DATA]), blobmsg_data_len(tb[MEMORY_DATA]));

    sprintf(message, "{\"free_ram\":%lld,\"total_ram\":%lld}", blobmsg_get_u64(memory[FREE_MEMORY]), blobmsg_get_u64(memory[TOTAL_MEMORY]));
    tuya_loop(message);
}
static void handle_kill(int signum)
{
    // deallocation goes here

    //
    program_is_killed = 1;
}
