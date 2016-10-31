//--------------------------------------------------
/*
CMD:帧标识
	100-APP端向设备端请求数据模板
	101-APP端向设备端发送数据包

	200-设备端向APP发送数据模板
	201-设备端向APP发送的数据包
*/
const C2D_GET_TEMPLATE 	= 100; 
const C2D_SEND_COMMANDS = 101;
const D2C_ACK_TEMPLATE 	= 200;
const D2C_SEND_PACKETS 	= 201;
//---------------------------------------------------
/*
//21到39为只读类型
21---温度 		--- 	数据显示+单位显示
22---湿度 		--- 	数据显示+单位显示
23---光照强度 	--- 	数据显示+单位显示
24---距离 		--- 	数据显示+单位显示
25---通用型 		--- 	数据显示+单位显示
26---加速度角速度	--- 	数据显示+单位显示

//41-59 开关型
41---电机 		---		开关显示

//61-79为 PWM控制类型
61---PWM电机 		--- 	打开新页面,滑块显示(模板的G中包含特殊部分D_MIN和D_MAX)


//81往后属于定制类型
81---RGB灯 		--- 	打开新页面,状态和命令的数据格式自定义
*/
const _TYPE_READ_MIN = 21;
const _TYPE_READ_MAX = 39;
const _TYPE_BOOL_CTRL_MIN = 41;
const _TYPE_BOOL_CTRL_MAX = 59;
const _TYPE_PWM_CTRL_MIN = 61;
const _TYPE_PWM_CTRL_MAX = 79;
const _TYPE_PRIVATE_MIN = 81;
const _TYPE_PRIVATE_MAX = 99;


//---------------------------------------------------
//21到39为只读类型
const _MICO_TEMPLATE_TYPE_TEMPERATURE = 21; //温度
const _MICO_TEMPLATE_TYPE_HUMIDITY = 22; //湿度
const _MICO_TEMPLATE_TYPE_LIGHT_INTENSITY = 23; //光照强度
const _MICO_TEMPLATE_TYPE_INFRARED = 24; //红外
const _MICO_TEMPLATE_TYPE_ACC_ANG = 25; //加速度角速度图标
const _MICO_TEMPLATE_TYPE_IOT_COMMON = 26; //通用图标

//41-59 开关型
const _MICO_TEMPLATE_TYPE_MOTOR = 41; //电机 开关型
const _MICO_TEMPLATE_TYPE_LED = 42; //LED 开关型

//61-79为 PWM控制类型
const _MICO_TEMPLATE_TYPE_PWM_MOTOR = 61; //PWM电机

//81往后属于定制类型
const _MICO_TEMPLATE_TYPE_RGB_LED = 81; //RGB_LED

//--------------------------------------------------
const _TYPE_21_ICON = "mui-icon device_iconfont icon-wendu";
const _TYPE_22_ICON = "mui-icon device_iconfont icon-shidu";
const _TYPE_23_ICON = "mui-icon device_iconfont icon-sensor-guangzhao";
const _TYPE_24_ICON = "mui-icon device_iconfont icon-hongwai";
const _TYPE_25_ICON = "mui-icon device_iconfont icon-acc_ang";
const _TYPE_26_ICON = "mui-icon device_iconfont icon-iot_common";

const _TYPE_41_ICON = "mui-icon device_iconfont icon-dianji";
const _TYPE_42_ICON = "mui-icon device_iconfont icon-led3"; 

const _TYPE_61_ICON = "mui-icon device_iconfont icon-pwm"; 

const _TYPE_81_ICON = "mui-icon device_iconfont icon-rgbled";

//----------------------------------------------------------
var type_to_icon_table = new Array();

type_to_icon_table[_MICO_TEMPLATE_TYPE_TEMPERATURE] = _TYPE_21_ICON;
type_to_icon_table[_MICO_TEMPLATE_TYPE_HUMIDITY] = _TYPE_22_ICON;
type_to_icon_table[_MICO_TEMPLATE_TYPE_LIGHT_INTENSITY] = _TYPE_23_ICON;
type_to_icon_table[_MICO_TEMPLATE_TYPE_INFRARED] = _TYPE_24_ICON;
type_to_icon_table[_MICO_TEMPLATE_TYPE_ACC_ANG] = _TYPE_25_ICON;
type_to_icon_table[_MICO_TEMPLATE_TYPE_IOT_COMMON] = _TYPE_26_ICON;

type_to_icon_table[_MICO_TEMPLATE_TYPE_MOTOR] = _TYPE_41_ICON;
type_to_icon_table[_MICO_TEMPLATE_TYPE_LED] = _TYPE_42_ICON;

type_to_icon_table[_MICO_TEMPLATE_TYPE_PWM_MOTOR] = _TYPE_61_ICON;

type_to_icon_table[_MICO_TEMPLATE_TYPE_RGB_LED] = _TYPE_81_ICON;
