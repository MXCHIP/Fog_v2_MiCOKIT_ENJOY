#ifndef _USER_COMMON_H_
#define _USER_COMMON_H_

#include "mico.h"

typedef struct _EXT_SENSOR_DATA
{
    uint32_t    command_id;         //下行数据命令号
    float       temperature;        //温度
    float       humidity;           //湿度
    uint16_t    infrared_reflective;             //紫外线
    int16_t     ACC_X;
    int16_t     ACC_Y;
    int16_t     ACC_Z;
    int16_t     GYR_X;
    int16_t     GYR_Y;
    int16_t     GYR_Z;
}EXT_SENSOR_DATA;


extern void free_json_obj(json_object **json_obj);
extern void micokit_ext_set(void);
extern void micokit_set_oled(uint8_t temperature, uint8_t humidity);

#endif
