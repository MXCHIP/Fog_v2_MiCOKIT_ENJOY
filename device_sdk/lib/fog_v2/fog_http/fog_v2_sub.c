#include "fog_v2_include.h"
#include "fog_v2_sub.h"
#include "mico.h"
#include "fog_v2_user_notification.h"

#define app_log(M, ...)                     custom_log("FOG_V2_SUB", M, ##__VA_ARGS__)

#ifndef FOG_V2_USE_SUB_DEVICE
    #error "FOG_V2_USE_SUB_DEVICE is not define"
#endif

#if (FOG_V2_USE_SUB_DEVICE == 1)

OSStatus fog_v2_subdevice_des_init(void);

static OSStatus fog_v2_subdevice_register(const char *product_id, const char *mac);
static OSStatus fog_v2_subdevice_unregister(const char *s_product_id, const char *s_mac);
static OSStatus fog_v2_subdevice_attach(const char *s_product_id, const char *s_mac);
static OSStatus fog_v2_subdevice_detach(const char *s_product_id, const char *s_mac);
static OSStatus fog_v2_subdevice_send_event(const char *payload, const char *s_product_id, const char *s_mac, uint32_t type);

//static OSStatus add_mqtt_topic_cmd_by_mac(const char *s_product_id, const char *mac);
OSStatus remove_mqtt_topic_by_mac(const char *s_product_id, const char *mac);
OSStatus add_mqtt_topic_by_mac(const char *s_product_id, const char *mac);
static OSStatus add_mqtt_topic_command_by_mac(const char *s_product_id, const char *s_mac);

OSStatus fog_v2_subdevice_add_timeout(const char *s_product_id);

OSStatus fog_v2_add_subdevice( const char *s_product_id, const char *s_mac, bool auto_set_online);
OSStatus fog_v2_remove_subdevice( const char *s_product_id, const char *s_mac );
OSStatus fog_v2_set_subdevice_status(const char *s_product_id, const char *s_mac, bool online);
OSStatus fog_v2_subdevice_send(const char *s_product_id, const char *s_mac, const char *payload, uint32_t flag);
OSStatus fog_v2_subdevice_recv(const char *s_product_id, const char *s_mac, char *payload, uint32_t payload_len, uint32_t timeout);
OSStatus fog_v2_subdeice_get_list(char *http_response, uint32_t recv_len, bool *get_http_response_success);

//bind monitor相关内容
void start_gateway_bind_monitor(void);
bool is_subdevice_cmd_queue_init(void);//队列是否已经初始化
OSStatus push_cmd_to_subdevice_queue(SUBDEVICE_CMD_TYPE type, const char *device_id);//往队列中插入一条数据

static mico_queue_t sub_device_cmd_queue = NULL; //接收子设备cmd消息的队列

//子设备设备注册
static OSStatus fog_v2_subdevice_register(const char *s_product_id, const char *s_mac)
{
    OSStatus err = kGeneralErr;
    const char *sub_device_register_body = "{\"productid\":\"%s\",\"mac\":\"%s\",\"extend\":\"%s\"}";
    char http_body[256] = {0};
    char device_id_temp[64] = {0};
    int32_t code = -1;
    uint32_t index = 0;
    FOG_HTTP_RESPONSE_SETTING_S user_http_res;
    uint32_t usable_index = 0;
    uint8_t retry = 0;

    memset(&user_http_res, 0, sizeof(user_http_res));

    if ( fog_v2_is_have_superuser( ) == false )
    {
        app_log("[ERROR]gateway don't have superuser!");
        return kGeneralErr;
    }

    if(get_sub_device_queue_index_by_mac(&index, s_product_id, s_mac) == true)
    {
        app_log("this device is already register!!! index = %ld", index);
        return kNoErr;
    }

    if(get_sub_device_queue_usable_index(&usable_index) == false)
    {
        app_log("can't find usable index");
        return kGeneralErr;
    }else
    {
        app_log("usable index = %ld", usable_index);
    }

start_subdevice_register:
    while(get_wifi_status() == false)
    {
        app_log("https disconnect, fog_v2_subdevice_register is waitting...");
        mico_thread_msleep(200);
        err = kGeneralErr;
        goto exit;
    }

    sprintf(http_body, sub_device_register_body, s_product_id, s_mac, "123");

    app_log("=====> sub_device_register send ======>");

    err = fog_v2_push_http_req_mutex(true, FOG_V2_SUB_REGISTER_METHOD, FOG_V2_SUB_REGISTER_URI, FOG_V2_HTTP_DOMAIN_NAME, http_body, &user_http_res);
    require_noerr( err, exit );

    //处理返回结果
    err = process_response_body_string(user_http_res.fog_response_body, &code, "deviceid", device_id_temp, sizeof(device_id_temp));
    require_noerr( err, exit );

    err = check_http_body_code(code);   //如果是token过期是错误问题，函数内部会处理完成之后再返回
    require_noerr( err, exit );

    if(sub_device_queue_get(s_product_id, s_mac, device_id_temp) == false)
    {
        app_log("sub_device_queue_get() error!!!");

        if(user_http_res.fog_response_body != NULL) //释放资源
        {
            free(user_http_res.fog_response_body);
            user_http_res.fog_response_body = NULL;
        }

        return kGeneralErr;
    }

    app_log("register success, sub_device_id: %s", device_id_temp);
    app_log("<===== sub_device_register success <======");

 exit:
     if(user_http_res.fog_response_body != NULL) //释放资源
     {
         free(user_http_res.fog_response_body);
         user_http_res.fog_response_body = NULL;
     }


    if(err != kNoErr)
    {
        if ( (code == FOG_HTTP_PRODUCTI_ID_ERROR) || (code == FOG_HTTP_PRODUCTI_ID_NOT_SUB) || (code == FOG_HTTP_PRODUCTI_ID_NOT_GATEWAY))
        {
            app_log("subdevice product id is error! code = %ld", code);
            app_log("<===== sub_device_register error <======");
            return err;
        }

        app_log("activate error, send_status:%d, status_code:%ld", user_http_res.send_status, user_http_res.status_code);
        app_log("<===== sub_device_register error <======");

        if ( (HTTP_CONNECT_ERROR == user_http_res.send_status) || (HTTP_RESPONSE_FAILURE == user_http_res.send_status))
        {
            mico_rtos_thread_msleep( 200 );

            retry ++;
            if(retry >= HTTP_MAX_RETRY_TIMES)
            {
                return kGeneralErr;
            }

            goto start_subdevice_register;
        }
    }

    return err;
}


//子设备注销
static OSStatus fog_v2_subdevice_unregister(const char *s_product_id, const char *s_mac)
{
    OSStatus err = kGeneralErr;
    const char *sub_device_unregister_body = "{\"deviceid\":\"%s\"}";
    char http_body[256] = {0};
    int32_t code = -1;
    FOG_HTTP_RESPONSE_SETTING_S user_http_res;
    char *subdevice_id = NULL;
    uint32_t index = 0;
    uint8_t retry = 0;

    memset(&user_http_res, 0, sizeof(user_http_res));

    if ( get_sub_device_queue_index_by_mac( &index, s_product_id, s_mac ) == false )
    {
        app_log("mac is error");
        return kGeneralErr;
    }

    if((subdevice_id = get_sub_device_id_by_index(index)) == NULL)
    {
        app_log("subdevice_id is error");
        return kGeneralErr;
    }

start_subdevice_unregister:
    while(get_wifi_status() == false)
    {
        app_log("https disconnect, fog_v2_subdevice_unregister is waitting...");
        mico_thread_msleep(200);
        err = kGeneralErr;
        goto exit;
    }

    sprintf(http_body, sub_device_unregister_body, subdevice_id);

    app_log("=====> subdevice unregister send ======>");

    err = fog_v2_push_http_req_mutex(true, FOG_V2_SUB_UNREGISTER_METHOD, FOG_V2_SUB_UNREGISTER_URI, FOG_V2_HTTP_DOMAIN_NAME, http_body, &user_http_res);
    require_noerr( err, exit );

    //处理返回结果
    err = process_response_body(user_http_res.fog_response_body, &code);
    require_noerr( err, exit );

    err = check_http_body_code(code);   //如果是token过期是错误问题，函数内部会处理完成之后再返回
    if(err != kNoErr)
    {
        if(code == FOG_HTTP_DEVICE_ID_ERROR) //云端可能不存在这个device id,处理这种情况
        {
            err = kNoErr;
            app_log("[NOTICE]code = 27030!");
            push_cmd_to_subdevice_queue(MQTT_CMD_SUB_UNBIND, subdevice_id);//发送消息给队列
        }
    }

    app_log("subdevice unregister success, device_id:%s", subdevice_id);
    app_log("<===== subdevice unregister success <======");

 exit:
    if(user_http_res.fog_response_body != NULL) //释放资源
    {
        free(user_http_res.fog_response_body);
        user_http_res.fog_response_body = NULL;
    }

    if(err != kNoErr)
    {
        if ( (code == FOG_HTTP_PRODUCTI_ID_ERROR) || (code == FOG_HTTP_PRODUCTI_ID_NOT_SUB) || (code == FOG_HTTP_PRODUCTI_ID_NOT_GATEWAY))
        {
            app_log("subdevice product id is error! code = %ld", code);
            app_log("<===== subdevice unregister error <======");
            return err;
        }

        app_log("unregister error, send_status:%d, status_code:%ld", user_http_res.send_status, user_http_res.status_code);
        app_log("<===== subdevice unregister error <======");

        if ( (HTTP_CONNECT_ERROR == user_http_res.send_status) || (HTTP_RESPONSE_FAILURE == user_http_res.send_status))
        {
            mico_thread_msleep( 200 );

            retry ++;
            if(retry >= HTTP_MAX_RETRY_TIMES)
            {
                return kGeneralErr;
            }

            goto start_subdevice_unregister;
        }
    }

    return err;
}

//子设备连接
static OSStatus fog_v2_subdevice_attach(const char *s_product_id, const char *s_mac)
{
    OSStatus err = kGeneralErr;
    const char *sub_device_attach_body = "{\"deviceid\":\"%s\"}";
    char http_body[256] = {0};
    int32_t code = -1;
    FOG_HTTP_RESPONSE_SETTING_S user_http_res;
    uint32_t index = 0;
    char *subdevice_id = NULL;
    uint8_t retry = 0;

    memset(&user_http_res, 0, sizeof(user_http_res));

    if(get_sub_device_queue_index_by_mac(&index, s_product_id, s_mac) == false)
    {
        app_log("mac is error");
        return kGeneralErr;
    }

    if((subdevice_id = get_sub_device_id_by_index(index)) == NULL)
    {
        app_log("subdevice_id is error");
        return kGeneralErr;
    }

start_subdevice_attach:
    while(get_wifi_status() == false)
    {
        app_log("https disconnect, fog_v2_subdeice_attach is waitting...");
        mico_thread_msleep(200);
        err = kGeneralErr;
        goto exit;
    }

    sprintf(http_body, sub_device_attach_body, subdevice_id);

    app_log("=====> subdevice attach send ======>");

    err = fog_v2_push_http_req_mutex(true, FOG_V2_SUB_ATTACH_METHOD, FOG_V2_SUB_ATTACH_URI, FOG_V2_HTTP_DOMAIN_NAME, http_body, &user_http_res);
    require_noerr( err, exit );

    //处理返回结果
    err = process_response_body(user_http_res.fog_response_body, &code);
    require_noerr( err, exit );

    err = check_http_body_code(code);   //如果是token过期是错误问题，函数内部会处理完成之后再返回
    if(err != kNoErr)
    {
        if(code == FOG_HTTP_DEVICE_ID_ERROR) //云端可能不存在这个device id,处理这种情况,连接出现异常
        {
            app_log("[ATTACH ERROR]code = 27030!");
            app_log("<===== subdevice attach error <======");

            push_cmd_to_subdevice_queue(MQTT_CMD_SUB_UNBIND, subdevice_id);//发送消息给队列

            if(user_http_res.fog_response_body != NULL) //释放资源
            {
                free(user_http_res.fog_response_body);
                user_http_res.fog_response_body = NULL;
            }

            return kGeneralErr;
        }else if ( (code == FOG_HTTP_PRODUCTI_ID_ERROR) || (code == FOG_HTTP_PRODUCTI_ID_NOT_SUB) || (code == FOG_HTTP_PRODUCTI_ID_NOT_GATEWAY))
        {
            app_log("subdevice product id is error! code = %ld", code);
            app_log("<===== subdevice attach error <======");

            if(user_http_res.fog_response_body != NULL) //释放资源
            {
                free(user_http_res.fog_response_body);
                user_http_res.fog_response_body = NULL;
            }

            return kGeneralErr;
        }
    }

    app_log("subdevice attach success, sub device_id:%s", subdevice_id);
    app_log("<===== subdevice attach success <======");

 exit:
     if(user_http_res.fog_response_body != NULL) //释放资源
     {
         free(user_http_res.fog_response_body);
         user_http_res.fog_response_body = NULL;
     }

    if(err != kNoErr)
    {
        app_log("subdeice attach error, send_status:%d, status_code:%ld", user_http_res.send_status, user_http_res.status_code);
        app_log("<===== subdevice attach error <======");
        if ( (HTTP_CONNECT_ERROR == user_http_res.send_status) || (HTTP_RESPONSE_FAILURE == user_http_res.send_status))
        {
            mico_thread_msleep( 200 );

            retry ++;
            if(retry >= HTTP_MAX_RETRY_TIMES)
            {
                return kGeneralErr;
            }

            goto start_subdevice_attach;
        }
    }

    return err;
}


//子设备断开连接
static OSStatus fog_v2_subdevice_detach(const char *s_product_id, const char *s_mac)
{
    OSStatus err = kGeneralErr;
    const char *sub_device_detach_body = "{\"deviceid\":\"%s\"}";
    char http_body[256] = {0};
    int32_t code = -1;
    FOG_HTTP_RESPONSE_SETTING_S user_http_res;
    char *subdevice_id = NULL;
    uint32_t index = 0;
    uint8_t retry = 0;

    memset(&user_http_res, 0, sizeof(user_http_res));

    if(get_sub_device_queue_index_by_mac(&index, s_product_id, s_mac) == false)
    {
        app_log("mac is error");
        return kGeneralErr;
    }

    if((subdevice_id = get_sub_device_id_by_index(index)) == NULL)
    {
        app_log("subdevice_id is error");
        return kGeneralErr;
    }

start_subdeice_detach:
    while(get_wifi_status() == false)
    {
        app_log("https disconnect, fog_v2_subdevice_detach is waitting...");
        mico_thread_msleep(200);
        err = kGeneralErr;
        goto exit;
    }

    sprintf(http_body, sub_device_detach_body, subdevice_id);

    app_log("=====> subdevice detach send ======>");

    err = fog_v2_push_http_req_mutex(true, FOG_V2_SUB_DETACH_METHOD, FOG_V2_SUB_DETACH_URI, FOG_V2_HTTP_DOMAIN_NAME, http_body, &user_http_res);
    require_noerr( err, exit );

    //处理返回结果
    err = process_response_body(user_http_res.fog_response_body, &code);
    require_noerr( err, exit );

    err = check_http_body_code(code);   //如果是token过期是错误问题，函数内部会处理完成之后再返回
    {
        if(code == FOG_HTTP_DEVICE_ID_ERROR) //云端可能不存在这个device id,处理这种情况,连接出现异常
        {
            app_log("[DETACH ERROR]code = 27030!");
            app_log("<===== subdevice detach error <======");

            push_cmd_to_subdevice_queue(MQTT_CMD_SUB_UNBIND, subdevice_id);//发送消息给队列

            if(user_http_res.fog_response_body != NULL) //释放资源
            {
                free(user_http_res.fog_response_body);
                user_http_res.fog_response_body = NULL;
            }

            return kGeneralErr;
        }else if ( (code == FOG_HTTP_PRODUCTI_ID_ERROR) || (code == FOG_HTTP_PRODUCTI_ID_NOT_SUB) || (code == FOG_HTTP_PRODUCTI_ID_NOT_GATEWAY))
        {
            app_log("subdevice product id is error! code = %ld", code);
            app_log("<===== subdevice detach error <======");

            if(user_http_res.fog_response_body != NULL) //释放资源
            {
                free(user_http_res.fog_response_body);
                user_http_res.fog_response_body = NULL;
            }

            return kGeneralErr;
        }
    }

    app_log("subdevice detach success, sub device_id:%s", subdevice_id);
    app_log("<===== subdevice detach success <======");

 exit:
     if(user_http_res.fog_response_body != NULL) //释放资源
     {
         free(user_http_res.fog_response_body);
         user_http_res.fog_response_body = NULL;
     }

    if(err != kNoErr)
    {
        app_log("subdeice deaach error, send_status:%d, status_code:%ld", user_http_res.send_status, user_http_res.status_code);
        app_log("<===== subdevice detach error <======");

        if ( (HTTP_CONNECT_ERROR == user_http_res.send_status) || (HTTP_RESPONSE_FAILURE == user_http_res.send_status))
        {
            mico_thread_msleep( 200 );

            retry ++;
            if(retry >= HTTP_MAX_RETRY_TIMES)
            {
                return kGeneralErr;
            }

            goto start_subdeice_detach;
        }
    }

    return err;
}

//子设备添加超时
OSStatus fog_v2_subdevice_add_timeout(const char *s_product_id)
{
    OSStatus err = kGeneralErr;
    const char *sub_device_add_timeout_body = "{\"productid\":\"%s\"}";
    char http_body[256] = {0};
    int32_t code = -1;
    FOG_HTTP_RESPONSE_SETTING_S user_http_res;
    uint8_t retry = 0;

    memset(&user_http_res, 0, sizeof(user_http_res));

start_add_timeout:
    while(get_wifi_status() == false)
    {
        app_log("https disconnect, fog_v2_subdevice_add_timeout is waitting...");
        mico_thread_msleep(200);
        err = kGeneralErr;
        goto exit;
    }

    sprintf(http_body, sub_device_add_timeout_body, s_product_id);

    app_log("=====> subdevice add timeout send ======>");

    err = fog_v2_push_http_req_mutex(true, FOG_V2_SUB_ADD_TIMEOUT_METHOD, FOG_V2_SUB_ADD_TIMEOUT_URI, FOG_V2_HTTP_DOMAIN_NAME, http_body, &user_http_res);
    require_noerr( err, exit );

    //处理返回结果
    err = process_response_body(user_http_res.fog_response_body, &code);
    require_noerr( err, exit );

    err = check_http_body_code(code);   //如果是token过期是错误问题，函数内部会处理完成之后再返回
    require_noerr( err, exit );

    app_log("subdevice send add timeout success, product id:%s", s_product_id);
    app_log("<===== subdevice add timeout success <======");

 exit:
     if(user_http_res.fog_response_body != NULL) //释放资源
     {
         free(user_http_res.fog_response_body);
         user_http_res.fog_response_body = NULL;
     }

    if(err != kNoErr)
    {
        app_log("subdeice add timeout error, send_status:%d, status_code:%ld", user_http_res.send_status, user_http_res.status_code);
        app_log("<===== subdevice add timeout error <======");
        if ( (HTTP_CONNECT_ERROR == user_http_res.send_status) || (HTTP_RESPONSE_FAILURE == user_http_res.send_status))
        {
            mico_thread_msleep( 200 );

            retry ++;
            if(retry >= HTTP_MAX_RETRY_TIMES)
            {
                return kGeneralErr;
            }

            goto start_add_timeout;
        }
    }

    return err;
}

//获取子设备列表
OSStatus fog_v2_subdeice_get_list(char *http_response, uint32_t recv_len, bool *get_http_response_success)
{
    OSStatus err = kGeneralErr;

    char http_body[256] = {0};
    int32_t code = -1;
    FOG_HTTP_RESPONSE_SETTING_S user_http_res;
    uint8_t retry = 0;

    memset(&user_http_res, 0, sizeof(user_http_res));

start_get_list:
    while(get_wifi_status() == false)
    {
        app_log("https disconnect, fog_v2_subdeice_get_list is waitting...");
        mico_thread_msleep(200);
        err = kGeneralErr;
        goto exit;
    }

    app_log("=====> subdevice get list send ======>");

    err = fog_v2_push_http_req_mutex(true, FOG_V2_SUB_GET_LIST_METHOD, FOG_V2_SUB_GET_LIST_URI, FOG_V2_HTTP_DOMAIN_NAME, http_body, &user_http_res);
    require_noerr( err, exit );

    //处理返回结果
    err = process_response_body(user_http_res.fog_response_body, &code);
    require_noerr( err, exit );

    err = check_http_body_code(code);   //如果是token过期是错误问题，函数内部会处理完成之后再返回
    require_noerr( err, exit );

    app_log("body:%s", user_http_res.fog_response_body);

    if(recv_len > strlen(user_http_res.fog_response_body))
    {
        memcpy(http_response, user_http_res.fog_response_body, strlen(user_http_res.fog_response_body));
        *get_http_response_success = true;
        app_log("[SUCCESS] copy subdevice list response!");
    }else
    {
        *get_http_response_success = false;
        app_log("[ERROR] recv_len size is small!");
    }

    app_log("<===== subdevice get list success <======");

 exit:
     if(user_http_res.fog_response_body != NULL) //释放资源
     {
         free(user_http_res.fog_response_body);
         user_http_res.fog_response_body = NULL;
     }

    if(err != kNoErr)
    {
        app_log("subdeice get list error, send_status:%d, status_code:%ld", user_http_res.send_status, user_http_res.status_code);
        app_log("<===== subdevice get list error <======");

        if ( (HTTP_CONNECT_ERROR == user_http_res.send_status) || (HTTP_RESPONSE_FAILURE == user_http_res.send_status))
        {
            mico_thread_msleep( 200 );

            retry ++;
            if(retry >= HTTP_MAX_RETRY_TIMES)
            {
                return kGeneralErr;
            }

            goto start_get_list;
        }
    }

    return err;
}


//子设备发送数据
static OSStatus fog_v2_subdevice_send_event(const char *payload, const char *s_product_id, const char *s_mac, uint32_t type)
{
    OSStatus err = kGeneralErr;
    int32_t code = -1;
    json_object *send_json_object = NULL;
    char *http_body = NULL;
    FOG_HTTP_RESPONSE_SETTING_S user_http_res;
    char *subdevice_id = NULL;
    uint32_t index = 0;
    uint8_t retry = 0;

    memset(&user_http_res, 0, sizeof(user_http_res));

    if(fog_v2_is_have_superuser() == false)
    {
        app_log("[ERROR]gateway don't have superuser!");
        return kGeneralErr;
    }

start_sub_send_event:
    while(get_wifi_status() == false)
    {
        app_log("https disconnect, fog_v2_subdevice_send_event is waitting...");
        mico_thread_msleep(200);
        err = kGeneralErr;
        goto exit;
    }

    if(get_sub_device_queue_index_by_mac(&index, s_product_id, s_mac) == false)
    {
        app_log("mac is error");
        return kGeneralErr;
    }

    if((subdevice_id = get_sub_device_id_by_index(index)) == NULL)
    {
        app_log("subdevice_id is error");
        return kGeneralErr;
    }

    send_json_object = json_object_new_object();
    require_string(send_json_object != NULL, exit, "json_object_new_object() error");

    json_object_object_add(send_json_object, "subdeviceid", json_object_new_string(subdevice_id));
    json_object_object_add(send_json_object, "flag", json_object_new_int(type));
    json_object_object_add(send_json_object, "format", json_object_new_string("json"));
    json_object_object_add(send_json_object, "payload", json_object_new_string(payload));
    http_body = (char *)json_object_to_json_string(send_json_object);
    require_action_string(http_body != NULL, exit, err = kGeneralErr, "json_object_to_json_string() is error");

    app_log("=====> subdevice send_event send ======>");

    err = fog_v2_push_http_req_mutex(true, FOG_V2_SUB_SENDEVENT_METHOD, FOG_V2_SUB_SENDEVENT_URI, FOG_V2_HTTP_DOMAIN_NAME, http_body, &user_http_res);
    require_noerr( err, exit );

    user_free_json_obj(&send_json_object);

    //处理返回结果
    err = process_response_body(user_http_res.fog_response_body, &code);
    require_noerr( err, exit );

    err = check_http_body_code(code);   //如果是token过期是错误问题，函数内部会处理完成之后再返回
    require_noerr( err, exit );

    app_log("<===== subdevice send_event success <======");

 exit:
     user_free_json_obj(&send_json_object);

     if(user_http_res.fog_response_body != NULL) //释放资源
     {
         free(user_http_res.fog_response_body);
         user_http_res.fog_response_body = NULL;
     }

    if(err != kNoErr)
    {
        app_log("subdeice send_event error, send_status:%d, status_code:%ld", user_http_res.send_status, user_http_res.status_code);
        app_log("<===== subdevice send_event error <======");
        if ( (HTTP_CONNECT_ERROR == user_http_res.send_status) || (HTTP_RESPONSE_FAILURE == user_http_res.send_status))
        {
            mico_thread_msleep( 200 );

            retry ++;
            if(retry >= HTTP_MAX_RETRY_TIMES)
            {
                return kGeneralErr;
            }

            goto start_sub_send_event;
        }
    }

    return err;
}


OSStatus add_mqtt_topic_by_mac(const char *s_product_id, const char *mac)
{
    OSStatus err = kGeneralErr;

    if(mac == NULL || s_product_id == NULL)
    {
        app_log("param error!");
        return kGeneralErr;
    }

    err = add_mqtt_topic_command_by_mac(s_product_id, mac);
    require_noerr(err, exit);

//    err = add_mqtt_topic_cmd_by_mac(s_product_id, mac);
//    require_noerr(err, exit);

    exit:
    return err;
}

static OSStatus add_mqtt_topic_command_by_mac(const char *s_product_id, const char *s_mac)
{
    uint32_t index = 0;
    char *commands_topic = NULL;
    OSStatus err = kGeneralErr;

    if(s_mac == NULL || s_product_id == NULL)
    {
        app_log("[ERROR]param error");
        return kGeneralErr;
    }

    if(get_sub_device_queue_index_by_mac(&index, s_product_id, s_mac) == false)
    {
        app_log("[ERROR]mac is error");
        return kGeneralErr;
    }

    if((commands_topic = get_sub_device_commands_topic_by_index(index)) == NULL)
    {
        app_log("[ERROR]commands_topic is NULL");
        return kGeneralErr;
    }

    err = add_mqtt_topic_command( commands_topic );
    if ( err != kNoErr )
    {
        app_log("command err = %d", err);
    }

    return err;
}

//static OSStatus add_mqtt_topic_cmd_by_mac(const char *s_product_id, const char *s_mac )
//{
//    uint32_t index = 0;
//    char *cmd_topic = NULL;
//    OSStatus err = kGeneralErr;
//
//    if ( s_mac == NULL )
//    {
//        app_log("[ERROR]s_mac is NULL");
//        return kGeneralErr;
//    }
//
//    if ( get_sub_device_queue_index_by_mac( &index, s_product_id, s_mac ) == false )
//    {
//        app_log("[ERROR]mac is error");
//        return kGeneralErr;
//    }
//
//    if ( (cmd_topic = get_sub_device_cmd_topic_by_index( index )) == NULL )
//    {
//        app_log("[ERROR]cmd_topic is NULL");
//        return kGeneralErr;
//    }
//
//    err = add_mqtt_topic_cmd( cmd_topic );
//    if ( err != kNoErr )
//    {
//        app_log("command err = %d", err);
//    }
//
//    return err;
//}

OSStatus remove_mqtt_topic_by_mac(const char *s_product_id, const char *s_mac)
{
    uint32_t index = 0;
//    char *cmd_topic = NULL;
    char *commands_topic = NULL;
    OSStatus err = kGeneralErr;

    if ( s_mac == NULL )
    {
        app_log("[ERROR]s_mac is NULL");
        return kGeneralErr;
    }

    if ( get_sub_device_queue_index_by_mac( &index, s_product_id, s_mac ) == false )
    {
        app_log("[ERROR]mac is error");
        return kGeneralErr;
    }

    if ( (commands_topic = get_sub_device_commands_topic_by_index( index )) == NULL )
    {
        app_log("[ERROR]commands_topic is NULL");
        return kGeneralErr;
    }

    err = remove_mqtt_topic( commands_topic );
    if ( err != kNoErr )
    {
        app_log("remove command err = %d, mac = %s", err, s_mac);
    }

//    if ( (cmd_topic = get_sub_device_cmd_topic_by_index( index )) == NULL )
//    {
//        app_log("[ERROR]cmd_topic is NULL");
//        return kGeneralErr;
//    }

//    err = remove_mqtt_topic( cmd_topic );
//    if ( err != kNoErr )
//    {
//        app_log("remove cmd err = %d, mac = %s", err, s_mac);
//    }

    return err;
}

//功能：添加一个子设备
//参数： s_product_id - 子设备产品ID
//参数： s_mac - 子设备MAC地址
//返回值：kNoErr为成功 其他值为失败
OSStatus fog_v2_add_subdevice( const char *s_product_id, const char *s_mac, bool set_auto_online)
{
    OSStatus err = kGeneralErr;

    require_action(get_fog_des_g() != NULL, exit_fun, err = kGeneralErr);

    require_action((s_product_id != NULL && s_mac != NULL), exit, err = kGeneralErr);

    //子设备注册
    err = fog_v2_subdevice_register( s_product_id, s_mac );//内部会申请子设备资源
    require_noerr( err, exit );

    //频道订阅
    err = add_mqtt_topic_by_mac( s_product_id, s_mac );
    require_noerr( err, exit );

    if(set_auto_online == true)
    {
        //设置子设备在线
        err = fog_v2_set_subdevice_status(s_product_id, s_mac, true);
        require_noerr( err, exit );
    }

    app_log("register, add mqtt topic, set online! product_id:%s, mac:%s", s_product_id, s_mac);

    exit:
    if(err != kNoErr)
    {
        fog_v2_remove_subdevice(s_product_id, s_mac);
        app_log("fog_v2_add_subdevice() failed, remove it now~");
    }

    exit_fun:
    return err;
}

//功能：删除一个子设备
//参数： s_product_id - 子设备产品ID
//参数： s_mac - 子设备MAC地址
//返回值：kNoErr为成功 其他值为失败
OSStatus fog_v2_remove_subdevice( const char *s_product_id, const char *s_mac )
{
    OSStatus err = kGeneralErr;

    require_action(get_fog_des_g() != NULL, exit, err = kGeneralErr);

    require_action((s_product_id != NULL && s_mac != NULL), exit, err = kGeneralErr);

    //设置子设备离线
    err = fog_v2_set_subdevice_status( s_product_id, s_mac, false );
    if(err != kNoErr)
    {
        app_log("set offline error, product id:%s, mac:%s", s_product_id, s_mac);
    }

    //子设备取消订阅topic
    err = remove_mqtt_topic_by_mac( s_product_id, s_mac );
    if(err != kNoErr)
    {
        app_log("remove topic error, product id:%s, mac:%s", s_product_id, s_mac);
    }

    //子设备注销
    err = fog_v2_subdevice_unregister( s_product_id, s_mac ); //内部会释放子设备资源
    if(err != kNoErr)
    {
        app_log("unregister error, product id:%s, mac:%s", s_product_id, s_mac);
    }

    if(sub_device_queue_put_by_mac(s_product_id, s_mac) == false) //释放资源
    {
        app_log("sub_device_queue_put_by_mac() error!!! product id:%s, mac:%s", s_product_id, s_mac);
    }

    app_log("unregister! remove mqtt topic! set offline! release subdevice!");

    exit:
    return err;
}

//功能：设置子设备在线离线状态
//参数： s_product_id - 子设备产品ID
//参数： s_mac - 子设备MAC地址
//参数：online - 子设备是否在线
//返回值：kNoErr为成功 其他值为失败
OSStatus fog_v2_set_subdevice_status(const char *s_product_id, const char *s_mac, bool online)
{
    OSStatus err = kGeneralErr;

    require_action(get_fog_des_g() != NULL, exit, err = kGeneralErr);

    require_action((s_product_id != NULL && s_mac != NULL), exit, err = kGeneralErr);

    if ( online == true )
    {
        //子设备连接
        err = fog_v2_subdevice_attach( s_product_id, s_mac );
        require_noerr( err, exit );
    } else
    {
        //子设备断开连接 detach
        err = fog_v2_subdevice_detach( s_product_id, s_mac );
        require_noerr( err, exit );
    }

    exit:
    return err;
}


//功能：子设备发送数据
//参数： s_product_id - 子设备产品ID
//参数： s_mac - 子设备MAC地址
//参数： flag - 发送方式
//下面两个宏定义组合,采用异或组合的方式
//FOG_V2_SEND_EVENT_RULES_PUBLISH  向设备的topic去publish数据
//FOG_V2_SEND_EVENT_RULES_DATEBASE 将此次的payload数据存入数据库
//返回值：kNoErr为成功 其他值为失败
OSStatus fog_v2_subdevice_send(const char *s_product_id, const char *s_mac, const char *payload, uint32_t flag)
{
    OSStatus err = kGeneralErr;

    require_action(get_fog_des_g() != NULL, exit, err = kGeneralErr);

    require_action((s_product_id != NULL && s_mac != NULL && payload != NULL), exit, err = kGeneralErr);

    err = fog_v2_subdevice_send_event(payload, s_product_id, s_mac, flag);
    require_noerr( err, exit );

    exit:
    return err;
}

//功能：子设备接收数据
//参数： s_product_id - 子设备产品ID
//参数： s_mac - 子设备MAC地址
//参数： payload - 接收数据缓冲区地址
//参数： payload_len - 接收数据缓冲区地址的长度
//参数： timeout - 接收数据的超时时间
//返回值：kNoErr-成功  kDeletedErr-该设备已被删除  kGeneralErr-超时
OSStatus fog_v2_subdevice_recv(const char *s_product_id, const char *s_mac, char *payload, uint32_t payload_len, uint32_t timeout)
{
    OSStatus err = kGeneralErr;
    uint32_t index = 0;
    SUBDEVICE_RECV_DATA_S *subdevice_recv_p = NULL;
    mico_queue_t *sub_device_queue_p = NULL;

    require_action(get_fog_des_g() != NULL, exit, err = kGeneralErr);

    require_action( (s_mac != NULL && s_product_id != NULL && payload != NULL && payload_len != 0), exit, err = kGeneralErr );

    if(get_sub_device_queue_index_by_mac(&index, s_product_id, s_mac) == false)
    {
        //app_log("get_sub_device_queue_index_by_mac error");
        return kDeletedErr;
    }

    sub_device_queue_p = get_sub_device_queue_addr_by_index(index);
    require(sub_device_queue_p != NULL, exit);

    err = mico_rtos_pop_from_queue(sub_device_queue_p, &subdevice_recv_p, timeout);
    if(err != kNoErr)
    {
        err = kGeneralErr;
        goto exit;
    }

    require_action_string( subdevice_recv_p != NULL, exit, err = kGeneralErr, "subdevice_recv_p is NULL");

    require_action_string( payload_len > subdevice_recv_p->data_len, exit, err = kGeneralErr, "payload_len is too short");

    memset(payload, 0, payload_len);
    memcpy(payload, subdevice_recv_p->data, subdevice_recv_p->data_len);

    err = kNoErr;
  exit:
    if ( subdevice_recv_p != NULL )
    {
        if ( subdevice_recv_p->data != NULL )
        {
            free( subdevice_recv_p->data );
            subdevice_recv_p->data = NULL;
        }
        free( subdevice_recv_p ); //释放本体
        subdevice_recv_p = NULL;
    }

    return err;
}




//-------------------------子设备监听线程相关内容------------------

//队列是否已经初始化
bool is_subdevice_cmd_queue_init(void)
{
    if(sub_device_cmd_queue == NULL)
        return false;
    else
        return true;
}


//往队列中插入一条数据
OSStatus push_cmd_to_subdevice_queue(SUBDEVICE_CMD_TYPE type, const char *device_id)
{
    SUBDEVICE_RECV_CMD_DATA_S cmd_msg;
    OSStatus err = kGeneralErr;

    if(is_subdevice_cmd_queue_init() == false)
    {
        app_log("is_subdevice_cmd_queue_init() return false!");
        return kGeneralErr;
    }

    if(MQTT_CMD_GATEWAY_UNBIND != type && MQTT_CMD_GATEWAY_BIND != type && MQTT_CMD_SUB_UNBIND != type)
    {
        app_log("type error");
        return false;
    }

    if(device_id == NULL)
    {
        app_log("device_id is NULL");
        return false;
    }

    if ( strlen( device_id ) >= sizeof(cmd_msg.device_id) )
    {
        app_log("device_id is too long");
        return false;
    }

    cmd_msg.cmd_type = type;
    strcpy(cmd_msg.device_id, device_id);

    err = mico_rtos_push_to_queue( &sub_device_cmd_queue, &cmd_msg, 100 );
    if ( kNoErr != err )
    {
        app_log("[error]push msg into sub_device_cmd_queue, err=%d", err);
    } else
    {
        app_log("push cmd to queue success!");
    }

    return err;
}


//监控函数
void gateway_bind_monitor(mico_thread_arg_t arg)
{
    OSStatus err = kGeneralErr;
    SUBDEVICE_RECV_CMD_DATA_S cmd_msg;
    uint32_t index = 0;
    char s_product_id[64] = {0};
    char s_mac[16] = {0};

    app_log("----------------gateway_bind_monitor thread start----------------");

    err = mico_rtos_init_queue( &sub_device_cmd_queue, "sub device cmd queue", sizeof(SUBDEVICE_RECV_CMD_DATA_S), FOG_V2_SUB_DEVICE_MAX_NUM );
    require_noerr( err, exit );

    while(1)
    {
        memset(&cmd_msg, 0, sizeof(cmd_msg));
        mico_rtos_pop_from_queue(&sub_device_cmd_queue, &cmd_msg, MICO_NEVER_TIMEOUT);

        if(cmd_msg.cmd_type == MQTT_CMD_GATEWAY_UNBIND)
        {
            app_log("cmd type:gateway unbind! deviceid:%s", cmd_msg.device_id);

            user_fog_v2_device_notification(MQTT_CMD_GATEWAY_UNBIND, NULL, NULL);
        }else if(cmd_msg.cmd_type == MQTT_CMD_GATEWAY_BIND)
        {
            app_log("cmd type:gateway bind! deviceid:%s", cmd_msg.device_id);

            user_fog_v2_device_notification(MQTT_CMD_GATEWAY_BIND, NULL, NULL);
        }else if(cmd_msg.cmd_type == MQTT_CMD_SUB_UNBIND)
        {
            app_log("cmd type:subdevice unbind! deviceid:%s", cmd_msg.device_id);

            if(get_sub_device_queue_index_by_deviceid(&index ,cmd_msg.device_id) == true)
            {
                if(get_sub_device_product_id_by_index(index) != NULL && get_sub_device_mac_by_index(index) != NULL)
                {
                    memset(s_product_id, 0, sizeof(s_product_id));
                    memset(s_mac, 0, sizeof(s_mac));

                    strcpy( s_product_id, (const char *) get_sub_device_product_id_by_index( index ) );
                    strcpy( s_mac, (const char *) get_sub_device_mac_by_index( index ) );

                    user_fog_v2_device_notification(MQTT_CMD_SUB_UNBIND, s_product_id, s_mac);

                    fog_v2_remove_subdevice((const char *)get_sub_device_product_id_by_index(index), (const char *)get_sub_device_mac_by_index(index));
                }
            }else
            {
                app_log("deviceid is not available, %s", cmd_msg.device_id);
            }
        }else
        {
            app_log("cmd type error!");
        }
    }

 exit:
    mico_rtos_delete_thread(NULL);
}


void start_gateway_bind_monitor(void)
{
    OSStatus err = kGeneralErr;

    /* Create a new thread */
    err = mico_rtos_create_thread( NULL, MICO_APPLICATION_PRIORITY, "gateway bind monitor", gateway_bind_monitor, 0x1000, (uint32_t) NULL );
    require_noerr_string( err, exit, "ERROR: Unable to start the send_event_test thread" );

 exit:
    return;
}

//-------------------------子设备监听线程相关内容------------------

#endif



