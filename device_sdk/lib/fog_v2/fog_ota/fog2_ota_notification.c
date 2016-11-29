#include "mico.h"
#include "fog2_ota_notification.h"

#define app_log(M, ...)                     custom_log("FOG_V2_OTA_NOTIFICATION", M, ##__VA_ARGS__)

WEAK void user_fog_v2_ota_notification(FOG2_OTA_EVENT_TYPE ota_event_type);

WEAK void user_fog_v2_ota_notification(FOG2_OTA_EVENT_TYPE ota_event_type)
{
    switch(ota_event_type)
    {
        case FOG2_OTA_CHECK_FAILURE:
            app_log("OTA EVENT: FOG2_OTA_CHECK_FAILURE");
            break;

        case FOG2_OTA_NO_NEW_VERSION:
            app_log("OTA EVENT: FOG2_OTA_NO_NEW_VERSION");
            break;

        case FOG2_OTA_IN_UPDATE:
            app_log("OTA EVENT: FOG2_OTA_IN_UPDATE");
            break;

        case FOG2_OTA_UPDATE_SUCCESS:
            app_log("OTA EVENT: FOG2_OTA_UPDATE_SUCCESS");
            break;

        case FOG2_OTA_UPDATE_FAILURE:
            app_log("OTA EVENT: FOG2_OTA_UPDATE_FAILURE");
            break;
    }

    return;
}
