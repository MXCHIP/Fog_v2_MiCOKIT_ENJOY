# FOG2.0 设备端SDK使用说明

`设备端SDK`   `MiCOSDK中间件`

---

>FOG2.0设备端SDK是一个依托于MiCOSDK的中间件,完成和FOG2.0服务器后台的交互,屏蔽底层交互细节，为应用层提供出简单好用的API接口，为设备端的快速开发提供保障。更多详细资料请参考MiCO WIKI资料库：[FogCloud 设备端 SDK][1]

---


## 1. 依赖 
MiCOSDK：3.1及其以后版本 
[MICOSDK下载路径][2]

由于MICOSDK3.2版本之后有些接口发生了变化，在中间件中的lib/fog_v2/mico_v2_config.h中增加了ADAPT_MICO_SDK_VSERSION宏定义来适配不同版本的MICOSDK

ADAPT_MICO_SDK_VSERSION | 适配MICOSDK版本
---|---
1|MICOSDK3.1、MICOSDK3.1.1
2|MICOSDK3.2

## 2. 目录结构 
目录 | 内容
--- | ---
lib/mqtt-c | mqtt客户端源码(版本v0.1.9)，**MICOSDK3.2版本之后已经包含了此文件夹**
lib/fog_v2 | 与FOGCLOUD V2后台的交互逻辑(版本v2.6.0)
application/fog_v2_micokit_enjoy | 为庆科的MiCOKIT开发板制作的应用程序,支持硬件`MiCOkit-3165`、`MiCOkit-3166`、`MiCOkit-3239`、`MiCOkit-3031`
application/fog_v2_nucleo_enjoy  | 为庆科的MiCOKIT开发板制作的应用程序,支持硬件`MiCOkit-NUCEOF411`
application/fog_v2_api_test | 测试fog_v2底层文件夹提供的API简单demo

>**注意：fog_v2和mqtt-c文件夹的内容是一个依赖于MiCOSDK的中间件，为上层应用提供基础API支撑，fog_v2_micokit_enjoy和fog_v2_nucleo_enjoy为依托于具体硬件设备的应用案例**

## 3. 使用步骤 
### 3.1 将FOG2.0SDK中间件加入到MiCoder 
将fog_v2和mqtt-c文件夹复制到MiCOSDK工程目录MiCO SDK/libraries/protocols/中
根据当前你的硬件平台,将application文件夹里面的应用程序demo复制到MiCOSDK工程目录MiCO_SDK/demos/application中

### 3.2 在FogCloud后台创建新产品 
在FogCloud后台创建一个新的产品,得到新的产品ID。将lib/fog_v2文件夹中的`fog_v2_config.h`中的` FOG_V2_PRODUCT_ID `宏定义替换成云端创建得到的产品ID。

### 3.3 在MiCoder工程中输入创建Target 
当基于MiCOKit开发板调试时（以MiCOKit-3165为例）：
如果使用Jlink仿真器， 命令： ` application.xxxxxx@MK3165 download total run `  
如果使用Stlink仿真器，命令： ` application.xxxxxx@MK3165 download JTAG=stlink-v2 total run `

当基于Nucleo开发板调试时：
当使用Stlink仿真器时， 命令： ` application.xxxxxx@NUCLEO_F411RE download JTAG=stlink-v2-1 run `


## 4. Fog SDK中间件API说明 
###  4.1 普通设备API接口如下： 
```
//-------用户调用接口-------
//功能：初始化fog服务
//参数：无
//返回值：kNoErr为成功 其他为失败
OSStatus init_fog_v2_service(void);

//功能：开启fog的服务
//参数：无
//返回值：kNoErr为成功 其他为失败
OSStatus start_fog_v2_service(void);

//功能：往云端发送数据
//参数： payload - 要往云端发送的数据，该指针指向的数据只能为json格式
//参数： flag - 发送方式
//下面三个宏定义组合,采用或组合的方式
//FOG_V2_SEND_EVENT_RULES_PUBLISH  向设备的topic去publish数据
//FOG_V2_SEND_EVENT_RULES_DATEBASE 将此次的payload数据存入数据库
//FOG_V2_SEND_EVENT_RULES_PRODUCT  向设备对应产品的topic去publish数据(数据推送给厂商)
//返回值：kNoErr为成功 其他值为失败
OSStatus fog_v2_device_send_event(const char *payload, uint32_t flag);

//功能：从云端接收数据
//参数： payload - 接收数据缓冲区地址
//参数： payload_len - 接收数据缓冲区地址的长度
//参数： timeout - 接收数据等待时间
//返回值：kNoErr为成功 其他值为失败
OSStatus fog_v2_device_recv_command(char *payload, uint32_t payload_len, uint32_t timeout);

#if (FOG_V2_USER_FLASH_PARAM == 1)
//功能：将用户数据写入到中间件的用户flash参数区
//参数：user_data-用户数据指针
//参数：offset-偏移量
//参数：user_data_len-用户数据长度
//返回值：kNoErr-成功,其他-失败
OSStatus fog_v2_write_user_param(const uint8_t *user_data, uint32_t user_data_len, uint32_t offset);

//功能：将中间件的用户flash参数区读出
//参数：user_data-用户数据指针
//参数：offset-偏移量
//参数：user_data_len-用户数据长度
//返回值：kNoErr-成功,其他-失败
OSStatus fog_v2_read_user_param(uint8_t *user_data, uint32_t user_data_len, uint32_t offset);
#endif

//功能：设备端设置回收标志位 若调用该接口，设备重启后联网会自动向云端发起设备回收请求
//参数：无
//返回值：true - 设置成功，flase - 设置失败
bool fog_v2_set_device_recovery_flag(void);

//功能：查询本设备是否有超级用户（被绑定)
//参数：无
//返回值：true - 有超级用户，flase - 无超级用户
bool fog_v2_is_have_superuser(void);

//功能：OTA状态通知
//参数：type - 事件类型
//事件类型一共有五种
//FOG2_OTA_CHECK_FAILURE 检查OTA失败
//FOG2_OTA_NO_NEW_VERSION 检查发现无新版本
//FOG2_OTA_IN_UPDATE 正在OTA更新
//FOG2_OTA_UPDATE_SUCCESS 更新成功
//FOG2_OTA_UPDATE_FAILURE 更新失败
//返回值：无
//USED void user_fog_v2_ota_notification(FOG2_OTA_EVENT_TYPE ota_event_type)
//{
//    //user todo....
//}

//-------子设备相关接口-------
//注意：要开启网关子设备API,则需要在应用程序config/fog_v2_config.h中 FOG_V2_USE_SUB_DEVICE 宏定义更改为
//#define FOG_V2_USE_SUB_DEVICE  (1)才能生效。

//功能：添加一个子设备
//参数： s_product_id - 子设备产品ID
//参数： s_mac - 子设备MAC地址
//返回值：kNoErr为成功 其他值为失败
OSStatus fog_v2_add_subdevice( const char *s_product_id, const char *s_mac, bool auto_set_online);

//功能：删除一个子设备
//参数： s_product_id - 子设备产品ID
//参数： s_mac - 子设备MAC地址
//返回值：kNoErr为成功 其他值为失败
OSStatus fog_v2_remove_subdevice( const char *s_product_id, const char *s_mac );

//功能：设置子设备在线离线状态
//参数： s_product_id - 子设备产品ID
//参数： s_mac - 子设备MAC地址
//参数：online - 子设备是否在线
//返回值：kNoErr为成功 其他值为失败
OSStatus fog_v2_set_subdevice_status(const char *s_product_id, const char *s_mac, bool online);

//功能：设备端从服务器获取时间
//参数：recv_p - 接收缓冲区地址
//参数：recv_len - 接收缓冲区长度,长度需要大于512 Byte
//返回值：kNoErr为成功 其他为失败
OSStatus fog_v2_device_get_server_time(char *recv_p, uint32_t recv_len);

//功能：子设备发送数据
//参数： s_product_id - 子设备产品ID
//参数： s_mac - 子设备MAC地址
//参数： flag - 发送方式
//下面两个宏定义组合,采用或组合的方式
//FOG_V2_SEND_EVENT_RULES_PUBLISH  向设备的topic去publish数据
//FOG_V2_SEND_EVENT_RULES_DATEBASE 将此次的payload数据存入数据库
//返回值：kNoErr为成功 其他值为失败
OSStatus fog_v2_subdevice_send(const char *s_product_id, const char *s_mac, const char *payload, uint32_t flag);

//功能：子设备接收数据
//参数： s_product_id - 子设备产品ID
//参数： s_mac - 子设备MAC地址
//参数： payload - 接收数据缓冲区地址
//参数： payload_len - 接收数据缓冲区地址的长度
//参数： timeout - 接收数据的超时时间
//返回值：kNoErr-成功  kDeletedErr-该设备已被删除  kGeneralErr-超时
OSStatus fog_v2_subdevice_recv(const char *s_product_id, const char *s_mac, char *payload, uint32_t payload_len, uint32_t timeout);

//功能：子设备接收数据
//参数： user_callbck - 用户回调
//返回值：kNoErr-成功 kGeneralErr-失败
OSStatus fog_v2_get_sub_device_all_available(FOG_V2_SUBDEVICE_AVAILABLE_CB user_callbck);

//功能：获取子设备的数量信息
//参数： tatal - 底层定义的子设备总数
//参数： used - 已经使用的子设备总数
//返回值：kNoErr-成功 kGeneralErr-失败
OSStatus fog_v2_get_subdevice_num_info(uint32_t *tatal, uint32_t *used);

//功能：添加子设备超时
//参数： s_product_id - 子设备产品ID
//返回值：kNoErr-成功 kGeneralErr-失败
OSStatus fog_v2_subdevice_add_timeout(const char *s_product_id);

//功能：设备状态改变通知（该函数需要用户在自己的代码中复写函数体，中间件是使用WEAK定义调用该函数）
//参数：type - 事件类型
//事件类型一共有三种
//MQTT_CMD_GATEWAY_UNBIND   网关设备被解绑(s_product_id和s_mac无效)
//MQTT_CMD_GATEWAY_BIND     网关设备被绑定(s_product_id和s_mac无效)
//MQTT_CMD_SUB_UNBIND       子设备被解绑(s_product_id和s_mac有效)
//参数： s_product_id - 子设备产品ID
//参数： s_mac - 子设备MAC地址
//返回值：无
//USED void user_fog_v2_device_notification(SUBDEVICE_CMD_TYPE type, const char *s_product_id, const char *s_mac)
//{
//    //user todo....
//}

//功能：APP通知网关添加子设备通知
//参数： s_product_id - 子设备产品ID
//参数： timeout - APP设置的添加超时时间,单位为秒
//返回值：无
//USED void user_fog_v2_subdevice_add(const char *s_product_id, uint32_t timeout)
//{
//    //user todo....
//}
```

## 5.设备端SDK注意事项

### 5.1 设备OTA 
 FOG2.0平台提供设备OTA的功能，在设备端SDK中间件中已经包含了OTA流程。
 步骤：
 (1)将中间件中fog_v2_config.h里的FOG_V2_REPORT_VER_NUM定义变为更大的数值。
 (2)将生成的bin文件的的文件名改为`AAA@BBB.bin`，`AAA`为fog_v2_config.h中的**FOG_V2_REPORT_VER**，`BBB`为fog_v2_config.h中新的**FOG_V2_REPORT_VER_NUM**。
 **(3)保证固件中的宏定义和上传到fog2.0网站上面的OTA文件名保持一致，否则会导致更新失败！**
 
### 5.2 Mdns通知
设备端在连接上路由器后会开启自身的Mdns服务，供App端搜索到设备。
Mdns通知的service name是`_easylink._tcp.local.`

![2.PNG-75.4kB][3]

例如在IOS上使用Discovery bonjour软件抓取到的设备端信息：
```
MiCOKit 3165#077AC0._easylink._tcp.local. 
MiCOKit\0323165#077AC0.local:8002 
10.0.0.8:8002 

Firmware Rev = FOG_V2_EMW3165@001 
FogProductId = 84756a84-4f1c-11e6-9d95-00163e103941 
IsHaveSuperUser = true 
MAC = D0:BA:E4:07:7A:C0 
MICO OS Rev = 31621002.050 
Model = MiCOKit-3165 
Protocol = fog2.4.0 
``` 


字段|说明
---|---
`Firmware Rev` | 该设备的固件版本
`FogProductId` | 该设备产品ID
`IsHaveSuperUser` | 该设备是否有超级用户，有`true`, `flase`, `UNCHECK`三种状态
`MICO OS Rev` | 该设备的MICO SDK内核版本
`Model` | 该设备的硬件型号
`Protocol` | 该设备使用的Fog SDK中间件版本


APP端的SDK集成了设备发现服务，可以调用设备发现的API获取到设备端的详细信息。

### 5.3 设备绑定
设备端在WIFI station模式下开启了一个TCP SERVER，端口默认为`8002`。
APP端发起绑定操作时连接设备端的TCP SERVER，发送相应指令给设备端，设备端去云端取到一个验证码再返还给APP，最后APP端用得到的验证码去云端发起绑定请求。

>**注意：设备端绑定逻辑已经在SDK中实现，用户无需手动实现。**

### 5.4 设备解绑
如果APP的超级用户删除了设备，设备端如果在线，则会收到通知消息，然后改变自身Mdns通知的`IsHaveSuperUser`字段信息为`false`。

### 5.5 设备配网
如果设备端收到配网信息，则设备端会改变自身Mnds通知的`IsHaveSuperUser`字段信息为`UNCHECK`,待去云端查询到设备是否有超级用户后会改变自身Mnds通知的`IsHaveSuperUser`字段信息为`true`或`false`。

>**注意：如果APP发现设备列表里面的`IsHaveSuperUser`字段为`UNCHECK`的时候，就因该停止配网，减少路由器的配网负载压力。待设备端到云端查询完时候有超级用户后会自动更新`IsHaveSuperUser`字段信息，APP端再做是否绑定处理。**

### 5.6 普通设备API使用说明
在调用init_fog_v2_service()和start_fog_v2_service()接口返回正确之后，即可调用设备发送数据接口fog_v2_device_send_event()和设备接收数据接口fog_v2_device_recv_command()。

### 5.7 子设备API注意事项
在某些应用场景中，有些设备无法直接上网和服务器进行数据交互，但是用户又想让该设备成为独立的设备。为了解决该问题，我们让能上网的设备作为WIFI网关，其他不能上网的设备作为网关的子设备来处理。
WIFI网关为子设备提供相应增加子设备、删除子设备、设置在线、设置离线、发送消息、接收消息等API，来实现子设备间接上网的功能。

#### 5.7.1 创建产品注意事项
在v2.fogcloud.io上面创建产品时，如果需要将`子设备`的产品类型选择为**网关子设备**，要将`父设备`产品类型设置为**网关父设备**。否则调用相关接口不会成功。

#### 5.7.2 接口调用说明
增加子设备成功后，即可设置子设备在线离线状态和对子设备收发消息。

#### 5.7.3 子设备在线离线
如果父设备是离线的，则app端查询时，子设备一定为离线（此功能云端尚在开发中）。
如果父设备是在线的，在子设备在线离线状态为用户调用子设备接口设置的状态。
在调用fog_v2_add_subdevice()时，可以通过`auto_set_online`参数来选择是否让接口内部设置子设备在线


  [1]: http://mico.io/wiki/download
  [2]: http://mico.io/wiki/download
  [3]: http://static.zybuluo.com/1113240207/1rbfag1e3ff15e5skivjnc2h/2.PNG