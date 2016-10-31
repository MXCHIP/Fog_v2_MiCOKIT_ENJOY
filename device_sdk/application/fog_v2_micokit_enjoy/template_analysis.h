#ifndef _TEMPLATE_ANALYSIS_H
#define _TEMPLATE_ANALYSIS_H

#define PROTOCOL_COMMAND_ID                         ("command_id")
#define PROTOCOL_CMD                                ("CMD")
#define PROTOCOL_G                                  ("G")
#define PROTOCOL_G_PID                              ("PID")
#define PROTOCOL_G_D                                ("D")
#define PROTOCOL_G_T                                ("T")
#define PROTOCOL_G_N                                ("N")
#define PROTOCOL_G_E                                ("E")
#define PROTOCOL_G_U                                ("U")
#define PROTOCOL_G_DMIN                             ("D_MIN")
#define PROTOCOL_G_DMAX                             ("D_MAX")

#define PROTOCOL_G_D_RGB_SWITCH                     ("switch")
#define PROTOCOL_G_D_RGB_H                          ("h")
#define PROTOCOL_G_D_RGB_S                          ("s")
#define PROTOCOL_G_D_RGB_V                          ("v")


//CMD定义
#define C2D_GET_TEMPLATE                            (100)
#define C2D_SEND_COMMANDS                           (101)
#define D2C_ACK_TEMPLATE                            (200)
#define D2C_SEND_PACKETS                            (201)


//类型的最大值和最小值定义
#define USER_NODE_TYPE_VALUE_MIN                    (20)
#define USER_NODE_TYPE_VALUE_MAX                    (100)
#define USER_NODE_TYPE_READ_MIN                     (21)
#define USER_NODE_TYPE_READ_MAX                     (39)
#define USER_NODE_TYPE_BOOL_CTRL_MIN                (41)
#define USER_NODE_TYPE_BOOL_CTRL_MAX                (59)
#define USER_NODE_TYPE_PWM_CTRL_MIN                 (61)
#define USER_NODE_TYPE_PWM_CTRL_MAX                 (79)
#define USER_NODE_TYPE_PRIVATE_MIN                  (81)
#define USER_NODE_TYPE_PRIVATE_MAX                  (99)

//21到39为只读类型
#define USER_NODE_TYPE_TEMPERATURE                  (21)
#define USER_NODE_TYPE_HUMIDITY                     (22)
#define USER_NODE_TYPE_LIGHT_INTENSITY              (23)
#define USER_NODE_TYPE_INFRARED                     (24)

//41-59 开关型
#define USER_NODE_TYPE_MOTOR                        (41)
#define USER_NODE_TYPE_LED                          (42)

//61-79为 PWM控制类型
#define USER_NODE_TYPE_PWM_MOTOR                    (61)

//81-99为 定制型
#define USER_NODE_TYPE_RGB_LED                      (81)


#define MAX_NAME_LEN                                (30)
#define MAX_UNIT_LEN                                (30)
#define MAX_EXT_LEN                                 (60)
#define USER_TEMPLATE_LEN                           (2048)    //用户模板最大为2K长度

#define MAX_NODE_NUMBER                             (20)

typedef struct _TEMPLATE_NODE
{
    uint32_t type; //T
    uint32_t pid;  //PID
    int32_t min;  //MIN
    int32_t max;  //MAX
    const char *uint; //U
    const char *name; //N
    const char *ext;  //E
    uint32_t uint_len;
    uint32_t name_len;
    uint32_t ext_len;
}TEMPLATE_NODE;

typedef void (*USER_BOOL_TYPE_CALLBACK)(uint32_t pid, bool data_bool);
typedef void (*USER_PWM_TYPE_CALLBACK)(uint32_t pid, int32_t pwm_data);
typedef void (*USER_PRIVATE_CALLBACK)(uint32_t pid, struct json_object* D_json_obj);

typedef struct _PLATFORM_APP_TEMPLATE_G
{
    uint32_t type; //T
    uint32_t pid;  //PID
    int32_t min;  //MIN
    int32_t max;  //MAX
    const char *uint; //U
    const char *name; //N
    const char *ext;  //E
    void *receive_callback;
}PLATFORM_APP_TEMPLATE_G;


extern OSStatus init_template(uint32_t command_id, uint32_t CMD);
extern OSStatus add_peripherals_to_template(TEMPLATE_NODE *template_node);
extern OSStatus generate_final_template(char *receice_data, uint32_t max_receice_buff_len);
extern void destory_template(void);
extern OSStatus set_node_attr(TEMPLATE_NODE *node, uint32_t type, uint32_t pid, int32_t min, int32_t max, const char *uint, const char *name, const char *ext);

extern OSStatus init_upload_data(uint32_t command_id, uint32_t CMD);
extern OSStatus add_bool_peripherals_to_upload_data(uint32_t type, uint32_t pid, bool data);
extern OSStatus add_int_peripherals_to_upload_data(uint32_t type, uint32_t pid, int32_t data);
extern OSStatus add_float_peripherals_to_upload_data(uint32_t type, uint32_t pid, float data);
extern OSStatus add_string_peripherals_to_upload_data(uint32_t type, uint32_t pid, const char  *data, uint32_t len);
extern OSStatus add_RGB_LED_peripherals_to_upload_data(uint32_t type, uint32_t pid, bool user_switch, uint32_t h, uint32_t s, uint32_t v);
extern OSStatus generate_final_upload_data(char *receice_data, uint32_t max_receice_buff_len);
extern void destory_upload_data(void);

#endif