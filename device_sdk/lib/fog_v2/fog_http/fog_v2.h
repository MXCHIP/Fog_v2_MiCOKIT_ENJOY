#ifndef __FOG_V2_H_
#define __FOG_V2_H_

#include "fog_http_client.h"
#include "fog_v2_include.h"

/*
 *     设备端fogcloud库版本
 *     版本号由3个数字组成：r.x.y
 *     r：目前发布的内核主版本。
 *     x：偶数表示稳定版本；奇数表示开发中版本。
 *     y：错误修补的次数。
 */
#define FOG_V2_LIB_VERSION_MAJOR        (2)
#define FOG_V2_LIB_VERSION_MINOR        (6)
#define FOG_V2_LIB_VERSION_REVISION     (0)


#define WAIT_HTTP_RES_MAX_TIME              (7*1000) //http等待返回最大超时时间为7s
#define HTTP_MAX_RETRY_TIMES                (4)      //最大重试次数

#define FOG_HTTP_SUCCESS                    (0)
#define FOG_HTTP_TOKEN_EXPIRED              (1200)    //token过期
#define FOG_HTTP_TOKEN_ERROR                (1201)    //token错误
#define FOG_HTTP_TOKEN_INVAILD              (1202)    //无效的token

#define FOG_HTTP_PRODUCTI_ID_ERROR          (27020)   //产品ID错误，找不到对应产品
#define FOG_HTTP_PRODUCTI_ID_NOT_SUB        (27021)   //该设备不是子设备
#define FOG_HTTP_PRODUCTI_ID_NOT_GATEWAY    (27022)   //该设备不是父设备

#define FOG_HTTP_DEVICE_ID_ERROR            (27030)   //device id错误，找不到对应设备/子设备
#define FOG_HTTP_PASSSWORD_ERROR            (27032)   //密码错误
#define FOG_HTTP_DEVICE_HAVE_SUPER_USER     (27061)   //设备已经拥有超级用户
#define FOG_HTTP_OTA_NO_UPDATE              (25001)   //没有新的固件版本

//激活
#define FOG_V2_ACTIVATE_METHOD              HTTP_POST
#define FOG_V2_ACTIVATE_URI                 ("/device/activate/")

//获取授权
#define FOG_V2_GET_TOKEN_METHOD             HTTP_POST
#define FOG_V2_GET_TOKEN_URI                ("/device/token-auth/")

//检查超级用户
#define FOG_V2_CHECK_SUPERUSER_METHOD       HTTP_GET
#define FOG_V2_CHECK_SUPERUSER_URI          ("/device/checkdevicesuperuser/")

//回收授权 解绑
#define FOG_V2_RECOVERY_METHOD              HTTP_POST
#define FOG_V2_RECOVERY_URI                 ("/device/recoverydevicegrant/")

//状态同步
#define FOG_V2_SYNC_STATUS_METHOD           HTTP_POST
#define FOG_V2_SYNC_STATUS_URI              ("/device/syncstatus/")

//获取设备验证码  绑定用
#define FOG_V2_GENERATE_VERCODE_METHOD      HTTP_POST
#define FOG_V2_GENERATE_VERCODE_URI         ("/device/generatedevicevercode/")

//获取服务器时间
#define FOG_V2_GET_SERVER_TIME              HTTP_GET
#define FOG_V2_GET_SERVER_TIME_URI          ("/servertime/")

//发送数据
#define FOG_V2_SEND_EVENT_METHOD            HTTP_POST
#define FOG_V2_SEND_EVENT_URI               ("/device/sendeventadv/")

//OTA检查
#define FOG_V2_OTA_UP_DATA_CHECK            HTTP_POST
#define FOG_V2_OTA_UP_DATA_URI              ("/ota/updatecheck/")

//OTA上报
#define FOG_V2_OTA_UPLOAD_LOG               HTTP_POST
#define FOG_V2_OTA_UPLOAD_URI               ("/ota/otauploadlog/")

//发送数据相关发送设置
#define FOG_V2_SEND_EVENT_RULES_PUBLISH    ((0x01) << 0)   //是否向设备的topic去publish数据
#define FOG_V2_SEND_EVENT_RULES_DATEBASE   ((0x01) << 1)   //是否将此次的payload数据存入数据库
#define FOG_V2_SEND_EVENT_RULES_PRODUCT    ((0x01) << 2)   //是否向设备对应产品的topic去publish数据(数据推送给厂商)

#define DEVICE_ID_MIN_LEN                   (10)
#define DEVICE_TOKEN_MIN_LEN                (20)
#define DEVICE_MAC_LEN                      (12)

#define FOG_V2_PAYLOAD_LEN_MAX              (2048)

#define FOG_V2_SUB_DEVICE_MAX_NUM           (8) //子设备的最大支持数目
#define FOG_V2_SUB_DEVICE_MQTT_TOPIC_LEN    (64)
#define FOG_V2_SUB_DEVICE_DES_PASSWORD      (0xAAAAAAAA) //密码 上电之后读取该值,如果不是该值,需要清空子设备参数区

#define FOG_V2_DEVICEID_LEN                 (36)
#define FOG_V2_PRODUCT_ID_LEN               (36)

#define FOG_V2_SUBDEVICEID_LEN              (36)
#define FOG_V2_SUBPRODUCT_ID_LEN            (36)

typedef struct _FOG_SUB_DES_S
{
    bool s_is_activate;      //是否已激活子设备
    mico_queue_t queue;      //子设备自身的接收消息缓冲队列

    char *s_mqtt_topic_cmd;  //cmd地址(设备回收时记得释放)
    char *s_mqtt_topic_commands; //command地址(设备回收时记得释放)

    char s_device_mac[16];   //MAC地址

    char s_product_id[40];   //产品ID
    char s_device_id[40];    //设备ID
}FOG_SUB_DES_S;

typedef struct _FOG_DES_S
{
    bool is_activate;           //是否已激活设备
	bool is_hava_superuser;     //是否拥有超级用户
	bool is_recovery;           //是否需要回收授权

    char fog_v2_lib_version[16];//软件库版本号  mdns通知用
    char devicepw[16];          //设备密码
    char mico_version[16];      //MICO版本号
    char mxchip_sn[16];         //庆科模块sn，预留
    char device_mac[16];        //MAC地址
    char module_type[32];       //模块型号
    char firmware[32];          //软件版本
    char vercode[64];           //向云端请求得到的vercode
    char product_id[64];        //产品ID
    char device_id[64];         //设备ID
    char device_token[256];     //token

#if (FOG_V2_USER_FLASH_PARAM == 1)
    char user_app_data[FOG_V2_USER_FLASH_PARAM_LEN];    //user data
#endif

#if (FOG_V2_USE_SUB_DEVICE == 1)
    mico_mutex_t sub_des_mutex;
    uint32_t sub_des_password;
    FOG_SUB_DES_S sub_des[FOG_V2_SUB_DEVICE_MAX_NUM];   //子设备描述
#endif
}FOG_DES_S;


extern void user_free_json_obj(json_object **obj);
extern bool get_wifi_status(void);
extern FOG_DES_S *get_fog_des_g(void);
extern void fog_des_clean(void);
extern void fog_set_device_recovery_flag(void);
extern void set_https_connect_status(bool status);
extern bool get_https_connect_status(void);
extern void set_mqtt_connect_status(bool status);
extern void fog_WifiStatusHandler(WiFiEvent status, void* const inContext);
extern OSStatus fog_v2_push_http_req_mutex(bool is_jwt, FOG_HTTP_METHOD method, char *request_uri, char *host_name, char *http_body, FOG_HTTP_RESPONSE_SETTING_S *user_http_resopnse);

extern bool fog_v2_is_have_superuser(void);
extern bool fog_v2_is_https_connect(void);
extern bool fog_v2_is_mqtt_connect(void);

extern OSStatus check_http_body_code(int32_t code);
extern OSStatus process_response_body(const char *http_body, int32_t *code);
extern OSStatus process_response_body_string(const char *http_body, int32_t *code, const char *user_key, char *value, uint32_t value_len);
extern OSStatus process_response_body_bool(const char *http_body, int32_t *code, const char *user_key, bool *user_key_data);

extern OSStatus fog_v2_device_generate_device_vercode(void);
extern OSStatus fog_v2_ota_check(char *resoponse_body, int32_t resoponse_body_len, bool *need_update);
extern OSStatus fog_v2_ota_upload_log(void);


//用户调用接口
extern OSStatus init_fog_v2_service(void);     //初始化fog服务
extern OSStatus start_fog_v2_service(void);    //开启fog的服务
extern OSStatus fog_v2_device_get_server_time(char *server_time_p, uint32_t recv_len); //获取时间
extern OSStatus fog_v2_device_send_event(const char *payload, uint32_t flag); //往云端发送数据
extern bool fog_v2_set_device_recovery_flag(void);

#endif

