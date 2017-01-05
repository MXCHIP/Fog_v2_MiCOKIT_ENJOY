#include "mico.h"
#include "fog_mqtt.h"
#include "fog_process_mqtt_cmd.h"
#include "fog_v2_include.h"
#include "MQTTClient.h"

#ifndef FOG_MQTT_DEBUG
#error "FOG_MQTT_DEBUG is not define"
#endif

#if  (FOG_MQTT_DEBUG == 1)
#define app_log(M, ...)         custom_log("FOG_MQTT", M, ##__VA_ARGS__)
#else
#define app_log(M, ...)
#endif

#ifndef MQTT_CLIENT_SSL_ENABLE
#error "MQTT_CLIENT_SSL_ENABLE is not define"
#endif

#ifndef FOG_V2_MQTT_DOMAIN_NAME
#error "FOG_V2_MQTT_DOMAIN_NAME is not define"
#endif

#if (MQTT_CLIENT_SSL_ENABLE == 1)

#ifndef FOG_V2_MQTT_PORT_SLL
#error "FOG_V2_MQTT_PORT_SLL is not define"
#endif

#define MQTT_SERVER              FOG_V2_MQTT_DOMAIN_NAME
#define MQTT_SERVER_PORT         FOG_V2_MQTT_PORT_SLL
char* mqtt_server_ssl_cert_str =
"-----BEGIN CERTIFICATE-----\r\n\
MIIDIDCCAomgAwIBAgIENd70zzANBgkqhkiG9w0BAQUFADBOMQswCQYDVQQGEwJV\r\n\
UzEQMA4GA1UEChMHRXF1aWZheDEtMCsGA1UECxMkRXF1aWZheCBTZWN1cmUgQ2Vy\r\n\
dGlmaWNhdGUgQXV0aG9yaXR5MB4XDTk4MDgyMjE2NDE1MVoXDTE4MDgyMjE2NDE1\r\n\
MVowTjELMAkGA1UEBhMCVVMxEDAOBgNVBAoTB0VxdWlmYXgxLTArBgNVBAsTJEVx\r\n\
dWlmYXggU2VjdXJlIENlcnRpZmljYXRlIEF1dGhvcml0eTCBnzANBgkqhkiG9w0B\r\n\
AQEFAAOBjQAwgYkCgYEAwV2xWGcIYu6gmi0fCG2RFGiYCh7+2gRvE4RiIcPRfM6f\r\n\
BeC4AfBONOziipUEZKzxa1NfBbPLZ4C/QgKO/t0BCezhABRP/PvwDN1Dulsr4R+A\r\n\
cJkVV5MW8Q+XarfCaCMczE1ZMKxRHjuvK9buY0V7xdlfUNLjUA86iOe/FP3gx7kC\r\n\
AwEAAaOCAQkwggEFMHAGA1UdHwRpMGcwZaBjoGGkXzBdMQswCQYDVQQGEwJVUzEQ\r\n\
MA4GA1UEChMHRXF1aWZheDEtMCsGA1UECxMkRXF1aWZheCBTZWN1cmUgQ2VydGlm\r\n\
aWNhdGUgQXV0aG9yaXR5MQ0wCwYDVQQDEwRDUkwxMBoGA1UdEAQTMBGBDzIwMTgw\r\n\
ODIyMTY0MTUxWjALBgNVHQ8EBAMCAQYwHwYDVR0jBBgwFoAUSOZo+SvSspXXR9gj\r\n\
IBBPM5iQn9QwHQYDVR0OBBYEFEjmaPkr0rKV10fYIyAQTzOYkJ/UMAwGA1UdEwQF\r\n\
MAMBAf8wGgYJKoZIhvZ9B0EABA0wCxsFVjMuMGMDAgbAMA0GCSqGSIb3DQEBBQUA\r\n\
A4GBAFjOKer89961zgK5F7WF0bnj4JXMJTENAKaSbn+2kmOeUJXRmm/kEd5jhW6Y\r\n\
7qj/WsjTVbJmcVfewCHrPSqnI0kBBIZCe/zuf6IWUrVnZ9NA2zsmWLIodz2uFHdh\r\n\
1voqZiegDfqnc1zqcPGUIWVEX/r87yloqaKHee9570+sB3c4\r\n\
-----END CERTIFICATE-----";

#else  // ! MQTT_CLIENT_SSL_ENABLE

#ifndef FOG_V2_MQTT_PORT_NOSLL
#error "FOG_V2_MQTT_PORT_NOSLL is not define"
#endif

#define MQTT_SERVER              FOG_V2_MQTT_DOMAIN_NAME
#define MQTT_SERVER_PORT         FOG_V2_MQTT_PORT_NOSLL

#endif // MQTT_CLIENT_SSL_ENABLE

void init_fog_mqtt_service( void );

static void mqtt_client_thread( mico_thread_arg_t arg );

static void messageArrived( MessageData* md );
static void cmdArrived( MessageData* md );
OSStatus fog_v2_device_recv_command( char *payload, uint32_t payload_len, uint32_t timeout );


#if (FOG_V2_USE_SUB_DEVICE == 1)
//----------子设备API----------
//增加topic
OSStatus add_mqtt_topic_command(char* topic);
OSStatus add_mqtt_topic_cmd(char* topic);
static OSStatus add_mqtt_topic(char* topic, enum QoS qos, messageHandler messageHandler);

//删除topic
OSStatus remove_mqtt_topic( char* topic );

static void subdevice_command_handler( MessageData* md );
static void subdevice_cmd_handler( MessageData* md );

static void fog_v2_mqtt_process_subdevice_topic(Client *c);
//----------子设备API----------
#endif

static mqtt_context_t *mqtt_context = NULL;

static mqtt_sub_topic mqtt_sub_settings;
static mqtt_unsub_topic mqtt_unsub_settings;

//初始化fog mqtt 服务
void init_fog_mqtt_service( void )
{
    OSStatus err = kGeneralErr;

#if (MQTT_CLIENT_SSL_ENABLE == 1)
    int mqtt_thread_stack_size = 0x2800;
#else
    int mqtt_thread_stack_size = 0x1000;
#endif

    /* get free memory */
    app_log("num_of_chunks:%d,allocted_memory:%d, free:%d, total_memory:%d", MicoGetMemoryInfo()->num_of_chunks, MicoGetMemoryInfo()->allocted_memory, MicoGetMemoryInfo()->free_memory, MicoGetMemoryInfo()->total_memory);

#if (FOG_V2_USE_SUB_DEVICE == 1)
    err = mico_rtos_init_mutex( &(mqtt_sub_settings.mutex) );
    require_noerr( err, exit );

    err = mico_rtos_init_mutex( &(mqtt_unsub_settings.mutex) );
    require_noerr( err, exit );

    err = mico_rtos_init_semaphore( &mqtt_sub_settings.sub_sem, 1 ); //0/1 binary semaphore || 0/N semaphore
    require_noerr( err, exit );

    err = mico_rtos_init_semaphore( &mqtt_sub_settings.finish_sem, 1 ); //0/1 binary semaphore || 0/N semaphore
    require_noerr( err, exit );

    err = mico_rtos_init_semaphore( &mqtt_unsub_settings.unsub_sem, 1 ); //0/1 binary semaphore || 0/N semaphore
    require_noerr( err, exit );

    err = mico_rtos_init_semaphore( &mqtt_unsub_settings.finish_sem, 1 ); //0/1 binary semaphore || 0/N semaphore
    require_noerr( err, exit );
#endif
    /* Create application context */
    mqtt_context = (mqtt_context_t *) calloc( 1, sizeof(mqtt_context_t) );
    require_action( mqtt_context, exit, err = kNoMemoryErr );

//    mqtt_context->mqtt_client_connected = false;
    set_mqtt_connect_status( false );

    /* create msg send/recv queue */
    err = mico_rtos_init_queue( &(mqtt_context->mqtt_msg_recv_queue), "mqtt_msg_recv_queue", sizeof(p_mqtt_recv_msg_t), MAX_MQTT_RECV_QUEUE_SIZE );
    require_noerr_action( err, exit, app_log("ERROR: create mqtt msg recv queue err=%d.", err) );

    /* start mqtt client */
    err = mico_rtos_create_thread( NULL, MICO_APPLICATION_PRIORITY, "mqtt_client", mqtt_client_thread, mqtt_thread_stack_size, (uint32_t) NULL );
    require_noerr_string( err, exit, "ERROR: Unable to start the mqtt client thread." );

    exit:
    return;
}

//mqtt 客户端线程
static void mqtt_client_thread( mico_thread_arg_t arg )
{
    OSStatus err = kUnknownErr;
    uint32_t mqtt_lib_version = 0;
    int rc = -1;
    fd_set readfds;
    struct timeval t = { 0, MQTT_YIELD_TMIE * 1000 };
    char sub_topic_commands[256] = { 0 };
    char sub_topic_cmd[256] = { 0 };
    int sub_topic_event_fd = -1;
    int unsub_topic_event_fd = -1;

    Client c;  // mqtt client object
    Network n;  // socket network for mqtt client
    ssl_opts ssl_settings;
    MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;

    bool no_mqtt_msg_exchange = true;

    FOG_DES_S *fog_des = get_fog_des_g( );

    char *sub_deivce_id = fog_des->device_id;
    char *mqtt_client_id = fog_des->device_id;
    char *mqtt_client_username = fog_des->device_id;
    char *mqtt_client_password = fog_des->devicepw;

    UNUSED_PARAMETER( arg );
    UNUSED_PARAMETER( mqtt_lib_version );
    UNUSED_PARAMETER( err );

    if ( (fog_des->device_id == NULL) || (fog_des->devicepw == NULL) )
    {
        app_log("EXIT: MQTT client exit with err = %d.", err);
        mico_rtos_delete_thread( NULL );
    }

    app_log("MQTT client thread started...");

    memset( &c, 0, sizeof(c) );
    memset( &n, 0, sizeof(n) );

    mqtt_lib_version = MQTTClientLibVersion( );
    app_log("MQTT client version: [%ld.%ld.%ld]", 0xFF & (mqtt_lib_version >> 16), 0xFF & (mqtt_lib_version >> 8), 0xFF & mqtt_lib_version);

    sub_topic_event_fd = mico_create_event_fd( mqtt_sub_settings.sub_sem );
    require_action( sub_topic_event_fd >= 0, exit, err = kGeneralErr );

    unsub_topic_event_fd = mico_create_event_fd( mqtt_unsub_settings.unsub_sem );
    require_action( unsub_topic_event_fd >= 0, exit, err = kGeneralErr );

    MQTT_start:
    set_mqtt_connect_status( false );

    /* 1. create network connection */
#if (MQTT_CLIENT_SSL_ENABLE == 1)
    ssl_settings.ssl_enable = true;
    ssl_settings.ssl_debug_enable = false;  // ssl debug log
    ssl_settings.ssl_version = TLS_V1_1_MODE;
    ssl_settings.ca_str_len = strlen(mqtt_server_ssl_cert_str);
    ssl_settings.ca_str = mqtt_server_ssl_cert_str;
#else
    ssl_settings.ssl_enable = false;
#endif

    network_reconnect:
    rc = NewNetwork( &n, MQTT_SERVER, MQTT_SERVER_PORT, ssl_settings );
    if ( rc < 0 )
    {
        app_log("ERROR: MQTT network connection err=%d,reconnect after 3s...", rc);
        mico_thread_sleep( 3 );
        goto network_reconnect;
    }
    else
    {
        app_log("MQTT network connection success!");
    }

    /* 2. init mqtt client */
    c.heartbeat_retry_max = 2;
    app_log("MQTT client init...");
    rc = MQTTClientInit( &c, &n, MQTT_CMD_TIMEOUT );
    if ( MQTT_SUCCESS != rc )
    {
        app_log("ERROR: MQTT client init err=%d.", rc);
        goto MQTT_disconnect;
    }
    else
    {
        app_log("MQTT client init success!");
    }

    /* 3. create mqtt client connection */
    connectData.willFlag = 0;
    connectData.MQTTVersion = 4;  // 3: 3.1, 4: v3.1.1
    connectData.clientID.cstring = mqtt_client_id;
    connectData.username.cstring = mqtt_client_username;
    connectData.password.cstring = mqtt_client_password;
    connectData.keepAliveInterval = MQTT_CLIENT_KEEPALIVE;
    connectData.cleansession = 1;

    app_log("MQTT client connecting, username:%s, pw:%s", connectData.username.cstring, connectData.password.cstring);
    rc = MQTTConnect( &c, &connectData );
    if ( MQTT_SUCCESS == rc )
    {
        app_log("MQTT client connect success!");
    }
    else
    {
        app_log("ERROR: MQTT client connect err=%d.", rc);
        goto MQTT_disconnect;
    }

    /* 4. mqtt client subscribe */
    sprintf( sub_topic_commands, "c2d/%s/commands", sub_deivce_id );
    app_log("MQTT client subscribe commands");
    rc = MQTTSubscribe( &c, sub_topic_commands, QOS0, messageArrived );
    if ( MQTT_SUCCESS == rc )
    {
        app_log("MQTT client subscribe success! recv_topic=[%s].", sub_topic_commands);
    }
    else
    {
        app_log("ERROR: MQTT client subscribe err=%d.", rc);
        goto MQTT_disconnect;
    }

    sprintf( sub_topic_cmd, "c2d/%s/cmd", sub_deivce_id );
    app_log("MQTT client subscribe cmd");
    rc = MQTTSubscribe( &c, sub_topic_cmd, QOS0, cmdArrived );
    if ( MQTT_SUCCESS == rc )
    {
        app_log("MQTT client subscribe success! recv_topic=[%s].", sub_topic_cmd);
    }
    else
    {
        app_log("ERROR: MQTT client subscribe err=%d.", rc);
        goto MQTT_disconnect;
    }

    set_mqtt_connect_status( true );

#if (FOG_V2_USE_SUB_DEVICE == 1)
    //为子设备重新订阅频道
    fog_v2_mqtt_process_subdevice_topic(&c);
#endif

    /* 5. client loop for recv msg && keepalive */
    while ( 1 )
    {
        //app_log("MQTT client running...");
        no_mqtt_msg_exchange = true;
        FD_ZERO( &readfds );
        FD_SET( c.ipstack->my_socket, &readfds );

#if (FOG_V2_USE_SUB_DEVICE == 1)
        FD_SET( sub_topic_event_fd, &readfds );
        FD_SET( unsub_topic_event_fd, &readfds );

        select( Max(Max(sub_topic_event_fd, unsub_topic_event_fd), c.ipstack->my_socket) +1, &readfds, NULL, NULL, &t );
#else
        select( c.ipstack->my_socket + 1, &readfds, NULL, NULL, &t );
#endif



#if (FOG_V2_USE_SUB_DEVICE == 1)
        /* sub_mqtt_topic change */
        if ( FD_ISSET( sub_topic_event_fd, &readfds ) )
        {
            err = mico_rtos_get_semaphore( &(mqtt_sub_settings.sub_sem), MICO_WAIT_FOREVER ); //wait until get semaphore
            if ( err != kNoErr )
            {
                app_log("[ERROR]get sub_sem timeout!!!");
                mqtt_sub_settings.is_success = false;
                mico_rtos_set_semaphore( &(mqtt_sub_settings.finish_sem));
            }else
            {
                //app_log("get sub_sem!");

                err = mico_rtos_lock_mutex( &(mqtt_sub_settings.mutex) );
                require_noerr( err, exit );

                rc = MQTTSubscribe( &c, mqtt_sub_settings.topic, mqtt_sub_settings.qos, mqtt_sub_settings.messageHandler );
                if ( MQTT_SUCCESS == rc )
                {
                    app_log("MQTT client subscribe success! recv_topic=[%s].", mqtt_sub_settings.topic);

                    mqtt_sub_settings.is_success = true;

                    err = mico_rtos_unlock_mutex( &(mqtt_sub_settings.mutex) );
                    require_noerr( err, exit );

                    mico_rtos_set_semaphore( &(mqtt_sub_settings.finish_sem) );
                }
                else
                {
                    app_log("ERROR: MQTT client subscribe err=%d.", rc);

                    mqtt_sub_settings.is_success = false;

                    err = mico_rtos_unlock_mutex( &(mqtt_sub_settings.mutex) );
                    require_noerr( err, exit );

                    mico_rtos_set_semaphore( &(mqtt_sub_settings.finish_sem) );

                    goto MQTT_disconnect;
                }
            }

        }

        /* unsub_mqtt_topic change */
        if ( FD_ISSET( unsub_topic_event_fd, &readfds ) )
        {
            err = mico_rtos_get_semaphore( &mqtt_unsub_settings.unsub_sem, 10 ); //wait until get semaphore
            if(err != kNoErr)
            {
                app_log("[ERROR]get unsub_sem timeout!!!");
                mqtt_unsub_settings.is_success = false;
                mico_rtos_set_semaphore( &(mqtt_unsub_settings.finish_sem));
            } else
            {
                //app_log("get unsub_sem!");

                err = mico_rtos_lock_mutex( &(mqtt_unsub_settings.mutex) );
                require_noerr( err, exit );

                rc = MQTTUnsubscribe( &c, mqtt_unsub_settings.topic );
                if ( MQTT_SUCCESS == rc )
                {
                    app_log("MQTT client unsubscribe success! recv_topic=[%s].", mqtt_unsub_settings.topic);

                    mqtt_unsub_settings.is_success = true;

                    err = mico_rtos_unlock_mutex( &(mqtt_unsub_settings.mutex) );
                    require_noerr( err, exit );

                    mico_rtos_set_semaphore( &(mqtt_unsub_settings.finish_sem) );
                }
                else
                {
                    app_log("ERROR: MQTT client unsubscribe err=%d.", rc);

                    mqtt_unsub_settings.is_success = false;

                    err = mico_rtos_unlock_mutex( &(mqtt_unsub_settings.mutex) );
                    require_noerr( err, exit );

                    mico_rtos_set_semaphore( &(mqtt_unsub_settings.finish_sem) );

                    goto MQTT_disconnect;
                }
            }
        }
#endif

        /* recv msg from server */
        if ( FD_ISSET( c.ipstack->my_socket, &readfds ) )
        {
            rc = MQTTYield( &c, (int) MQTT_YIELD_TMIE );
            if ( MQTT_SUCCESS != rc )
            {
                goto MQTT_disconnect;
            }
            //no_mqtt_msg_exchange = false;
        }

        /* if no msg exchange, we need to check ping msg to keep alive. */
        if ( no_mqtt_msg_exchange )
        {
            rc = keepalive( &c );
            if ( MQTT_SUCCESS != rc )
            {
                app_log("ERROR: keepalive err=%d.", rc);
                goto MQTT_disconnect;
            }
        }

        continue;

        MQTT_disconnect:
        app_log("MQTT client disconnected, reconnect after 3s...");
        if ( c.isconnected )
        {
            MQTTDisconnect( &c );
        }  // send mqtt disconnect msg
        n.disconnect( &n );  // close connection
        rc = MQTTClientDeinit( &c );  // free mqtt client resource
        if ( MQTT_SUCCESS != rc )
        {
            app_log("MQTTClientDeinit failed!");
            err = kDeletedErr;
        }
        set_mqtt_connect_status( false );
        mico_thread_sleep( 3 );
        goto MQTT_start;
    }

    set_mqtt_connect_status( false );
    if ( c.isconnected )
    {
        MQTTDisconnect( &c );
    }
    n.disconnect( &n );
    rc = MQTTClientDeinit( &c );
    if ( MQTT_SUCCESS != rc )
    {
        app_log("MQTTClientDeinit failed!");
        err = kDeletedErr;
    }

  exit:
#if (FOG_V2_USE_SUB_DEVICE == 1)
    mico_rtos_deinit_mutex( &(mqtt_sub_settings.mutex) );
    mico_rtos_deinit_mutex( &(mqtt_unsub_settings.mutex) );

    mico_rtos_deinit_semaphore( &mqtt_sub_settings.sub_sem );
    mico_rtos_deinit_semaphore( &mqtt_sub_settings.finish_sem );
    mico_rtos_deinit_semaphore( &mqtt_unsub_settings.unsub_sem );
    mico_rtos_deinit_semaphore( &mqtt_unsub_settings.finish_sem );
#endif

    mico_rtos_deinit_event_fd( sub_topic_event_fd );
    mico_rtos_deinit_event_fd( unsub_topic_event_fd );

    app_log("EXIT: MQTT client exit with err = %d.", err);
    mico_rtos_delete_thread( NULL );
}

// msg received from mqtt server with callback
static void messageArrived( MessageData* md )
{
    OSStatus err = kUnknownErr;
    p_mqtt_recv_msg_t p_recv_msg = NULL;
    MQTTMessage* message = md->message;

    p_recv_msg = (p_mqtt_recv_msg_t) malloc( sizeof(mqtt_recv_msg_t) );
    if ( NULL != p_recv_msg )
    {
        memset( p_recv_msg, 0, sizeof(mqtt_recv_msg_t) );
        strncpy( p_recv_msg->topic, md->topicName->lenstring.data, md->topicName->lenstring.len );
        memcpy( p_recv_msg->data, message->payload, message->payloadlen );
        p_recv_msg->datalen = message->payloadlen;
        p_recv_msg->qos = (char) (message->qos);
        p_recv_msg->retained = message->retained;
        err = mico_rtos_push_to_queue( &(mqtt_context->mqtt_msg_recv_queue), &p_recv_msg, 0 );
        if ( kNoErr != err )
        {
            app_log("[error]push mqtt recv msg into recv queue err=%d. queue is full!", err);
            free( p_recv_msg ); //记得释放！！
            p_recv_msg = NULL;
        } else
        {
            //app_log("[mqtt command][%d][%s]", (int)message->payloadlen, (char*)message->payload);
        }
    } else
    {
        app_log("ERROR: create mqtt recv msg failed!!!");
    }
}

// msg received from mqtt server with callback
static void cmdArrived( MessageData* md )
{
    OSStatus err = kUnknownErr;
    p_mqtt_recv_msg_t p_recv_msg = NULL;
    MQTTMessage* message = md->message;

    p_recv_msg = (p_mqtt_recv_msg_t) malloc( sizeof(mqtt_recv_msg_t) );
    if ( NULL != p_recv_msg )
    {
        memset( p_recv_msg, 0, sizeof(mqtt_recv_msg_t) );
        strncpy( p_recv_msg->topic, md->topicName->lenstring.data, md->topicName->lenstring.len );
        memcpy( p_recv_msg->data, message->payload, message->payloadlen );
        p_recv_msg->datalen = message->payloadlen;
        p_recv_msg->qos = (char) (message->qos);
        p_recv_msg->retained = message->retained;

        //处理cmd数据....

        app_log("[mqtt CMD][%d][%s]", (int)p_recv_msg->datalen, (char*)p_recv_msg->data);

        err = process_fog_v2_mqtt_cmd( (const char *) (p_recv_msg->data) );
        require_noerr_action( err, exit, err = kGeneralErr );
    }
    else
    {
        app_log("ERROR: create mqtt recv msg failed!!!");
    }

    exit:
    if ( p_recv_msg != NULL )
    {
        free( p_recv_msg ); //记得释放！！
        p_recv_msg = NULL;
    }
}

//功能：从云端接收数据
//参数： payload - 接收数据缓冲区地址
//参数： payload_len - 接收数据缓冲区地址的长度
//参数： timeout - 接收数据等待时间
//返回值：kNoErr为成功 其他值为失败
OSStatus fog_v2_device_recv_command( char *payload, uint32_t payload_len, uint32_t timeout )
{
    OSStatus err = kUnknownErr;
    p_mqtt_recv_msg_t p_recv_msg = NULL;

    require_action( mqtt_context != NULL, exit, err = kGeneralErr );
    require_action( payload != NULL, exit, err = kGeneralErr );

    memset(payload, 0, payload_len);

    // get msg from recv queue
    err = mico_rtos_pop_from_queue( &(mqtt_context->mqtt_msg_recv_queue), &p_recv_msg, timeout );
    require_noerr( err, exit );

    if ( p_recv_msg )
    {
        //app_log("user get data success! from_topic=[%s], msg=[%d][%s].", p_recv_msg->topic, p_recv_msg->datalen, p_recv_msg->data);

        require_action( payload_len > p_recv_msg->datalen, exit, err = kGeneralErr );

        memset( payload, 0, payload_len );
        memcpy( payload, p_recv_msg->data, p_recv_msg->datalen );

        //app_log("[RECV] command: %s", payload);

        // release msg mem resource
        free( p_recv_msg );
        p_recv_msg = NULL;
    } else
    {
        app_log("[RECV] ERROR!");
    }

    exit:
    return err;
}

#if (FOG_V2_USE_SUB_DEVICE == 1)
//------------子设备相关api------------

//监听子设备command频道
OSStatus add_mqtt_topic_command(char* topic)
{
    return add_mqtt_topic(topic, QOS0, subdevice_command_handler);
}

//监听子设备cmd频道
OSStatus add_mqtt_topic_cmd(char* topic)
{
    return add_mqtt_topic(topic, QOS0, subdevice_cmd_handler);
}

//子设备增加topic 注意:topic的内存地址不能被释放
static OSStatus add_mqtt_topic(char* topic, enum QoS qos, messageHandler messageHandler)
{
    OSStatus err = kGeneralErr;

    if(strlen(topic) >= MAX_MQTT_TOPIC_SIZE)
        return kGeneralErr;

    if(messageHandler == NULL)
        return kGeneralErr;

    require_string(mqtt_unsub_settings.mutex != NULL, exit, "[ERROR] MQTT client is not init!");

    err = mico_rtos_lock_mutex(&(mqtt_sub_settings.mutex));
    require_noerr( err, exit );

    mqtt_sub_settings.qos = qos;
    mqtt_sub_settings.messageHandler = messageHandler;
    mqtt_sub_settings.topic = topic;
    mqtt_sub_settings.is_success = false;

    err = mico_rtos_unlock_mutex(&(mqtt_sub_settings.mutex));
    require_noerr( err, exit );

    mico_rtos_set_semaphore( &(mqtt_sub_settings.sub_sem));

    mico_rtos_get_semaphore( &mqtt_sub_settings.finish_sem, MICO_WAIT_FOREVER); //等待订阅完成

    err = mico_rtos_lock_mutex(&(mqtt_sub_settings.mutex));
    require_noerr( err, exit );

    if(mqtt_sub_settings.is_success == true)
    {
        err = kNoErr;
    }else if(mqtt_sub_settings.is_success == false)
    {
        err = kGeneralErr;
    }

    err = mico_rtos_unlock_mutex(&(mqtt_sub_settings.mutex));
    require_noerr( err, exit );
exit:
    return err;
}

//删除topic
OSStatus remove_mqtt_topic( char* topic )
{
    OSStatus err = kGeneralErr;

    if(strlen(topic) >= MAX_MQTT_TOPIC_SIZE)
        return kGeneralErr;

    app_log("remove mqtt topic : %s", topic);

    require_string(mqtt_unsub_settings.mutex != NULL, exit, "[ERROR] MQTT client is not init!");

    err = mico_rtos_lock_mutex(&(mqtt_unsub_settings.mutex));
    require_noerr( err, exit );

    mqtt_unsub_settings.topic = topic;
    mqtt_unsub_settings.is_success = false;

    err = mico_rtos_unlock_mutex(&(mqtt_unsub_settings.mutex));
    require_noerr( err, exit );

    mico_rtos_set_semaphore( &(mqtt_unsub_settings.unsub_sem));

    mico_rtos_get_semaphore( &mqtt_unsub_settings.finish_sem, MICO_WAIT_FOREVER); //等待订阅完成

    err = mico_rtos_lock_mutex(&(mqtt_unsub_settings.mutex));
    require_noerr( err, exit );

    if(mqtt_unsub_settings.is_success == true)
    {
        err = kNoErr;
    }else if(mqtt_unsub_settings.is_success == false)
    {
        err = kGeneralErr;
    }

    err = mico_rtos_unlock_mutex(&(mqtt_unsub_settings.mutex));
    require_noerr( err, exit );

exit:
    return err;
}

//子设备的command回调
static void subdevice_command_handler( MessageData* md )
{
    OSStatus err = kGeneralErr;
    uint32_t index = 0;
    SUBDEVICE_RECV_DATA_S *subdevice_recv_p = NULL;
    mico_queue_t *sub_device_queue_p = NULL;
    char mqtt_command_topic[128] = {0};

    memcpy(mqtt_command_topic, md->topicName->lenstring.data, md->topicName->lenstring.len);

    app_log("[COMMADN][%s]:[%d][%s]", mqtt_command_topic, (int)md->message->payloadlen, (char*)md->message->payload);

    if(get_sub_device_queue_index_by_mqtt_topic(&index, mqtt_command_topic) == false)
    {
        app_log("[error] get error command data!!!");
        return;
    }

    subdevice_recv_p = (SUBDEVICE_RECV_DATA_S *)malloc(sizeof(SUBDEVICE_RECV_DATA_S));
    require_action(subdevice_recv_p != NULL , exit, err = kGeneralErr );

    //malloc data and put data to queue
    memset(subdevice_recv_p, 0, sizeof(SUBDEVICE_RECV_DATA_S));

    subdevice_recv_p->data = malloc(md->message->payloadlen + 1);//多申请一个字节
    require_action(subdevice_recv_p->data != NULL , exit, err = kGeneralErr );

    subdevice_recv_p->data_len = md->message->payloadlen + 1;

    memset(subdevice_recv_p->data, 0, subdevice_recv_p->data_len);

    memcpy(subdevice_recv_p->data, md->message->payload, md->message->payloadlen); //少copy一个字节

    sub_device_queue_p = get_sub_device_queue_addr_by_index(index);
    require_action(sub_device_queue_p != NULL , exit, err = kGeneralErr );

    err = mico_rtos_push_to_queue( sub_device_queue_p, &subdevice_recv_p, 0 );
    require_action_string(err == kNoErr , exit, err = kGeneralErr, "queue is full!!");

  exit:
    if ( kNoErr != err )
    {
        if ( subdevice_recv_p != NULL )
        {
            if ( subdevice_recv_p->data != NULL )
            {
                free( subdevice_recv_p->data ); //如果插入数据失败, 记得释放！！！！
                subdevice_recv_p->data = NULL;
            }

            free( subdevice_recv_p ); //释放本体
            subdevice_recv_p = NULL;
        }
    }

    return;
}

//子设备的cmd回调
static void subdevice_cmd_handler( MessageData* md )
{
    app_log("[CMD][%s,%s]:[%d][%s]", md->topicName->cstring, md->topicName->lenstring.data, (int)md->message->payloadlen, (char*)md->message->payload);
}


//mqtt如果重连 则为子设备重新订阅频道
static void fog_v2_mqtt_process_subdevice_topic(Client *c)
{
    uint32_t i = 0, j = 0;
    int rc = -1;

    lock_subdevice_des_mutex(); //加锁 +++++++
    for ( i = 0; i < FOG_V2_SUB_DEVICE_MAX_NUM; i++ )
    {
        for(j = 0; j < MQTT_SUBSCRIBE_RETRY; j ++)
        {
            if ( get_fog_des_g( )->sub_des[i].s_is_activate == true )
            {
                if(strstr(get_fog_des_g( )->sub_des[i].s_mqtt_topic_commands, "/commands") == NULL)
                {
                    app_log("s_mqtt_topic_commands is error, product id:%s, mac:%s", get_fog_des_g( )->sub_des[i].s_product_id, get_fog_des_g( )->sub_des[i].s_device_mac);
                    break;
                }

                rc = MQTTSubscribe( c, get_fog_des_g( )->sub_des[i].s_mqtt_topic_commands, QOS0, subdevice_command_handler);
                if(rc == MQTT_SUCCESS)
                {
                    app_log("[SUCCESS]device add topic, %s", get_fog_des_g( )->sub_des[i].s_mqtt_topic_commands);
                    break;
                }else
                {
                    app_log("[FAILED]device add topic, %s", get_fog_des_g( )->sub_des[i].s_mqtt_topic_commands);
                }
            }
        }
    }
    unlock_subdevice_des_mutex(); //解锁锁 -------
}

#endif
//------------子设备相关api------------
