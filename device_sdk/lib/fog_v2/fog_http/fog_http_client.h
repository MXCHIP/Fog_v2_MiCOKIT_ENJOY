#ifndef __FOG_HTTP_CLIENT_H_
#define __FOG_HTTP_CLIENT_H_

#define HTTP_SEND_TIME_OUT              (3000)
#define HTTP_RECV_TIME_OUT              (3000)
#define HTTP_KEEP_IDLE_TIME             (6)
#define HTTP_KEEP_INTVL_TIME            (3)
#define HTTP_KEEP_COUNT                 (3)


#define HTTP_YIELD_TMIE                 (2000) //http超时时间

#define HTTP_REQUEST_BODY_MAX_LEN       (2048)
#define HTTP_REQUEST_HOST_NAME_MAX_LEN  (64)
#define HTTP_REQUEST_REQ_URI_MAX_LEN    (64)
#define HTTP_REQUEST_JWT_MAX_LEN        (512)

#define HTTP_RESPONSE_BODY_MAX_LEN      (2048)    //HTTP返回接收的最大长度为2K

#define HTTP_HEAD_METHOD_POST           ("POST")
#define HTTP_HEAD_METHOD_GET            ("GET")


typedef enum {
    HTTP_POST = 1,
    HTTP_GET = 2
} FOG_HTTP_METHOD;

typedef enum {
    HTTP_REQUEST_ERROR = 1,     //请求错误
    HTTP_CONNECT_ERROR = 2,     //连接失败
    HTTP_RESPONSE_SUCCESS = 3,  //成功
    HTTP_RESPONSE_FAILURE = 4   //失败
} FOG_HTTP_RESPONSE_E;


typedef struct _FOG_HTTP_RESPONSE_SETTING
{
    FOG_HTTP_RESPONSE_E send_status;    //发送状态
    uint32_t http_res_id;
    int32_t status_code;                //http错误码
    char *fog_response_body;
}FOG_HTTP_RESPONSE_SETTING_S;


typedef struct _FOG_HTTP_REQUEST_SETTING
{
    uint32_t http_req_id;
    bool is_jwt;    //是否jwt方式登录
    FOG_HTTP_METHOD method;
    char jwt[HTTP_REQUEST_JWT_MAX_LEN];
    char request_uri[HTTP_REQUEST_REQ_URI_MAX_LEN];
    char host_name[HTTP_REQUEST_HOST_NAME_MAX_LEN];
    char *http_body;
}FOG_HTTP_REQUEST_SETTING_S;


extern mico_queue_t fog_http_request_queue;  //FOG HTTP的发送请求队列
extern mico_queue_t fog_http_response_queue; //FOG HTTP的接收响应队列

extern OSStatus start_fogcloud_http_client(void);

#endif

