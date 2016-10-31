#ifndef __FOGCLOUD_DOWN_THREAD_H_
#define __FOGCLOUD_DOWN_THREAD_H_

#define RECV_DATA_QUEUE_MAX_NUM     (4)
#define MIN_TAIL_LEN                (2)

#define FOG_V2_RECV_BUFF_LEN        (1024)

extern void user_downstream_thread(mico_thread_arg_t args);
extern OSStatus init_receive_data_queue(void);

#endif

