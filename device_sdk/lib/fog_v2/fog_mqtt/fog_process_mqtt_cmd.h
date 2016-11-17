#ifndef _PORCESS_MQTT_CMD_H_
#define _PORCESS_MQTT_CMD_H_

#include "mico.h"

#define FOG_MQTT_CMD_BIND                   (1000) //app端解绑子设备
#define FOG_MQTT_CMD_UNBIND                 (1001) //app端解绑父设备
#define FOG_MQTT_CMD_SUBDEVICE_WAIT_BIND    (1002) //让父设备处于等待子设备绑定的状态


extern OSStatus process_fog_v2_mqtt_cmd(const char *cmd_payload);

#endif
