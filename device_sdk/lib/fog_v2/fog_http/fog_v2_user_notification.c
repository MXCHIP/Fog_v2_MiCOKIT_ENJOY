#include "mico.h"
#include "fog_v2_include.h"

#define app_log(M, ...)                     custom_log("FOG_V2_USER_NOTIFICATION", M, ##__VA_ARGS__)

#if (FOG_V2_USE_SUB_DEVICE == 1)

WEAK void user_fog_v2_device_notification(SUBDEVICE_CMD_TYPE type, const char *s_product_id, const char *s_mac);

WEAK void user_fog_v2_device_notification(SUBDEVICE_CMD_TYPE type, const char *s_product_id, const char *s_mac)
{
//    if(type == MQTT_CMD_SUB_UNBIND)
//    {
//        if(s_product_id == NULL || s_mac == NULL)
//        {
//            app_log("param error!");
//            return;
//        }
//
//        app_log("subdevice is unbind, product id:%s, mac: %s", s_product_id, s_mac);
//    }else if(type == MQTT_CMD_GATEWAY_UNBIND)
//    {
//        app_log("gateway is unbind!!! subdevice can't send data");
//    }else if(type == MQTT_CMD_GATEWAY_BIND)
//    {
//        app_log("gateway is bind!!! subdevice can send data");
//    }
//
//    return;
}

#endif
