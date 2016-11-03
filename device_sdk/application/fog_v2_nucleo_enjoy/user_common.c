#include "MiCOKit_STmems.h"
#include "user_common.h"

void micokit_set_oled(uint8_t temperature, uint8_t humidity);


//扩展板硬件初始化
void micokit_ext_set(void)
{
    micokit_STmems_init();

    dc_motor_init();
    dc_motor_set(0);
}

void micokit_set_oled(uint8_t temperature, uint8_t humidity)
{
    uint8_t oled_buff[16] = {0};
    net_para_st para;

    micoWlanGetIPStatus(&para, Station);

    sprintf((char *)oled_buff, "%s", para.ip);

    OLED_ShowString(OLED_DISPLAY_COLUMN_START, OLED_DISPLAY_ROW_2, (char *)"                ");
    OLED_ShowString(OLED_DISPLAY_COLUMN_START, OLED_DISPLAY_ROW_2, (char *)"FOG V2          ");

    OLED_ShowString(OLED_DISPLAY_COLUMN_START, OLED_DISPLAY_ROW_3, (char *)"                ");
    OLED_ShowString(OLED_DISPLAY_COLUMN_START, OLED_DISPLAY_ROW_3, (char *)oled_buff);

    memset(oled_buff, 0, sizeof(oled_buff));
    sprintf((char *)oled_buff, "T:%d C  H:%d %%", temperature, humidity);

    OLED_ShowString(OLED_DISPLAY_COLUMN_START, OLED_DISPLAY_ROW_4, (char *)"                ");
    OLED_ShowString(OLED_DISPLAY_COLUMN_START, OLED_DISPLAY_ROW_4, (char *)oled_buff);
}


//释放JSON对象
void free_json_obj(json_object **json_obj)
{
    if(*json_obj != NULL)
    {
        json_object_put(*json_obj);          // free memory of json object
        *json_obj = NULL;
    }

    return;
}
