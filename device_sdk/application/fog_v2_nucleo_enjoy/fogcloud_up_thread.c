#include "mico.h"
#include "fogcloud_up_thread.h"
#include "fog_v2_config.h"
#include "json_c/json_object.h"
#include "json_c/json.h"
#include "MiCOKit_STmems.h"
#include "sensor\DHT11\DHT11.h"
#include "user_common.h"
#include "template_analysis.h"
#include "user_template.h"


#define fogcloud_up_log(M, ...) custom_log("FOG_UP_DATA", M, ##__VA_ARGS__)


EXT_SENSOR_DATA     ext_sensor_data;

void user_upstream_thread(mico_thread_arg_t args);


//外设初始化
static OSStatus ext_module_init(void)
{
    OSStatus err = kNoErr;

    //初始化外部传感器
    err = hts221_sensor_init();             //温湿度传感器初始化
    require_noerr_action( err, exit, fogcloud_up_log("ERROR: Unable to Init DHT11") );

    //红外线 检测光线强度
    light_sensor_init();

    //姿态传感器初始化
    err = lsm9ds1_acc_gyr_sensor_init();
    require_noerr_action( err, exit, fogcloud_up_log("ERROR: Unable to Init lsm9ds1") );

    //初始化电机
    err = dc_motor_init();
    require_noerr_action( err, exit, fogcloud_up_log("ERROR: Unable to Init DC motor") );
    dc_motor_set(0);

    //初始化rgb_led
    hsb2rgb_led_init();
//    hsb2rgb_led_open(240, 100, 100);

    fogcloud_up_log("module init success");

 exit:
    return err;
}

OSStatus ext_moudule_read(void)
{
    OSStatus err = kUnknownErr;

    memset(&ext_sensor_data, 0, sizeof(ext_sensor_data));

    //读取温湿度
    err = hts221_Read_Data(&(ext_sensor_data.temperature), &(ext_sensor_data.humidity));
    require_noerr_action( err, exit, fogcloud_up_log("ERROR: hts221_Read_Data()") );
    //fogcloud_up_log("DHT11  T: %3.1fC  H: %3.1f%%", (float)ext_sensor_data.temperature, (float)ext_sensor_data.humidity);

    light_sensor_read(&(ext_sensor_data.infrared_reflective));

    //读取加速度
    err = lsm9ds1_acc_read_data(&ext_sensor_data.ACC_X, &ext_sensor_data.ACC_Y, &ext_sensor_data.ACC_Z);
    require_noerr_action( err, exit, fogcloud_up_log("ERROR: lsm9ds1_acc_read_data()") );

    //读取角速度
    err = lsm9ds1_gyr_read_data(&ext_sensor_data.GYR_X, &ext_sensor_data.GYR_Y, &ext_sensor_data.GYR_Z);
    require_noerr_action( err, exit, fogcloud_up_log("ERROR: lsm9ds1_acc_read_data()") );


    micokit_set_oled((uint8_t)(ext_sensor_data.temperature), (uint8_t)(ext_sensor_data.humidity));

 exit:
    return err;
}

OSStatus ext_moudule_upload(void)
{
    //static bool motor_state = false;
    char *upload_data = NULL;
    OSStatus err = kUnknownErr;
    uint32_t upload_data_len = 1024;

    upload_data = malloc(upload_data_len);
    if(upload_data == NULL)
    {
        return kNoMemoryErr;
    }
    memset(upload_data, 0, upload_data_len);


    //初始化上传数据模板
    err = init_upload_data(1234, D2C_SEND_PACKETS);
    require_noerr(err, exit);

    //添加温度数据
    err = add_float_peripherals_to_upload_data(USER_NODE_TYPE_TEMPERATURE ,USER_NODE_PID_TEMPERATURE, ext_sensor_data.temperature);
    require_noerr(err, exit);

    //添加湿度数据
    err = add_float_peripherals_to_upload_data(USER_NODE_TYPE_HUMIDITY, USER_NODE_PID_HUMIDITY, ext_sensor_data.humidity);
    require_noerr(err, exit);

    //添加红外线数据
    err = add_int_peripherals_to_upload_data(USER_NODE_TYPE_LIGHT_INTENSITY, USER_NODE_PID_LIGHT_INTENSITY, ext_sensor_data.infrared_reflective);
    require_noerr(err, exit);

    //添加加速度
    err = add_int_peripherals_to_upload_data(USER_NODE_TYPE_ACC_ANG, USER_NODE_PID_ACC_X, ext_sensor_data.ACC_X);
    require_noerr(err, exit);

    err = add_int_peripherals_to_upload_data(USER_NODE_TYPE_ACC_ANG, USER_NODE_PID_ACC_Y, ext_sensor_data.ACC_Y);
    require_noerr(err, exit);

    err = add_int_peripherals_to_upload_data(USER_NODE_TYPE_ACC_ANG, USER_NODE_PID_ACC_Z, ext_sensor_data.ACC_Z);
    require_noerr(err, exit);

    //添加角速度
    err = add_int_peripherals_to_upload_data(USER_NODE_TYPE_ACC_ANG, USER_NODE_PID_ANG_X, ext_sensor_data.GYR_X);
    require_noerr(err, exit);

    err = add_int_peripherals_to_upload_data(USER_NODE_TYPE_ACC_ANG, USER_NODE_PID_ANG_Y, ext_sensor_data.GYR_Y);
    require_noerr(err, exit);

    err = add_int_peripherals_to_upload_data(USER_NODE_TYPE_ACC_ANG, USER_NODE_PID_ANG_Z, ext_sensor_data.GYR_Z);
    require_noerr(err, exit);

    err = generate_final_upload_data(upload_data, upload_data_len);
    require_noerr(err, exit);

    fogcloud_up_log("[%d]%s", strlen(upload_data), upload_data);

    fog_v2_device_send_event(upload_data, FOG_V2_SEND_EVENT_RULES_PUBLISH | FOG_V2_SEND_EVENT_RULES_DATEBASE);

 exit:
    if(upload_data != NULL)  //回收资源
    {
        free(upload_data);
        upload_data = NULL;
    }

    destory_upload_data(); //回收资源
    return err;
}

void user_upstream_thread(mico_thread_arg_t args)
{
	OSStatus err = kUnknownErr;

	fogcloud_up_log("------------upstream_thread start------------");

    err = ext_module_init();      //外设初始化
    require_noerr(err, exit);

	while(1)
	{
        mico_thread_msleep(1300);

        system_log("num_of_chunks:%d,allocted_memory:%d, free:%d, total_memory:%d", MicoGetMemoryInfo()->num_of_chunks, MicoGetMemoryInfo()->allocted_memory, MicoGetMemoryInfo()->free_memory, MicoGetMemoryInfo()->total_memory);

        if(fog_v2_is_have_superuser() == true)
        {
            if(ext_moudule_read() == kNoErr)
            {
                ext_moudule_upload();
            }
        }
	}

 exit:
	mico_rtos_delete_thread(NULL);  // delete current thread

	return;
}
