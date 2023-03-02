#ifndef TUYAFUNCS_H
#define TUYAFUNCS_H

/*#include <cJSON.h>
#include <tuya_error_code.h>
#include <system_interface.h>
#include <mqtt_client_interface.h>
#include <tuyalink_core.h>*/

int tuya_connect(char *product_id, char *device_id, char *device_secret);
int tuya_loop(char json_msg[]);
void tuya_disconnect();

#endif