#include "mico.h"
#include "user_template.h"
#include "template_analysis.h"
#include "micokit_ext.h"

#define app_log(M, ...)     custom_log("USER_TEMPLATE", M, ##__VA_ARGS__)

//系统使用
OSStatus sys_generate_user_template(char *user_template, uint32_t template_len);
void sys_process_private_node(uint32_t pid, uint32_t type, struct json_object* D_json_obj);
void sys_process_pwm_node(uint32_t pid, uint32_t type, int32_t data);
void sys_process_bool_node(uint32_t pid, uint32_t type, bool data_bool);


//用户自定义的电机处理程序
void micokit_motor_callback(uint32_t pid, bool bool_status);
void micokit_pwm_callback(uint32_t pid, int32_t data);
void micokit_rgbled_callback(uint32_t pid, struct json_object* D_json_obj);


PLATFORM_APP_TEMPLATE_G paltform_app_template[] =
{
    {
        .type           =   USER_NODE_TYPE_TEMPERATURE,
        .pid            =   USER_NODE_PID_TEMPERATURE,
        .min            =   0,
        .max            =   0,
        .uint           =   "℃",
        .name           =   "温度",
        .ext            =   "ST的传感器",
    },
    {
        .type           =   USER_NODE_TYPE_HUMIDITY,
        .pid            =   USER_NODE_PID_HUMIDITY,
        .min            =   0,
        .max            =   0,
        .uint           =   "%",
        .name           =   "湿度",
        .ext            =   "ST的传感器",
    },
    {
        .type           =   USER_NODE_TYPE_LIGHT_INTENSITY,
        .pid            =   USER_NODE_PID_LIGHT_INTENSITY,
        .min            =   0,
        .max            =   0,
        .uint           =   "flux",
        .name           =   "光强",
        .ext            =   "ST的传感器",
    },
    {
        .type           =   USER_NODE_TYPE_INFRARED,
        .pid            =   USER_NODE_PID_INRFRARED,
        .min            =   0,
        .max            =   0,
        .uint           =   "mv",
        .name           =   "红外线",
        .ext            =   "ST的传感器",
    },
    {
        .type           =   USER_NODE_TYPE_MOTOR,
        .pid            =   USER_NODE_PID_MOTOR,
        .min            =   0,
        .max            =   0,
        .uint           =   "",
        .name           =   "电机",
        .ext            =   "ST的传感器",
        .receive_callback = (void *)micokit_motor_callback, //用户为每个外设定义单独的回调函数
    },
    {
        .type           =   USER_NODE_TYPE_RGB_LED,
        .pid            =   USER_NODE_PID_RGB_LED,
        .min            =   0,
        .max            =   0,
        .uint           =   "",
        .name           =   "RGB灯",
        .ext            =   "ST的传感器",
        .receive_callback = (void *)micokit_rgbled_callback,    //用户为每个外设定义单独的回调函数
    }
};


//生成用户的数据模板
OSStatus sys_generate_user_template(char *user_template, uint32_t template_len)
{
    OSStatus err = kUnknownErr;
    TEMPLATE_NODE node;
    uint32_t  i = 0, node_num = 0;

    memset(user_template, 0, template_len);

    //初始化数据点模板
    err = init_template(1000, D2C_ACK_TEMPLATE);
    require_noerr(err, exit);


    node_num = sizeof(paltform_app_template) / sizeof(paltform_app_template[0]); //获取数组成员数量

    for(i = 0; i < node_num; i ++)
    {
        err = set_node_attr(&node, paltform_app_template[i].type, paltform_app_template[i].pid, paltform_app_template[i].min, paltform_app_template[i].max, paltform_app_template[i].uint, paltform_app_template[i].name, paltform_app_template[i].ext);
        require_noerr_action(err, exit, app_log("node [%ld] is error", i));
    }

    err = generate_final_template(user_template, template_len);
    require_noerr(err, exit);

 exit:
    //注销数据点模板
    destory_template();
    return err;
}


//用户处理bool类型的下行数据
void sys_process_bool_node(uint32_t pid, uint32_t type, bool data_bool)
{
    uint32_t node_num = 0, i = 0;

    if(type < USER_NODE_TYPE_BOOL_CTRL_MIN || type > USER_NODE_TYPE_BOOL_CTRL_MAX)
    {
         app_log("get error type in sys_process_bool_node function, pid = %ld", pid);
         return;
    }

    node_num = (sizeof(paltform_app_template) / sizeof(PLATFORM_APP_TEMPLATE_G)); //获取数组成员数量

    for(i = 0; i < node_num; i ++)
    {
        if(paltform_app_template[i].pid == pid)
        {
            if(paltform_app_template[i].receive_callback != NULL)
            {
                ((USER_BOOL_TYPE_CALLBACK)(paltform_app_template[i].receive_callback))(pid, data_bool);
            }else
            {
                app_log("[WARNING] pid = %ld, receive_callback is NULL", pid);
            }
        }
    }
}


//处理PWM类型节点
void sys_process_pwm_node(uint32_t pid, uint32_t type, int32_t data_int)
{
    uint32_t node_num = 0, i = 0;

    if(type < USER_NODE_TYPE_PWM_CTRL_MIN || type > USER_NODE_TYPE_PWM_CTRL_MAX)
    {
         app_log("get error type in sys_process_pwm_node function, pid = %ld", pid);
         return;
    }

    node_num = (sizeof(paltform_app_template) / sizeof(PLATFORM_APP_TEMPLATE_G)); //获取数组成员数量

    for(i = 0; i < node_num; i ++)
    {
        if(paltform_app_template[i].pid == pid)
        {
            if(paltform_app_template[i].receive_callback != NULL)
            {
                ((USER_PWM_TYPE_CALLBACK)(paltform_app_template[i].receive_callback))(pid, data_int);
            }else
            {
                app_log("[WARNING] pid = %ld, receive_callback is NULL", pid);
            }
        }
    }
}


//处理私有类型的节点
void sys_process_private_node(uint32_t pid, uint32_t type, struct json_object* D_json_obj)
{
    uint32_t node_num = 0, i = 0;

    if(type < USER_NODE_TYPE_PRIVATE_MIN || type > USER_NODE_TYPE_PRIVATE_MAX)
    {
         app_log("get error type in sys_process_private_node function, pid = %ld", pid);
         return;
    }

    node_num = (sizeof(paltform_app_template) / sizeof(PLATFORM_APP_TEMPLATE_G)); //获取数组成员数量

    for(i = 0; i < node_num; i ++)
    {
        if(paltform_app_template[i].pid == pid)
        {
            if(paltform_app_template[i].receive_callback != NULL)
            {
                ((USER_PRIVATE_CALLBACK)(paltform_app_template[i].receive_callback))(pid, D_json_obj);
            }else
            {
                app_log("[WARNING] pid = %ld, receive_callback is NULL", pid);
            }
        }
    }
}




//--------------华丽丽的分割线--------------

//用户定义的三种回调函数需要符合这三种规范
//typedef void (*USER_BOOL_TYPE_CALLBACK)(uint32_t pid, bool data_bool);      //BOOL类型
//typedef void (*USER_PWM_TYPE_CALLBACK)(uint32_t pid, int32_t pwm_data);    //PWM类型
//typedef void (*USER_PRIVATE_CALLBACK)(uint32_t pid, struct json_object* D_json_obj); //私有类型

//用户注册的电机回调函数
void micokit_motor_callback(uint32_t pid, bool bool_status)
{
    //your code...
    dc_motor_set(bool_status);
    return;
}

//用户注册的pwm回调函数
void micokit_pwm_callback(uint32_t pid, int32_t data)
{
    app_log("user: pid = %ld, data = %ld", pid, data);
    //your code....

    return;
}

//用户注册的rgb灯回调函数
void micokit_rgbled_callback(uint32_t pid, struct json_object* D_json_obj)
{
    // your code...
    bool rgb_switch = false;
    uint32_t rgb_h = 0, rgb_s = 0, rgb_v = 0;

    if(pid == USER_NODE_PID_RGB_LED)
    {
        json_object_object_foreach(D_json_obj, key, val)
        {
            if(0 == strcmp(key, PROTOCOL_G_D_RGB_SWITCH))
            {
                rgb_switch = json_object_get_boolean(val);
            }else if(0 == strcmp(key, PROTOCOL_G_D_RGB_H))
            {
                rgb_h = json_object_get_int(val);
            }else if(0 == strcmp(key, PROTOCOL_G_D_RGB_S))
            {
                rgb_s = json_object_get_int(val);
            }else if(0 == strcmp(key, PROTOCOL_G_D_RGB_V))
            {
                rgb_v = json_object_get_int(val);
            }
        }

        if(rgb_switch == false)
        {
            hsb2rgb_led_open(0, 0, 0);
        }else if(rgb_switch == true)
        {
            hsb2rgb_led_open(rgb_h, rgb_s, rgb_v);
        }
    }
}
