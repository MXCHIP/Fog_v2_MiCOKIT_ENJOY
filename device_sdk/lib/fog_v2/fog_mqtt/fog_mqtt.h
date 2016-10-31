#ifndef __FOG_MQTT_H_
#define __FOG_MQTT_H_

#include "mico.h"
#include "MQTTClient.h"

#define MQTT_CLIENT_KEEPALIVE       (30)
#define MQTT_CMD_TIMEOUT            (5000)  // 5s
#define MQTT_YIELD_TMIE             (5000)  // 5s

#define MAX_MQTT_TOPIC_SIZE         (512)
#define MAX_MQTT_DATA_SIZE          (2048)
#define MAX_MQTT_RECV_QUEUE_SIZE    (3)
#define MAX_MQTT_SEND_QUEUE_SIZE    (3)

#define MQTT_SUBSCRIBE_RETRY        (3)  //MQTT订阅失败重试次数

typedef struct _mqtt_context_t
{
    mico_queue_t mqtt_msg_recv_queue;
} mqtt_context_t;

typedef struct _mqtt_recv_msg_t
{
    char topic[MAX_MQTT_TOPIC_SIZE];
    char qos;
    char retained;

    uint8_t data[MAX_MQTT_DATA_SIZE];
    uint32_t datalen;
} mqtt_recv_msg_t, *p_mqtt_recv_msg_t, mqtt_send_msg_t, *p_mqtt_send_msg_t;

//订阅频道的设置
typedef struct _mqtt_sub_topic_setting
{
    char topic[MAX_MQTT_TOPIC_SIZE];
    enum QoS qos;
    messageHandler messageHandler;
    mico_mutex_t mutex;
} mqtt_sub_topic_setting;

//订阅频道的设置
typedef struct _mqtt_sub_topic
{
    bool is_success;
    char *topic;
    enum QoS qos;
    messageHandler messageHandler;
    mico_mutex_t mutex;
    mico_semaphore_t sub_sem;
    mico_semaphore_t finish_sem;
} mqtt_sub_topic;

//取消订阅频道的设置
typedef struct _mqtt_unsub_topic
{
    bool is_success;
    char *topic;
    mico_mutex_t mutex;
    mico_semaphore_t unsub_sem;
    mico_semaphore_t finish_sem;
} mqtt_unsub_topic;

extern void init_fog_mqtt_service( void );
extern OSStatus fog_v2_device_recv_command( char *payload, uint32_t payload_len, uint32_t timeout );


//----------子设备API----------
//增加topic
extern OSStatus add_mqtt_topic_command(char* topic);
extern OSStatus add_mqtt_topic_cmd(char* topic);
//删除topic
extern OSStatus remove_mqtt_topic( char* topic );
//----------子设备API----------
#endif

