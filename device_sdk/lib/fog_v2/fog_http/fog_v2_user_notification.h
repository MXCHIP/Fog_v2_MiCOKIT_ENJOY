#ifndef __FOG_V2_USER_NOTIFICATION_H_
#define __FOG_V2_USER_NOTIFICATION_H_

#if (FOG_V2_USE_SUB_DEVICE == 1)

extern WEAK void user_fog_v2_device_notification(SUBDEVICE_CMD_TYPE type, const char *s_product_id, const char *s_mac);

#endif //end of  FOG_V2_USE_SUB_DEVICE

#endif


