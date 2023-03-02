#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>
#include <assert.h>
#include <time.h>
#include <string.h>
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


int tuya_connect(char *product_id, char *device_id, char *device_secret)
{
    int ret = OPRT_OK;

    signal(SIGKILL, handle_kill);
    signal(SIGTERM, handle_kill);
    signal(SIGINT, handle_kill);

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
    if( ret != OPRT_OK ) {
        syslog(LOG_ERR, "Failed to initialize Tuya context");
    }
    syslog(LOG_INFO, "Successfully initialized Tuya context");
    
    ret = tuya_mqtt_connect(&client_instance);
    if( ret != OPRT_OK ) {
        syslog(LOG_ERR, "Failed to start connection with Tuya cloud");
    }

    return ret;
}
int tuya_loop(char json_msg[])
{   
    tuyalink_thing_property_report(&client_instance, NULL, json_msg);
    
    int ret = tuya_mqtt_loop(&client_instance);

    return ret;
}
void tuya_disconnect()
{
    tuya_mqtt_disconnect(&client_instance);
    syslog(LOG_INFO, "Disconnected from Tuya cloud");
    closelog();
    tuya_mqtt_deinit(&client_instance);
}

static void on_connected(tuya_mqtt_context_t* context, void* user_data)
{
    syslog(LOG_INFO, "Connected to Tuya cloud");
}
static void on_disconnect(tuya_mqtt_context_t* context, void* user_data)
{

}
static void on_messages(tuya_mqtt_context_t* context, void* user_data, const tuyalink_message_t* msg)
{
    syslog(LOG_INFO, "Message received from Tuya cloud");
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
        syslog(LOG_WARNING, "Failed to open received_messages.txt for appending in path: %s", path);
        return;
    }

    if(msg == NULL){
        syslog(LOG_WARNING, "Message received was empty");
        fprintf(msg_file, "%s: %s\n", asctime(timeinfo), "(Empty message)");
    }
    else{
        fprintf(msg_file, "%s: %s\n", asctime(timeinfo), msg);
    }
    
    fclose(msg_file);
}
static void handle_kill(int signum)
{
    syslog(LOG_INFO, "Request to kill detected");
    // deallocation goes here

    //
    raise(SIGUSR1);
}