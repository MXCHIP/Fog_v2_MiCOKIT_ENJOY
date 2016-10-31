#ifndef _USER_COMMON_H_
#define _USER_COMMON_H_

#include "mico.h"

typedef struct _EXT_SENSOR_DATA
{
    uint32_t    command_id;     //下行数据命令号
    uint8_t     temperature;    //温度
    uint8_t     humidity;       //湿度
    uint16_t    light_sensor;   //光线强度
    uint16_t    infrared_reflective;  //红外
}EXT_SENSOR_DATA;


extern void free_json_obj(json_object **json_obj);
extern void micokit_ext_set(void);
extern void micokit_set_oled(uint8_t temperature, uint8_t humidity);

#endif
