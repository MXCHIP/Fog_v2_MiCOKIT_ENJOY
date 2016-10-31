#ifndef __FOG_V2_CONFIG_H_
#define __FOG_V2_CONFIG_H_

#define FOG_ENABLE  (1)
#define FOG_DISABLE (0)

//选择你的模块型号
#define NUCLEO_F411                     FOG_ENABLE

#if (NUCLEO_F411 == FOG_ENABLE)
#define FOG_V2_PRODUCT_ID               ("bd538076-87dd-11e6-9d95-00163e103941")    //云端建立产品得到的 产品ID
#define FOG_V2_REPORT_VER               ("FOG_V2_NUCLEO_F411@")     //固件主版本号
#define FOG_V2_REPORT_VER_NUM           ("001")                     //固件次版本号
#define FOG_V2_MODULE_TYPE              ("EMW3165")                 //由于云端不支持NUCLEO型号,所以用EMW3165临时代替
#endif

#define FOG_V2_DEVICE_SN                ("MXCHIP")              //芯片SN 默认为MXCHIP
#define FOG_V2_HTTP_DOMAIN_NAME         ("v2.fogcloud.io")      //HTTP服务器地址
#define FOG_V2_HTTP_PORT_SSL            (443)       //fog v2 http SSL端口

#define HTTP_REQ_LOG                    (0)     //1:enable 0:disable

#define FOG_V2_MQTT_DOMAIN_NAME         ("mqtt.fogcloud.io")  //MQTT服务器地址
#define FOG_V2_MQTT_PORT_SLL            (8443)  //fog v2 MQTT SSL端口
#define FOG_V2_MQTT_PORT_NOSLL          (1883)  //fog v2 MQTT 非SSL端口
#define MQTT_CLIENT_SSL_ENABLE          (0)     //1:enable 0:disable
#define FOG_MQTT_DEBUG                  (1)     //MQTT打印信息开关

#define FOG_BONJOUR_SERVICE_NAME        ("_easylink._tcp.local.")  //MDNS service name
#define FOG_BONJOUR_SERVICE_TTL         (2)     //bonjour的ttl时间

#define FOG_V2_TCP_SERVER_PORT          (8002)   //APP和设备的本地通信端口

#define FOG_V2_OTA_ENABLE               (1)      //1:enable 0:disable  打开后在fog初始化中会检查当前版本,如果有新的版本则自动更新然后重启

#define FOG_V2_USE_SUB_DEVICE           (0)      //1:enable 0:disable  是否使用子设备接口,只有EMW3239才会使用到

#include "fog_v2_include.h"
#endif

