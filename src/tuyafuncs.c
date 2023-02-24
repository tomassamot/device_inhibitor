#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>
#include <assert.h>
#include <time.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <signal.h>

#include <cJSON.h>
#include <tuya_error_code.h>
#include <system_interface.h>
#include <mqtt_client_interface.h>
#include <tuyalink_core.h>

#include "tuya_cacert.h"

static void on_connected(tuya_mqtt_context_t* context, void* user_data);
static void on_disconnect(tuya_mqtt_context_t* context, void* user_data);
static void on_messages(tuya_mqtt_context_t* context, void* user_data, const tuyalink_message_t* msg);
static void write_message_to_file(char msg[]);
static void handle_kill(int signum);




tuya_mqtt_context_t client_instance;

const char *product_id = "";
const char *device_id = "";
const char *device_secret = "";

int tuya_connect(char *recv_product_id, char *recv_device_id, char *recv_device_secret)
{
    product_id = recv_product_id;
    device_id = recv_device_id;
    device_secret = recv_device_secret;

    signal(SIGKILL, handle_kill);
    signal(SIGTERM, handle_kill);
    signal(SIGINT, handle_kill);

    int ret = OPRT_OK;

    ret = tuya_mqtt_init(&client_instance, &(const tuya_mqtt_config_t) {
        .host = "m1.tuyacn.com",
        .port = 8883,
        .cacert = tuya_cacert_pem,
        .cacert_len = sizeof(tuya_cacert_pem),
        .device_id = device_id,
        .device_secret = device_secret,
        .keepalive = 100,
        .timeout_ms = 2000,
        .on_connected = on_connected,
        .on_disconnect = on_disconnect,
        .on_messages = on_messages
    });
    assert(ret == OPRT_OK);

    ret = tuya_mqtt_connect(&client_instance);
    assert(ret == OPRT_OK);

    return ret;
}
int tuya_loop(char json_msg[])
{   
    syslog(LOG_INFO, "%s(%s) %s", "TUYA", device_id, "Sending information to cloud");
    tuyalink_thing_property_report(&client_instance, NULL, json_msg);

    int ret = tuya_mqtt_loop(&client_instance);
    return ret;
}
void tuya_disconnect()
{
    syslog(LOG_INFO, "%s(%s) %s", "TUYA", device_id, "Disconnecting from Tuya cloud...");
    closelog();
    tuya_mqtt_disconnect(&client_instance);
    tuya_mqtt_deinit(&client_instance);
}
static void on_connected(tuya_mqtt_context_t* context, void* user_data)
{
    syslog(LOG_INFO, "%s(%s) %s", "TUYA", device_id, "Connected to Tuya cloud");
}
static void on_disconnect(tuya_mqtt_context_t* context, void* user_data)
{

}
static void on_messages(tuya_mqtt_context_t* context, void* user_data, const tuyalink_message_t* msg)
{
    syslog(LOG_INFO, "%s(%s) %s", "TUYA", device_id, "Message received from cloud");
    write_message_to_file(msg->data_string);
}
static void write_message_to_file(char *msg)
{
    FILE *msg_file = NULL;
    time_t rawtime = time(0);
    struct tm *timeinfo = localtime(&rawtime);
    size_t path_len = 100;
    char path[path_len];

    getcwd(path, path_len);
    strcat(path, "/received_messsages.txt");

    msg_file = fopen(path, "a");
    if(msg_file == NULL){
        syslog(LOG_ERR, "%s(%s) %s %s", "TUYA", device_id, "Failed to open received_messages.txt for appending in path:", path);
        return;
    }

    syslog(LOG_ERR, "%s(%s) %s", "TUYA", device_id, "Saving message in received_messages.txt");
    if(msg == NULL){
        fprintf(msg_file, "%s: %s\n", asctime(timeinfo), "(Empty message)");
    }
    else{
        fprintf(msg_file, "%s: %s\n", asctime(timeinfo), msg);
    }
    
    fclose(msg_file);
}
static void handle_kill(int signum)
{
    // deallocation goes here

    syslog(LOG_INFO, "(TUYA) Request to kill detected");
    raise(SIGUSR1);
}