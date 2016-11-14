#ifndef _FOG_OTA_H_
#define _FOG_OTA_H_

#define OTA_SEND_TIME_OUT           (3000)
#define OTA_RECV_TIME_OUT           (3000)
#define OTA_KEEP_IDLE_TIME          (6)
#define OTA_KEEP_INTVL_TIME         (3)
#define OTA_KEEP_COUNT              (3)

#define OTA_RES_LEN_MAX             (1024)
#define OTA_URL_LEN_MAX             (512)
#define OTA_MD5_LEN_MAX             (128)

#define BIN_BUFF_LEN                (2048)

#define OTA_YIELD_TMIE              (5*1000)  //OTA select超时时间

#define OTA_RESPONSE_BODY_MAX_LEN   (4096)

#define OTA_PORT_SSL                (443)     //HTTP SSL端口


extern void fog_v2_ota(void);


#endif

