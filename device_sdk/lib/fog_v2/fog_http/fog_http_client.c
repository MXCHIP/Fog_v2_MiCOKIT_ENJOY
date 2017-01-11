#include "mico.h"
#include "fog_http_client.h"
#include "fog_v2_include.h"
#include "HTTPUtils.h"
#include "SocketUtils.h"

#define app_log(M, ...)             custom_log("FOG_V2_HTTP_CLIENT", M, ##__VA_ARGS__)

#ifndef FOG_V2_HTTP_DOMAIN_NAME
    #error "FOG_V2_HTTP_DOMAIN_NAME is not define"
#endif

#ifndef FOG_V2_HTTP_PORT_SSL
    #error "FOG_V2_HTTP_DOMAIN_NAME is not define"
#endif

#ifndef HTTP_REQ_LOG
    #error "FOG_V2_HTTP_PORT_SSL is not define"
#endif

typedef struct _http_context_t{
  char *content;
  uint32_t content_length;
} http_context_t;

char* http_server_ssl_cert_str =
"-----BEGIN CERTIFICATE-----\r\n\
MIIDIDCCAomgAwIBAgIENd70zzANBgkqhkiG9w0BAQUFADBOMQswCQYDVQQGEwJV\r\n\
UzEQMA4GA1UEChMHRXF1aWZheDEtMCsGA1UECxMkRXF1aWZheCBTZWN1cmUgQ2Vy\r\n\
dGlmaWNhdGUgQXV0aG9yaXR5MB4XDTk4MDgyMjE2NDE1MVoXDTE4MDgyMjE2NDE1\r\n\
MVowTjELMAkGA1UEBhMCVVMxEDAOBgNVBAoTB0VxdWlmYXgxLTArBgNVBAsTJEVx\r\n\
dWlmYXggU2VjdXJlIENlcnRpZmljYXRlIEF1dGhvcml0eTCBnzANBgkqhkiG9w0B\r\n\
AQEFAAOBjQAwgYkCgYEAwV2xWGcIYu6gmi0fCG2RFGiYCh7+2gRvE4RiIcPRfM6f\r\n\
BeC4AfBONOziipUEZKzxa1NfBbPLZ4C/QgKO/t0BCezhABRP/PvwDN1Dulsr4R+A\r\n\
cJkVV5MW8Q+XarfCaCMczE1ZMKxRHjuvK9buY0V7xdlfUNLjUA86iOe/FP3gx7kC\r\n\
AwEAAaOCAQkwggEFMHAGA1UdHwRpMGcwZaBjoGGkXzBdMQswCQYDVQQGEwJVUzEQ\r\n\
MA4GA1UEChMHRXF1aWZheDEtMCsGA1UECxMkRXF1aWZheCBTZWN1cmUgQ2VydGlm\r\n\
aWNhdGUgQXV0aG9yaXR5MQ0wCwYDVQQDEwRDUkwxMBoGA1UdEAQTMBGBDzIwMTgw\r\n\
ODIyMTY0MTUxWjALBgNVHQ8EBAMCAQYwHwYDVR0jBBgwFoAUSOZo+SvSspXXR9gj\r\n\
IBBPM5iQn9QwHQYDVR0OBBYEFEjmaPkr0rKV10fYIyAQTzOYkJ/UMAwGA1UdEwQF\r\n\
MAMBAf8wGgYJKoZIhvZ9B0EABA0wCxsFVjMuMGMDAgbAMA0GCSqGSIb3DQEBBQUA\r\n\
A4GBAFjOKer89961zgK5F7WF0bnj4JXMJTENAKaSbn+2kmOeUJXRmm/kEd5jhW6Y\r\n\
7qj/WsjTVbJmcVfewCHrPSqnI0kBBIZCe/zuf6IWUrVnZ9NA2zsmWLIodz2uFHdh\r\n\
1voqZiegDfqnc1zqcPGUIWVEX/r87yloqaKHee9570+sB3c4\r\n\
-----END CERTIFICATE-----";

const char *device_activate_string =
"POST /device/token-refresh/ HTTP/1.1\r\n"
"Content-Type: application/json\r\n"
"Host: v2.fogcloud.io\r\n"
"Connection: keep-alive\r\n"
"Content-Length: %d\r\n\r\n"
"%s";

bool has_http_req_send = true;  //是否有http请求需要发送

static FOG_HTTP_RESPONSE_SETTING_S fog_http_res; //全局变量 http响应的全局设置
static char *fog_http_requeset = NULL;

mico_queue_t fog_http_request_queue = NULL;  //FOG HTTP的发送请求队列
mico_queue_t fog_http_response_queue = NULL; //FOG HTTP的接收响应队列

OSStatus start_fogcloud_http_client(void);

static OSStatus http_queue_init(void);
static OSStatus http_queue_deinit(void);
static void onClearData( struct _HTTPHeader_t * inHeader, void * inUserContext );
static OSStatus onReceivedData( struct _HTTPHeader_t * inHeader, uint32_t inPos, uint8_t * inData, size_t inLen, void * inUserContext );
static void fog_v2_http_client_thread(mico_thread_arg_t arg);


//获取网络连接状态
static bool http_get_wifi_status(void)
{
    LinkStatusTypeDef link_status;

    memset(&link_status, 0, sizeof(link_status));

    micoWlanGetLinkStatus(&link_status);

    return (bool)(link_status.is_connected);
}

OSStatus start_fogcloud_http_client(void)
{
    OSStatus err = kGeneralErr;

    err = http_queue_init();
    require_noerr(err, exit);

    if(http_get_wifi_status() == false)
    {
        err = kGeneralErr;
        app_log("[ERROR]wifi is not connect!");
        goto exit;
    }

    err = mico_rtos_create_thread(NULL, MICO_APPLICATION_PRIORITY, "fog http client", fog_v2_http_client_thread, 0x2800, (uint32_t)NULL); //栈太小可能会导致ssl_connect无法返回

 exit:
     if(err != kNoErr)
     {
         http_queue_deinit();
     }

    return err;
}

static OSStatus http_queue_init(void)
{
    OSStatus err = kGeneralErr;

    err = mico_rtos_init_queue( &fog_http_request_queue, "fog_http_request_queue", sizeof(FOG_HTTP_REQUEST_SETTING_S *), 1);  //只容纳一个成员 传递的只是地址
    require_noerr( err, exit );

    err = mico_rtos_init_queue( &fog_http_response_queue, "fog_http_response_queue", sizeof(FOG_HTTP_RESPONSE_SETTING_S *), 1); //只容纳一个成员 传递的只是地址
    require_noerr( err, exit );

 exit:
    return err;
}

static OSStatus http_queue_deinit(void)
{
    if(fog_http_request_queue != NULL)
    {
        mico_rtos_deinit_queue( &fog_http_request_queue);
    }

    if(fog_http_response_queue != NULL)
    {
        mico_rtos_deinit_queue( &fog_http_response_queue);
    }
    return kNoErr;
}


//根据fog_http_req来生成一个http请求,组合出一个字符串
static OSStatus generate_fog_http_request(FOG_HTTP_REQUEST_SETTING_S *fog_http_req)
{
    OSStatus err = kGeneralErr;
    uint32_t http_req_all_len = strlen(fog_http_req->http_body) + 1024; //为head部分预留1024字节

    fog_http_requeset = malloc( http_req_all_len );
    if ( fog_http_requeset == NULL )
    {
        app_log("[ERROR]malloc error!!! malloc len is %ld", http_req_all_len);
        return kGeneralErr;
    }

    memset(fog_http_requeset, 0, http_req_all_len);

    if(fog_http_req->method != HTTP_POST && fog_http_req->method != HTTP_GET)
    {
        app_log("http method error!");
        err = kGeneralErr;
        goto exit;
    }

    if(fog_http_req->method == HTTP_POST)
    {
         sprintf(fog_http_requeset, "%s %s HTTP/1.1\r\n", HTTP_HEAD_METHOD_POST, fog_http_req->request_uri);
    }else if(fog_http_req->method == HTTP_GET)
    {
         sprintf(fog_http_requeset, "%s %s HTTP/1.1\r\n", HTTP_HEAD_METHOD_GET, fog_http_req->request_uri);
    }

    sprintf(fog_http_requeset + strlen(fog_http_requeset), "Host: %s\r\n", fog_http_req->host_name); //增加hostname

    sprintf(fog_http_requeset + strlen(fog_http_requeset), "Content-Type: application/json\r\nConnection: Keepalive\r\n"); //增加Content-Type和Connection设置
    //sprintf(fog_http_requeset + strlen(fog_http_requeset), "Content-Type: application/json\r\n"); //增加Content-Type和Connection设置
    //sprintf(fog_http_requeset + strlen(fog_http_requeset), "Content-Type: application/json\r\nConnection: Close\r\n"); //增加Content-Type和Connection设置

    if(fog_http_req->is_jwt == true)
    {
        sprintf(fog_http_requeset + strlen(fog_http_requeset), "Authorization: JWT %s\r\n", fog_http_req->jwt);
    }

    //增加http body部分
    if(fog_http_req->http_body != NULL)
    {
        sprintf(fog_http_requeset + strlen(fog_http_requeset), "Content-Length: %d\r\n\r\n", strlen((const char *)fog_http_req->http_body)); //增加Content-Length

        sprintf(fog_http_requeset + strlen(fog_http_requeset), "%s", fog_http_req->http_body);

        if(fog_http_req->http_body != NULL)
        {
            free(fog_http_req->http_body);
            fog_http_req->http_body = NULL;
        }
    }else
    {
        sprintf(fog_http_requeset + strlen(fog_http_requeset), "Content-Length: 0\r\n\r\n"); //增加Content-Length
    }

    err = kNoErr;

#if (HTTP_REQ_LOG == 1)
    app_log("--------------------------------------");
    app_log("\r\n%s", fog_http_requeset);
    app_log("--------------------------------------");
#endif

    exit:
    if(err != kNoErr)
    {
        if(fog_http_requeset != NULL)
        {
            free(fog_http_requeset);
            fog_http_requeset = NULL;
        }
    }

    return err;
}

//给response的消息队列发送消息
void send_response_to_queue(FOG_HTTP_RESPONSE_E status, uint32_t http_id, int32_t status_code, const char* response_body)
{
    OSStatus err = kGeneralErr;
    FOG_HTTP_RESPONSE_SETTING_S *fog_http_response_temp = NULL, *fog_http_res_p = NULL;
    static uint32_t local_id = 0;

    if(local_id == http_id)    //过滤重复id   理论上id是递增的
    {
        return;
    }

    fog_http_res.send_status = status;
    fog_http_res.http_res_id = http_id;
    fog_http_res.status_code = status_code;

    if(response_body != NULL)
    {
        fog_http_res.fog_response_body = malloc(strlen(response_body) + 2);
        require_action_string(fog_http_res.fog_response_body != NULL, exit, err = kNoMemoryErr, "[ERROR]malloc() error!");

        memset(fog_http_res.fog_response_body, 0, strlen(response_body) + 2); //清0

        memcpy(fog_http_res.fog_response_body, response_body, strlen(response_body));
    }else
    {
        fog_http_res.fog_response_body = NULL;
    }

    if(false == mico_rtos_is_queue_empty(&fog_http_response_queue))
    {
        app_log("[error]fog_http_response_queue is full");

        err = mico_rtos_pop_from_queue(&fog_http_response_queue, &fog_http_response_temp, 10);   //如果满先弹出一个
        require_noerr_action(err, exit, app_log("[error]mico_rtos_pop_from_queue err"));

        if(fog_http_response_temp->fog_response_body != NULL)
        {
            free(fog_http_response_temp->fog_response_body);
            fog_http_response_temp->fog_response_body = NULL;
        }

        fog_http_response_temp = NULL;
    }

    fog_http_res_p = &fog_http_res;
    err = mico_rtos_push_to_queue(&fog_http_response_queue, &fog_http_res_p, 10);
    require_noerr_action(err, exit, app_log("[error]mico_rtos_push_to_queue err"));

    local_id = http_id;

    exit:
    if(err != kNoErr)
    {
        if(fog_http_res.fog_response_body != NULL)
        {
            free(fog_http_res.fog_response_body);
            fog_http_res.fog_response_body = NULL;
        }
    }

    return;
}

static OSStatus usergethostbyname( const char * domain, uint8_t * addr, uint8_t addrLen )
{
    struct hostent* host = NULL;
    struct in_addr in_addr;
    char **pptr = NULL;
    char *ip_addr = NULL;

    if(addr == NULL || addrLen < 16)
    {
        return kGeneralErr;
    }

    host = gethostbyname( domain );
    if((host == NULL) || (host->h_addr_list) == NULL)
    {
        return kGeneralErr;
    }

    pptr = host->h_addr_list;
//    for (; *pptr != NULL; pptr++ )
    {
        in_addr.s_addr = *(uint32_t *) (*pptr);
        ip_addr = inet_ntoa(in_addr);
        memset(addr, 0, addrLen);
        memcpy(addr, ip_addr, strlen(ip_addr));
    }

    return kNoErr;
}

//设置tcp keep_alive 参数
static int user_set_tcp_keepalive(int socket, int send_timeout, int recv_timeout, int idle, int interval, int count)
{
    int retVal = 0, opt = 0;

    retVal = setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO, (char *)&send_timeout,sizeof(int));
    require_string(retVal >= 0, exit, "SO_SNDTIMEO setsockopt error!");

    app_log("setsockopt SO_SNDTIMEO=%d ms ok.", send_timeout);

//    retVal = setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&recv_timeout,sizeof(int));
//    require_string(retVal >= 0, exit, "SO_RCVTIMEO setsockopt error!");
//
//    app_log("setsockopt SO_RCVTIMEO=%d ms ok.", recv_timeout);

    // set keepalive
    opt = 1;
    retVal = setsockopt(socket, SOL_SOCKET, SO_KEEPALIVE, (void *)&opt, sizeof(opt)); // 开启socket的Keepalive功能
    require_string(retVal >= 0, exit, "SO_KEEPALIVE setsockopt error!");

    opt = idle;
    retVal = setsockopt(socket, IPPROTO_TCP, TCP_KEEPIDLE, (void *)&opt, sizeof(opt)); // TCP IDLE idle秒以后开始发送第一个Keepalive包
    require_string(retVal >= 0, exit, "TCP_KEEPIDLE setsockopt error!");

    opt = interval;
    retVal = setsockopt(socket, IPPROTO_TCP, TCP_KEEPINTVL, (void *)&opt, sizeof(opt)); // TCP后面的Keepalive的间隔时间是interval秒
    require_string(retVal >= 0, exit, "TCP_KEEPINTVL setsockopt error!");

    opt = count;
    retVal = setsockopt(socket, IPPROTO_TCP, TCP_KEEPCNT, (void *)&opt, sizeof(opt)); // Keepalive 数量为count次
    require_string(retVal >= 0, exit, "TCP_KEEPCNT setsockopt error!");

    app_log("set tcp keepalive: idle=%d, interval=%d, cnt=%d.", idle, interval, count);

    exit:
    return retVal;
}


//FOG V2 http client
static void fog_v2_http_client_thread(mico_thread_arg_t arg)
{
    OSStatus err = kGeneralErr;
    int http_fd = -1;
    int ssl_errno = 0;
    int ret = 0;
    mico_ssl_t client_ssl = NULL;
    fd_set readfds;
    char ipstr[20] = {0};
    struct sockaddr_in addr;
    HTTPHeader_t *httpHeader = NULL;
    http_context_t context = { NULL, 0 };
    char *fog_host = FOG_V2_HTTP_DOMAIN_NAME;
    struct timeval t = {0, HTTP_YIELD_TMIE*1000};
    uint32_t req_id = 0;
    FOG_HTTP_REQUEST_SETTING_S *fog_http_req = NULL; //http 请求

 HTTP_SSL_START:
    set_https_connect_status(false);

    app_log("start dns annlysis, domain:%s", fog_host);
    err = usergethostbyname(fog_host, (uint8_t *)ipstr, sizeof(ipstr));
    if ( err != kNoErr )
    {
        app_log("dns error!!! doamin:%s", fog_host);
        mico_thread_msleep( 200 );
        goto HTTP_SSL_START;
    }

    app_log("HTTP server address: host:%s, ip: %s", fog_host, ipstr);

    /*HTTPHeaderCreateWithCallback set some callback functions */
    httpHeader = HTTPHeaderCreateWithCallback( HTTP_RESPONSE_BODY_MAX_LEN, onReceivedData, onClearData, &context );
    if ( httpHeader == NULL )
    {
        mico_thread_msleep( 200 );
        app_log("HTTPHeaderCreateWithCallback() error");
        goto HTTP_SSL_START;
    }

    SocketClose( &http_fd );
    http_fd = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr( ipstr );
    addr.sin_port = htons(FOG_V2_HTTP_PORT_SSL); //HTTP SSL端口 443

    ret = user_set_tcp_keepalive(http_fd, HTTP_SEND_TIME_OUT, HTTP_RECV_TIME_OUT, HTTP_KEEP_IDLE_TIME, HTTP_KEEP_INTVL_TIME, HTTP_KEEP_COUNT);
    if(ret < 0)
    {
        app_log("user_set_tcp_keepalive() error");
        goto exit;
    }

    app_log("start connect");
    //app_log("#####start connect#####:num_of_chunks:%d,allocted_memory:%d, free:%d, total_memory:%d", MicoGetMemoryInfo()->num_of_chunks, MicoGetMemoryInfo()->allocted_memory, MicoGetMemoryInfo()->free_memory, MicoGetMemoryInfo()->total_memory);

    err = connect( http_fd, (struct sockaddr *)&addr, sizeof(addr) );
    require_noerr_action( err, exit, app_log("connect http server failed"));

    //ssl_version_set(TLS_V1_2_MODE);    //设置SSL版本
    ssl_set_client_version(TLS_V1_2_MODE);

    app_log("start ssl_connect");

    client_ssl = ssl_connect( http_fd, 0, NULL, &ssl_errno );
    //client_ssl = ssl_connect( http_fd, strlen(http_server_ssl_cert_str), http_server_ssl_cert_str, &ssl_errno );
    require_action( client_ssl != NULL, exit, {err = kGeneralErr; app_log("https ssl_connnect error, errno = %d", ssl_errno);} );

    app_log("#####https connect#####:num_of_chunks:%d, free:%d", MicoGetMemoryInfo()->num_of_chunks, MicoGetMemoryInfo()->free_memory);

 SSL_SEND:
    set_https_connect_status(true);

    //1.从消息队列中取一条信息
    err = mico_rtos_pop_from_queue( &fog_http_request_queue, &fog_http_req, MICO_WAIT_FOREVER);
    require_noerr( err, SSL_SEND );

    req_id = fog_http_req->http_req_id;

    err = generate_fog_http_request(fog_http_req);   //生成http请求
    if(err != kNoErr)
    {
        send_response_to_queue(HTTP_REQUEST_ERROR, req_id, 0, NULL);
        goto SSL_SEND;
    }

    ret = ssl_send( client_ssl, fog_http_requeset, strlen((const char *)fog_http_requeset) );       /* Send HTTP Request */

    if(fog_http_requeset != NULL) //释放发送缓冲区
    {
        free(fog_http_requeset);
        fog_http_requeset = NULL;
    }

    if(ret > 0)
    {
       //app_log("ssl_send success [%d] [%d]", strlen((const char *)fog_http_requeset) ,ret);
       //app_log("%s", fog_http_requeset);
    }else
    {
       app_log("-----------------ssl_send error, ret = %d", ret);
       err = kGeneralErr;
       goto exit;
    }

    FD_ZERO( &readfds );
    FD_SET( http_fd, &readfds );

    ret = select( http_fd + 1, &readfds, NULL, NULL, &t );
    if(ret == -1 || ret == 0)
    {
       app_log("-----------------select error, ret = %d", ret);
       err = kGeneralErr;
       goto exit;
    }

    if( FD_ISSET( http_fd, &readfds ) )
    {
        /*parse header*/
        err = SocketReadHTTPSHeader( client_ssl, httpHeader );
        switch ( err )
        {
         case kNoErr:
            {
                 if((httpHeader->statusCode == -1) || (httpHeader->statusCode >= 500))
                 {
                    app_log("[ERROR]fog http response error, code:%d", httpHeader->statusCode);
                    goto exit; //断开重新连接
                 }

                 if((httpHeader->statusCode == 405) || (httpHeader->statusCode == 404))
                 {
                    app_log( "[FAILURE]fog http response fail, code:%d", httpHeader->statusCode);
                    send_response_to_queue(HTTP_RESPONSE_FAILURE, req_id, httpHeader->statusCode, NULL);
                    break;
                 }

                //app_log( "[OK]fog http response ok, code:%d", httpHeader->statusCode);

                //只有code正确才解析返回数据,错误情况下解析容易造成内存溢出
                if((httpHeader->statusCode == 200) || (httpHeader->statusCode == 201) || (httpHeader->statusCode == 202) || (httpHeader->statusCode == 400) || (httpHeader->statusCode == 401) || (httpHeader->statusCode == 403))
                {
                    //PrintHTTPHeader( httpHeader );
                    err = SocketReadHTTPSBody( client_ssl, httpHeader );    /*get body data*/
                    require_noerr( err, exit );
#if (HTTP_REQ_LOG == 1)
                    app_log( "Content Data:[%ld]%s", context.content_length, context.content );     /*get data and print*/
#endif
                    send_response_to_queue(HTTP_RESPONSE_SUCCESS, req_id, httpHeader->statusCode, context.content);
                }else
                {
                    app_log( "[ERR]fog http response ok, but code = %d !!!", httpHeader->statusCode);
                    goto exit;
                }

                break;
            }
         case EWOULDBLOCK:
            {
                break;
            }
         case kNoSpaceErr:
            {
                app_log("SocketReadHTTPSHeader kNoSpaceErr");
                goto exit;
                break;
            }
         case kConnectionErr:
            {
                app_log("SocketReadHTTPSHeader kConnectionErr");
                goto exit;
                break;
            }
         default:
            {
                app_log("ERROR: HTTP Header parse error: %d", err);
                goto exit;
                break;
            }
        }
    }

    HTTPHeaderClear( httpHeader );
    app_log("#####https send#####:num_of_chunks:%d, free:%d", MicoGetMemoryInfo()->num_of_chunks, MicoGetMemoryInfo()->free_memory);
    goto SSL_SEND;

 exit:
    set_https_connect_status(false);
    send_response_to_queue(HTTP_CONNECT_ERROR, req_id, 0, NULL);    //只有发生连接发生错误的时候才会进入exit中

    if( client_ssl )
    {
        ssl_close( client_ssl );
        client_ssl = NULL;
    }

    SocketClose( &http_fd );

    HTTPHeaderDestory( &httpHeader );

    mico_thread_msleep(200);
    app_log("#####https disconnect#####:num_of_chunks:%d, free:%d", MicoGetMemoryInfo()->num_of_chunks, MicoGetMemoryInfo()->free_memory);
    goto HTTP_SSL_START;

    if(fog_http_requeset != NULL)
    {
        free(fog_http_requeset);
        fog_http_requeset = NULL;
    }

    http_queue_deinit();

    app_log( "Exit: Client exit with err = %d, fd:%d", err, http_fd );
    mico_rtos_delete_thread(NULL);
    return;
}

/*one request may receive multi reply*/
static OSStatus onReceivedData( struct _HTTPHeader_t * inHeader, uint32_t inPos, uint8_t * inData, size_t inLen, void * inUserContext )
{
    OSStatus err = kNoErr;
    http_context_t *context = inUserContext;
    if( inHeader->chunkedData == false){ //Extra data with a content length value
        if( inPos == 0 && context->content == NULL ){
            context->content = calloc( inHeader->contentLength + 1, sizeof(uint8_t) );
            require_action( context->content, exit, err = kNoMemoryErr );
            context->content_length = inHeader->contentLength;

        }
        memcpy( context->content + inPos, inData, inLen );
    }else{ //extra data use a chunked data protocol
        //app_log("This is a chunked data, %d", inLen);
        if( inPos == 0 ){
            context->content = calloc( inHeader->contentLength + 1, sizeof(uint8_t) );
            require_action( context->content, exit, err = kNoMemoryErr );
            context->content_length = inHeader->contentLength;
        }else{
            context->content_length += inLen;
            context->content = realloc(context->content, context->content_length + 1);
            require_action( context->content, exit, err = kNoMemoryErr );
        }
        memcpy( context->content + inPos, inData, inLen );
    }

 exit:
    return err;
}

/* Called when HTTPHeaderClear is called */
static void onClearData( struct _HTTPHeader_t * inHeader, void * inUserContext )
{
    UNUSED_PARAMETER( inHeader );
    http_context_t *context = inUserContext;
    if( context->content ) {
        free( context->content );
        context->content = NULL;
    }
}
