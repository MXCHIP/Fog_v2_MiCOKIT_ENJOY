#ifndef _USER_TEMPLATE_H_
#define _USER_TEMPLATE_H_


//外设PID定义   2001~3000
enum USER_NODE_PID {
    USER_NODE_PID_TEMPERATURE = 2001,
    USER_NODE_PID_HUMIDITY,
    USER_NODE_PID_LIGHT_INTENSITY,
    USER_NODE_PID_ACC_X,
    USER_NODE_PID_ACC_Y,
    USER_NODE_PID_ACC_Z,
    USER_NODE_PID_ANG_X,
    USER_NODE_PID_ANG_Y,
    USER_NODE_PID_ANG_Z,
    USER_NODE_PID_MOTOR,
    USER_NODE_PID_RGB_LED
};

extern OSStatus sys_generate_user_template(char *user_template, uint32_t template_len);
extern void sys_process_private_node(uint32_t pid, uint32_t type, struct json_object* D_json_obj);
extern void sys_process_pwm_node(uint32_t pid, uint32_t type, int32_t data_int);
extern void sys_process_bool_node(uint32_t pid, uint32_t type, bool data_bool);


#endif
