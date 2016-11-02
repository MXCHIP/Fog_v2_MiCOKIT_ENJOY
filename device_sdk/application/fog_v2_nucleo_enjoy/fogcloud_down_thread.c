#include "mico.h"
#include "fogcloud_down_thread.h"
#include "fog_v2_config.h"
#include "user_common.h"
#include "MiCOKit_STmems.h"
#include "sensor\DHT11\DHT11.h"
#include "template_analysis.h"
#include "user_template.h"

#define fogcloud_down_log(M, ...) custom_log("FOG_DOWN_DATA", M, ##__VA_ARGS__)

void user_downstream_thread(mico_thread_arg_t arg);
OSStatus init_receive_data_queue(void);
void process_recv_data(char *data, uint32_t len);


//解析数据包中的command_data
void process_command_data(uint32_t command_id, struct json_object* json_command_G)
{
    json_object  *node_json_obj = NULL, *PID_json_obj = NULL, *D_json_obj = NULL, *T_json_obj = NULL;
    uint32_t G_length = 0;
    uint32_t i = 0, pid = 0, type = 0;
    bool data_bool = false;
    int data_int = 0;

    G_length = json_object_array_length(json_command_G);
    fogcloud_down_log("G's length is %ld", G_length);

    for(i = 0; i < G_length; i ++)
    {
        node_json_obj = json_object_array_get_idx(json_command_G, i);
        if(node_json_obj == NULL)
        {
            continue;
        }

        PID_json_obj = json_object_object_get(node_json_obj, PROTOCOL_G_PID);
        if((PID_json_obj == NULL) || ((json_object_get_object(PID_json_obj)->head) == NULL))
        {
            fogcloud_down_log("get PID_json_obj error");
            continue;
        }

        pid = json_object_get_int(PID_json_obj);


        T_json_obj = json_object_object_get(node_json_obj, PROTOCOL_G_T);
        if((T_json_obj == NULL) || ((json_object_get_object(T_json_obj)->head) == NULL))
        {
            fogcloud_down_log("get T_json_obj error");
            continue;
        }

        type = json_object_get_int(T_json_obj);

        fogcloud_down_log("PID = %ld, type = %ld", pid, type);

        if(type < USER_NODE_TYPE_VALUE_MIN || type > USER_NODE_TYPE_VALUE_MAX)
        {
            fogcloud_down_log("type errror, type = %ld", type);
            continue;
        }

        if(type >= USER_NODE_TYPE_READ_MIN && type <= USER_NODE_TYPE_READ_MAX)
        {
            fogcloud_down_log("type is read type? please insure your micokit app. pid = %ld", pid);
            continue;
        }

        //BOOL控制型
        if(type >= USER_NODE_TYPE_BOOL_CTRL_MIN && type <= USER_NODE_TYPE_BOOL_CTRL_MAX)
        {
            D_json_obj = json_object_object_get(node_json_obj, PROTOCOL_G_D);
            if((D_json_obj == NULL) || ((json_object_get_object(D_json_obj)->head) == NULL))
            {
                fogcloud_down_log("get T_json_obj error");
                continue;
            }

            if(json_object_is_type(D_json_obj, json_type_boolean) == false)
            {
                fogcloud_down_log("data type is not bool, pid = %ld", pid);
                continue;
            }

            data_bool = json_object_get_boolean(D_json_obj);

            sys_process_bool_node(pid, type, data_bool);
            continue;
        }

        //PWM控制类型
        if(type >= USER_NODE_TYPE_PWM_CTRL_MIN && type <= USER_NODE_TYPE_PWM_CTRL_MAX)
        {
            D_json_obj = json_object_object_get(node_json_obj, PROTOCOL_G_D);
            if((D_json_obj == NULL) || ((json_object_get_object(D_json_obj)->head) == NULL))
            {
                fogcloud_down_log("get T_json_obj error");
                continue;
            }

            if(json_object_is_type(D_json_obj, json_type_int) == false)
            {
                fogcloud_down_log("data type is not int, pid = %ld", pid);
                continue;
            }

           data_int = json_object_get_int(D_json_obj);

            sys_process_pwm_node(pid, type, data_int);
            continue;
        }

        //私有类型
        if(type >= USER_NODE_TYPE_PRIVATE_MIN && type <= USER_NODE_TYPE_PRIVATE_MAX)
        {
            D_json_obj = json_object_object_get(node_json_obj, PROTOCOL_G_D);
            if((D_json_obj == NULL) || ((json_object_get_object(D_json_obj)->head) == NULL))
            {
                fogcloud_down_log("get T_json_obj error");
                continue;
            }

            if(json_object_is_type(D_json_obj, json_type_object) == false)
            {
                fogcloud_down_log("data type is not object, pid = %ld", pid);
                continue;
            }

            sys_process_private_node(pid, type, D_json_obj);
            continue;
        }
    }

    return;
}


OSStatus send_user_template(void)
{
    uint32_t user_template_len = 1024;  //需要看模板大小
    char *user_template = NULL;
    OSStatus err = kGeneralErr;

    user_template = malloc(user_template_len);
    require_action_string((user_template != NULL), exit, err = kNoMemoryErr, "create user_template error!");


    memset(user_template, 0, user_template_len);

    err = sys_generate_user_template(user_template, user_template_len);
    require_noerr( err, exit );

    fogcloud_down_log("[%d]%s", strlen(user_template) ,user_template);

    fog_v2_device_send_event(user_template, FOG_V2_SEND_EVENT_RULES_PUBLISH);

 exit:
    if(user_template != NULL)
    {
        free(user_template);
        user_template = NULL;
    }
    return err;
}


//解析接收到的数据包
void process_recv_data(char *data, uint32_t len)
{
    json_object *recv_json_object = NULL, *command_data_json_obj = NULL, *protocol_command_id_obj = NULL, *protocol_cmd_obj = NULL;
    uint32_t cmd = 0, command_id = 0;


    if(*data != '{')
    {
        fogcloud_down_log("error:recv data is not json format");
        goto exit;
    }

    recv_json_object = json_tokener_parse((const char*)(data));
    if (NULL == recv_json_object)
    {
        fogcloud_down_log("json_tokener_parse error");
        goto exit;
    }

    //2.解析CMD和command_id
    protocol_command_id_obj = json_object_object_get(recv_json_object, PROTOCOL_COMMAND_ID);
    if((protocol_command_id_obj == NULL) || ((json_object_get_object(protocol_command_id_obj)->head) == NULL))
    {
        fogcloud_down_log("get protocol_command_id_obj error");
        goto exit;
    }

    command_id = json_object_get_int(protocol_command_id_obj);


    protocol_cmd_obj = json_object_object_get(recv_json_object, PROTOCOL_CMD);
    if((protocol_cmd_obj == NULL) || ((json_object_get_object(protocol_cmd_obj)->head) == NULL))
    {
        fogcloud_down_log("get cmd error");
        goto exit;
    }

    cmd = json_object_get_int(protocol_cmd_obj);

    if(C2D_SEND_COMMANDS != cmd && C2D_GET_TEMPLATE != cmd)
    {
        fogcloud_down_log("cmd error, cmd = %ld", cmd);
        goto exit;
    }

    if(C2D_GET_TEMPLATE == cmd)
    {
        send_user_template();
        goto exit;
    }

    //3.解析command data
    command_data_json_obj = json_object_object_get(recv_json_object, PROTOCOL_G);
    if((command_data_json_obj == NULL) || ((json_object_get_object(command_data_json_obj)->head) == NULL))
    {
        fogcloud_down_log("json string parse command_type error");
        goto exit;
    }

    process_command_data(command_id, command_data_json_obj);

 exit:
    free_json_obj(&recv_json_object);    //只要free最顶层的那个就可以
}

//用户接收数据线程
void user_downstream_thread( mico_thread_arg_t arg )
{
    OSStatus err = kUnknownErr;
    char *fog_recv_buff = NULL;

    fog_recv_buff = malloc( FOG_V2_RECV_BUFF_LEN );
    require( fog_recv_buff != NULL, exit );

    fogcloud_down_log("------------downstream_thread start------------");

    while ( 1 )
    {
        memset(fog_recv_buff, 0, FOG_V2_RECV_BUFF_LEN);
        err = fog_v2_device_recv_command( fog_recv_buff, FOG_V2_RECV_BUFF_LEN, MICO_NEVER_TIMEOUT );
        if ( err == kNoErr )
        {
            fogcloud_down_log("recv:%s", fog_recv_buff);
            process_recv_data( fog_recv_buff, strlen(fog_recv_buff ) );    //解析数据
        }
    }

    exit:
    if ( fog_recv_buff != NULL )
    {
        free( fog_recv_buff );
        fog_recv_buff = NULL;
    }

    if ( kNoErr != err )
    {
        fogcloud_down_log("ERROR: user_downstream_thread exit with err=%d", err);
    }
    mico_rtos_delete_thread( NULL );  // delete current thread

    return;
}
