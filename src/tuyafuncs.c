#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>
#include <assert.h>
#include <time.h>
#include <string.h>

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
static void interrupt_handler(int signum);

tuya_mqtt_context_t client_instance;

const char *product_id = "";
const char *device_id = "";
const char *device_secret = "";


int start_tuya_connection(char *recv_product_id, char *recv_device_id, char *recv_device_secret)
{
    product_id = recv_product_id;
    device_id = recv_device_id;
    device_secret = recv_device_secret;

    int ret = OPRT_OK;

    tuya_mqtt_context_t* client = &client_instance;

    ret = tuya_mqtt_init(client, &(const tuya_mqtt_config_t) {
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

    ret = tuya_mqtt_connect(client);
    assert(ret == OPRT_OK);

    for (;;) {
        // Loop to receive packets, and handles client keepalive
        tuya_mqtt_loop(client);
    }

    return ret;
}
static void on_connected(tuya_mqtt_context_t* context, void* user_data)
{
    syslog(LOG_INFO, "%s(%s) %s", "TUYA", device_id, "Connection to Tuya Cloud started");


    int i = 0;
    while(i < 2){
        if (i != 0)
            sleep(3);
        syslog(LOG_INFO, "%s(%s) %s", "TUYA", device_id, "Sending device property report to cloud");

        tuyalink_thing_data_model_get(context, NULL);
        tuyalink_thing_desired_get(context, NULL, "[\"switch10\"]");
        tuyalink_thing_property_report(context, NULL, "{\"power\":{\"value\":1234,\"time\":1631708204231}}");
        tuyalink_thing_property_report_with_ack(context, NULL, "{\"power\":{\"value\":1234,\"time\":1631708204231}}");
        i++;
    }
}
static void on_disconnect(tuya_mqtt_context_t* context, void* user_data)
{
    syslog(LOG_INFO, "%s(%s) %s", "TUYA", device_id, "Disconnecting from Tuya cloud");
    closelog();
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