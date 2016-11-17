#ifndef __FOG_V2_CONFIG_H_
#define __FOG_V2_CONFIG_H_

#define FOG_ENABLE  (1)
#define FOG_DISABLE (0)

//选择你的硬件型号
#define EMW_3165                            FOG_DISABLE
#define EMW_3166                            FOG_DISABLE
#define EMW_3239                            FOG_ENABLE
#define EMW_3031                            FOG_DISABLE
#define NUCLEO_F411                         FOG_DISABLE

#if (EMW_3165 == FOG_ENABLE)
    #define FOG_V2_PRODUCT_ID               ("3c800be4-8612-11e6-9d95-00163e103941")    //云端建立产品得到的 产品ID
    #define FOG_V2_REPORT_VER               ("FOG_V2_EMW3165@")     //固件主版本号
    #define FOG_V2_REPORT_VER_NUM           ("001")                 //固件次版本号
    #define FOG_V2_MODULE_TYPE              ("EMW3165")             //模块型号
#elif (EMW_3166 == FOG_ENABLE)
    #define FOG_V2_PRODUCT_ID               ("5719ac44-8612-11e6-9d95-00163e103941")    //云端建立产品得到的 产品ID
    #define FOG_V2_REPORT_VER               ("FOG_V2_EMW3166@")     //固件主版本号
    #define FOG_V2_REPORT_VER_NUM           ("001")                 //固件次版本号
    #define FOG_V2_MODULE_TYPE              ("EMW3166")             //模块型号
#elif(EMW_3239 == FOG_ENABLE)
    #define FOG_V2_PRODUCT_ID               ("712d8ee8-8612-11e6-9d95-00163e103941")    //云端建立产品得到的 产品ID
    #define FOG_V2_REPORT_VER               ("FOG_V2_EMW3239@")     //固件主版本号
    #define FOG_V2_REPORT_VER_NUM           ("001")                 //固件次版本号
    #define FOG_V2_MODULE_TYPE              ("EMW3239")             //模块型号
#elif (EMW_3031 == FOG_ENABLE)
    #define FOG_V2_PRODUCT_ID               ("b4754452-8612-11e6-9d95-00163e103941")    //云端建立产品得到的 产品ID
    #define FOG_V2_REPORT_VER               ("FOG_V2_EMW3031@")     //固件主版本号
    #define FOG_V2_REPORT_VER_NUM           ("001")                 //固件次版本号
    #define FOG_V2_MODULE_TYPE              ("EMW3031")             //模块型号
#elif (NUCLEO_F411 == FOG_ENABLE)
    #define FOG_V2_PRODUCT_ID               ("bd538076-87dd-11e6-9d95-00163e103941")    //云端建立产品得到的 产品ID
    #define FOG_V2_REPORT_VER               ("FOG_V2_NUCLEO_F411@") //固件主版本号
    #define FOG_V2_REPORT_VER_NUM           ("001")                 //固件次版本号
    #define FOG_V2_MODULE_TYPE              ("EMW1062")             //模块型号
#endif

#define FOG_V2_DEVICE_SN                ("MXCHIP")                  //芯片SN 默认为MXCHIP

#define FOG_V2_HTTP_DOMAIN_NAME         ("v2.fogcloud.io")          //HTTP服务器地址
#define FOG_V2_HTTP_PORT_SSL            (443)   //fog v2 http SSL端口
#define HTTP_REQ_LOG                    (0)     //1:enable 0:disable

#define FOG_V2_MQTT_DOMAIN_NAME         ("mqtt.fogcloud.io")  //MQTT服务器地址
#define FOG_V2_MQTT_PORT_SLL            (8443)  //fog v2 MQTT SSL端口
#define FOG_V2_MQTT_PORT_NOSLL          (1883)  //fog v2 MQTT 非SSL端口
#define MQTT_CLIENT_SSL_ENABLE          (0)      //1:enable 0:disable (EMW3165的ROM较小 建议设置为0)
#define FOG_MQTT_DEBUG                  (1)      //MQTT打印信息开关

#define FOG_BONJOUR_SERVICE_NAME        ("_easylink._tcp.local.")  //MDNS service name
#define FOG_BONJOUR_SERVICE_TTL         (2)     //bonjour的ttl时间

#define FOG_V2_TCP_SERVER_PORT          (8002)   //APP和设备的本地通信端口

#define FOG_V2_OTA_ENABLE               (1)      //1:enable 0:disable  打开后在fog初始化中会检查当前版本,如果有新的版本则自动更新然后重启

#define FOG_V2_USE_SUB_DEVICE           (1)      //1:enable 0:disable  是否使用子设备接口,只有EMW3239才会使用到

#endif

