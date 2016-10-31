//注意该文件一定要用UTF-8格式打开,否则云端接收数据可能中文显示错误,导致发送数据失败

#include "user_common.h"
#include "template_analysis.h"


#define app_log(M, ...)     custom_log("TEMPLATE", M, ##__VA_ARGS__)
#define app_log_trace()     custom_log_trace("TEMPLATE")


//模板
OSStatus init_template(uint32_t command_id, uint32_t CMD);
OSStatus add_peripherals_to_template(TEMPLATE_NODE *template_node);
OSStatus generate_final_template(char *receice_data, uint32_t max_receice_buff_len);
void destory_template(void);
OSStatus set_node_attr(TEMPLATE_NODE *node, uint32_t type, uint32_t pid, int32_t min, int32_t max, const char *uint, const char *name, const char *ext);


//上传数据类
OSStatus init_upload_data(uint32_t command_id, uint32_t CMD);
OSStatus add_bool_peripherals_to_upload_data(uint32_t type, uint32_t pid, bool data);
OSStatus add_int_peripherals_to_upload_data(uint32_t type, uint32_t pid, int32_t data);
OSStatus add_float_peripherals_to_upload_data(uint32_t type, uint32_t pid, float data);
OSStatus add_string_peripherals_to_upload_data(uint32_t type, uint32_t pid, const char  *data, uint32_t len);
OSStatus add_RGB_LED_peripherals_to_upload_data(uint32_t type, uint32_t pid, bool user_switch, uint32_t h, uint32_t s, uint32_t v);
OSStatus generate_final_upload_data(char *receice_data, uint32_t max_receice_buff_len);
void destory_upload_data(void);


json_object *mico_template = NULL, *mico_template_G = NULL;
json_object *mico_data = NULL, *mico_data_G = NULL;


//设置节点属性
OSStatus set_node_attr(TEMPLATE_NODE *node, uint32_t type, uint32_t pid, int32_t min, int32_t max, const char *uint, const char *name, const char *ext)
{
    OSStatus err = kUnknownErr;

    if(uint == NULL || name == NULL || ext == NULL)
    {
        app_log("pointer is NULL");
        return kGeneralErr;
    }

    memset(node, 0, sizeof(TEMPLATE_NODE));

    node->type = type;
    node->pid = pid;

    node->name_len = strlen(name);
    node->uint_len = strlen(uint);
    node->ext_len = strlen(ext);

    if((node->name_len > MAX_NAME_LEN) || (node->uint_len > MAX_UNIT_LEN) || (node->ext_len > MAX_EXT_LEN))
    {
        app_log("max len is overflow");
        return kGeneralErr;
    }

    node->uint = uint;
    node->name = name;
    node->ext = ext;

    if((type >= USER_NODE_TYPE_PWM_CTRL_MIN) && type <= USER_NODE_TYPE_PWM_CTRL_MAX)
    {
        if(min >= max)
        {
            app_log("min is bigger than max");
            return kGeneralErr;
        }
        node->min = min;
        node->max = max;
    }

    err = add_peripherals_to_template(node);
    require_noerr(err, exit);

 exit:
    return err;
}


//1.初始化模板  不能多次初始化
OSStatus init_template(uint32_t command_id, uint32_t CMD)
{
    OSStatus err = kUnknownErr;

    if(mico_template != NULL || mico_template_G != NULL)
    {
       destory_template();
    }

    mico_template = json_object_new_object();
    require_action_string(mico_template, exit, err = kNoMemoryErr, "create json object error!");

    json_object_object_add(mico_template, PROTOCOL_COMMAND_ID, json_object_new_int(command_id));
    json_object_object_add(mico_template, PROTOCOL_CMD, json_object_new_int(CMD));

    mico_template_G = json_object_new_array();
    require_action_string(mico_template_G, exit, err = kNoMemoryErr, "create json object error!");

    err = kNoErr;

 exit:
    if(err != kNoErr)
    {
        destory_template();
    }

    return err;
}


//2.增加模板
OSStatus add_peripherals_to_template(TEMPLATE_NODE *template_node)
{
    OSStatus err = kUnknownErr;
    json_object *peripherals_member = NULL;

    if(mico_template_G == NULL)
    {
        app_log("mico_template_G is null?");
        err = kGeneralErr;
        goto exit;
    }

    if((template_node->name_len != strlen((const char *)(template_node->name))) || (template_node->uint_len != strlen((const char *)(template_node->uint))) || (template_node->ext_len != strlen((const char *)(template_node->ext))))
    {
        app_log("len error");
        err = kGeneralErr;
        goto exit;
    }

    if(( template_node->name_len > MAX_NAME_LEN) || ( template_node->uint_len > MAX_UNIT_LEN) || ( template_node->ext_len > MAX_EXT_LEN))
    {
        app_log("max len error");
        err = kGeneralErr;
        goto exit;
    }

    peripherals_member = json_object_new_object();
    require_action_string(peripherals_member, exit, err = kNoMemoryErr, "create peripherals_member error!");


    json_object_object_add(peripherals_member, PROTOCOL_G_PID, json_object_new_int(template_node->pid));
    json_object_object_add(peripherals_member, PROTOCOL_G_T, json_object_new_int(template_node->type));
    json_object_object_add(peripherals_member, PROTOCOL_G_E, json_object_new_string((const char *)(template_node->ext)));
    json_object_object_add(peripherals_member, PROTOCOL_G_N, json_object_new_string((const char *)(template_node->name)));

    if((template_node->type >= USER_NODE_TYPE_READ_MIN) && (template_node->type <= USER_NODE_TYPE_READ_MAX)) //只有当node为只读类型时,才有效
    {
        json_object_object_add(peripherals_member, PROTOCOL_G_U, json_object_new_string((const char *)(template_node->uint)));
    }

    if((template_node->type >= USER_NODE_TYPE_PWM_CTRL_MIN) && (template_node->type <= USER_NODE_TYPE_PWM_CTRL_MAX))  //只有当PWM为可读类型时,才有效
    {
        if(template_node->min > template_node->max)
        {
            free_json_obj(&peripherals_member);
            app_log("min is bigger than max");
            err = kGeneralErr;
            goto exit;
        }

        json_object_object_add(peripherals_member, PROTOCOL_G_DMIN, json_object_new_int(template_node->min));
        json_object_object_add(peripherals_member, PROTOCOL_G_DMAX, json_object_new_int(template_node->max));
    }

    json_object_array_add(mico_template_G, peripherals_member);

    err = kNoErr;

 exit:
    if(err != kNoErr)
    {
        destory_template();
    }

    return err;
}


//3.生成最终模板
//接收长度小于最终长度,返回false,则模板依旧保留. 若接收长度大于生成长度,则模板自动销毁,返回true.
OSStatus generate_final_template(char *receice_data, uint32_t max_receice_buff_len)
{
    OSStatus err = kUnknownErr;
    const char *generate_template = NULL;
    uint32_t  generate_template_len = 0;

    require_action_string(receice_data, exit, err = kGeneralErr, "receice_data is NULL");

    if(mico_template == NULL || mico_template_G == NULL)
    {
        err = kGeneralErr;
        app_log("mico_template or  mico_template_G is NULL");
        goto exit;
    }

    json_object_object_add(mico_template, PROTOCOL_G, mico_template_G);

    generate_template = json_object_to_json_string(mico_template);
    require_action_string(generate_template, exit, err = kNoMemoryErr, "create generate_template string error!");

    generate_template_len = strlen(generate_template);
    require_action_string((max_receice_buff_len - 1) > generate_template_len, exit, err = kGeneralErr, "receive len is short");

    memcpy(receice_data, generate_template, generate_template_len);

    err = kNoErr;

 exit:
    destory_template();

    return err;
}

//4.注销模板
void destory_template(void)
{
    free_json_obj(&mico_template);
    free_json_obj(&mico_template_G);
    return;
}


//1.初始化外设数据
OSStatus init_upload_data(uint32_t command_id, uint32_t CMD)
{
    OSStatus err = kUnknownErr;

    if(mico_data != NULL || mico_data_G != NULL)
    {
        destory_upload_data();
    }

    mico_data = json_object_new_object();
    require_action_string(mico_data, exit, err = kNoMemoryErr, "create json object error!");

    json_object_object_add(mico_data, PROTOCOL_COMMAND_ID, json_object_new_int(command_id));
    json_object_object_add(mico_data, PROTOCOL_CMD, json_object_new_int(CMD));

    mico_data_G = json_object_new_array();
    require_action_string(mico_data_G, exit, err = kNoMemoryErr, "create json object error!");

    err = kNoErr;

 exit:
    if(err != kNoErr)
    {
        destory_upload_data();
    }
    return err;

}
//2.增加bool型外设数据
OSStatus add_bool_peripherals_to_upload_data(uint32_t type, uint32_t pid, bool data)
{
    json_object *peripherals_member = NULL;
    OSStatus err = kUnknownErr;


    peripherals_member = json_object_new_object();
    require_action_string(peripherals_member, exit, err = kNoMemoryErr, "create peripherals_member error!");

    json_object_object_add(peripherals_member, PROTOCOL_G_T, json_object_new_int(type));
    json_object_object_add(peripherals_member, PROTOCOL_G_PID, json_object_new_double(pid));
    json_object_object_add(peripherals_member, PROTOCOL_G_D, json_object_new_boolean(data));

    json_object_array_add(mico_data_G, peripherals_member);

    err = kNoErr;

 exit:
    if(err != kNoErr)
    {
        destory_upload_data();
    }
    return err;
}


//3.增加int型外设数据
OSStatus add_int_peripherals_to_upload_data(uint32_t type, uint32_t pid, int32_t data)
{
    json_object *peripherals_member = NULL;
    OSStatus err = kUnknownErr;

    peripherals_member = json_object_new_object();
    require_action_string(peripherals_member, exit, err = kNoMemoryErr, "create peripherals_member error!");

    json_object_object_add(peripherals_member, PROTOCOL_G_T, json_object_new_int(type));
    json_object_object_add(peripherals_member, PROTOCOL_G_PID, json_object_new_double(pid));
    json_object_object_add(peripherals_member, PROTOCOL_G_D, json_object_new_int(data));

    json_object_array_add(mico_data_G, peripherals_member);

    err = kNoErr;
 exit:
    if(err != kNoErr)
    {
        destory_upload_data();
    }
    return err;
}

//4.增加float型外设数据
OSStatus add_float_peripherals_to_upload_data(uint32_t type, uint32_t pid, float data)
{
    json_object *peripherals_member = NULL;
    OSStatus err = kUnknownErr;

    peripherals_member = json_object_new_object();
    require_action_string(peripherals_member, exit, err = kNoMemoryErr, "create peripherals_member error!");

    json_object_object_add(peripherals_member, PROTOCOL_G_T, json_object_new_int(type));
    json_object_object_add(peripherals_member, PROTOCOL_G_PID, json_object_new_double(pid));
    json_object_object_add(peripherals_member, PROTOCOL_G_D, json_object_new_double(data));

    json_object_array_add(mico_data_G, peripherals_member);

    err = kNoErr;

 exit:
    if(err != kNoErr)
    {
        destory_upload_data();
    }
    return err;
}

//5.增加string型外设数据
OSStatus add_string_peripherals_to_upload_data(uint32_t type, uint32_t pid, const char  *data, uint32_t len)
{
    json_object *peripherals_member = NULL;
    OSStatus err = kUnknownErr;

    uint8_t *json_data = NULL;

    if(len > 1024)
    {
        destory_upload_data();
        return kGeneralErr;
    }

    json_data = malloc(len + 1);
    if(json_data == NULL)
    {
        destory_upload_data();
        return kNoMemoryErr;
    }
    memset(json_data, 0, sizeof(len + 1));
    memcpy(json_data, data, len);

    peripherals_member = json_object_new_object();
    require_action_string(peripherals_member, exit, err = kNoMemoryErr, "create peripherals_member error!");

    json_object_object_add(peripherals_member, PROTOCOL_G_T, json_object_new_int(type));
    json_object_object_add(peripherals_member, PROTOCOL_G_PID, json_object_new_double(pid));
    json_object_object_add(peripherals_member, PROTOCOL_G_D, json_object_new_string(data));

    json_object_array_add(mico_data_G, peripherals_member);

    err = kNoErr;

 exit:
    if(err != kNoErr)
    {
        destory_upload_data();
    }
    return err;
}

//6.增加RGB型外设数据
OSStatus add_RGB_LED_peripherals_to_upload_data(uint32_t type, uint32_t pid, bool user_switch, uint32_t h, uint32_t s, uint32_t v)
{
    json_object *peripherals_member = NULL;
    OSStatus err = kUnknownErr;

    peripherals_member = json_object_new_object();
    require_action_string(peripherals_member, exit, err = kNoMemoryErr, "create peripherals_member error!");

    json_object_object_add(peripherals_member, PROTOCOL_G_D_RGB_SWITCH, json_object_new_boolean(user_switch));
    json_object_object_add(peripherals_member, PROTOCOL_G_D_RGB_H, json_object_new_int(h));
    json_object_object_add(peripherals_member, PROTOCOL_G_D_RGB_S, json_object_new_int(s));
    json_object_object_add(peripherals_member, PROTOCOL_G_D_RGB_V, json_object_new_int(v));


    json_object_object_add(peripherals_member, PROTOCOL_G_PID, json_object_new_int(pid));
    json_object_object_add(peripherals_member, PROTOCOL_G_D, peripherals_member);
    json_object_object_add(peripherals_member, PROTOCOL_G_T, json_object_new_int(type));

    json_object_array_add(mico_data_G, peripherals_member);

    err = kNoErr;

 exit:
    if(err != kNoErr)
    {
        destory_upload_data();
    }
    return err;
}

//7.生成外设数据
//接收长度小于最终长度,返回false,则模板依旧保留. 若接收长度大于生成长度,则模板自动销毁,返回true.
OSStatus generate_final_upload_data(char *receice_data, uint32_t max_receice_buff_len)
{
    OSStatus err = kUnknownErr;
    const char *generate_data = NULL;
    uint32_t  generate_data_len = 0;

    require_action_string(receice_data, exit, err = kGeneralErr, "receice_data is NULL");

    if(mico_data == NULL || mico_data_G == NULL)
    {
        err = kGeneralErr;
        app_log("mico_data or  mico_data_G is NULL");
        goto exit;
    }

    json_object_object_add(mico_data, PROTOCOL_G, mico_data_G);

    generate_data = json_object_to_json_string(mico_data);
    require_action_string(generate_data, exit, err = kNoMemoryErr, "create generate_data string error!");

    generate_data_len = strlen(generate_data);
    require_action_string((max_receice_buff_len - 1) > generate_data_len, exit, err = kGeneralErr, "receive len is short");

    memcpy(receice_data, generate_data, generate_data_len);

    err = kNoErr;

 exit:

    destory_upload_data();   //必须注销
    return err;
}

//8.注销外设数据
void destory_upload_data(void)
{
    free_json_obj(&mico_data);
    free_json_obj(&mico_data_G);
    return;
}







