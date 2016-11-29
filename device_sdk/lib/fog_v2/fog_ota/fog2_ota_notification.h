#ifndef _FOG2_OTA_NOTIFICATION_H_
#define _FOG2_OTA_NOTIFICATION_H_

typedef enum {
    FOG2_OTA_CHECK_FAILURE = 1,             //检查OTA失败
    FOG2_OTA_NO_NEW_VERSION = 2,            //检查发现无新版本
    FOG2_OTA_IN_UPDATE = 3,                 //正在OTA更新
    FOG2_OTA_UPDATE_SUCCESS = 4,            //更新成功
    FOG2_OTA_UPDATE_FAILURE = 5             //更新失败
} FOG2_OTA_EVENT_TYPE;

extern WEAK void user_fog_v2_ota_notification(FOG2_OTA_EVENT_TYPE ota_event_type);

#endif
