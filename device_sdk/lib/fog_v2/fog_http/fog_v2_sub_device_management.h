#ifndef _FOG_V2_SUB_DEVICE_MANAGEMENT_H_
#define _FOG_V2_SUB_DEVICE_MANAGEMENT_H_

#if (FOG_V2_USE_SUB_DEVICE == 1)

#define REMOTE_DEVICE_LIST_BUFF_LEN     (2048)

//available subdevice callback
typedef void (*FOG_V2_SUBDEVICE_AVAILABLE_CB)(const char *product_id, const char *mac);


extern OSStatus fog_v2_subdevice_des_init(void);

extern bool get_sub_device_queue_is_available_by_index(uint32_t index);
extern mico_queue_t *get_sub_device_queue_addr_by_index(uint32_t index);
extern char *get_sub_device_id_by_index(uint32_t index);
extern char *get_sub_device_commands_topic_by_index( uint32_t index );
extern char *get_sub_device_cmd_topic_by_index( uint32_t index );
extern char *get_sub_device_mac_by_index( uint32_t index );
extern char *get_sub_device_product_id_by_index( uint32_t index );
extern OSStatus fog_v2_get_sub_device_all_available(FOG_V2_SUBDEVICE_AVAILABLE_CB user_callbck);

extern OSStatus remove_mqtt_topic_by_mac(const char *s_product_id, const char *mac);
extern OSStatus add_mqtt_topic_by_mac(const char *s_product_id, const char *mac);

extern bool get_sub_device_queue_usable_index(uint32_t *index);
extern bool get_sub_device_queue_index_by_mac(uint32_t *index, const char *s_product_id, const char *s_mac);
extern bool get_sub_device_queue_index_by_deviceid(uint32_t *index, const char *s_device_id);
extern bool get_sub_device_queue_index_by_mqtt_topic(uint32_t *index, const char *s_mqtt_topic);

extern bool sub_device_queue_get(const char *s_product_id, const char *s_mac, const char *s_device_id); //申请子设备资源
extern bool sub_device_queue_put_by_mac(const char *s_product_id, const char *s_mac); //释放子设备资源 通过mac地址
extern bool sub_device_queue_put_by_deviceid(const char *s_deviceid);
extern bool sub_device_queue_put_by_mqtt_topic(const char *s_mqtt_topic);

extern void get_sub_device_queue_index_info(uint32_t index);
extern void get_sub_device_queue_all_info(void);

extern OSStatus init_subdevice_des_mutex( ); //初始化锁
extern OSStatus lock_subdevice_des_mutex( ); //获取锁
extern OSStatus unlock_subdevice_des_mutex( ); //释放锁

#endif //end of FOG_V2_USE_SUB_DEVICE

#endif
