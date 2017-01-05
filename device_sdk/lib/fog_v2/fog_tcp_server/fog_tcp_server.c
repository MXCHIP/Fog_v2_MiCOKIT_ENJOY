#include "mico.h"
#include "SocketUtils.h"
#include "fog_tcp_server.h"
#include "fog_v2_include.h"

#define tcp_server_log(M, ...)  custom_log("FOG_TCP_SEVER", M, ##__VA_ARGS__)

#ifndef FOG_V2_TCP_SERVER_PORT
    #error "FOG_V2_TCP_SERVER_PORT is not define"
#endif

OSStatus fog_tcp_server_start( void );
OSStatus fog_tcp_server_close( void );
static bool fog_v2_tcp_server_is_start(void);

void fog_tcp_client_thread(mico_thread_arg_t arg);
void fog_tcp_server_thread(mico_thread_arg_t arg);

const char *fog_local_response_vercode_json = "{\"deviceid\":\"%s\",\"devicepw\":\"%s\",\"vercode\":\"%s\"}";
const char *fog_local_response_vercode_error = "{\"error\":\"no_vercode\"}";
const char *fog_local_response_vercode_is_bind = "{\"error\":\"is_bind\"}";
const char *fog_local_response_http = "HTTP/1.1 200 OK\r\n\
Content-Type: application/json\r\n\
Content-Length: %d\r\n\r\n\
%s\r\n";

bool fog_v2_tcp_server_has_client = false;  //已有客户端标志位
static bool fog_v2_is_tcp_server_start = false;
static mico_semaphore_t fog_v2_close_tcp_server = NULL;

static bool fog_v2_tcp_server_is_start(void)
{
    return fog_v2_is_tcp_server_start;
}

//校验收到的客户端数据
bool check_recv_data(char *buf)
{
    if(strstr(buf, "getvercode") != NULL)
    {
        return true;
    }else
    {
        return false;
    }
}

//从云端拿到vercode 并发送给手机
OSStatus get_vercode_from_fog(int client_fd)
{
    OSStatus err = kGeneralErr;
    ssize_t send_len = 0;
    char *response_body = NULL;
    char *response_html = NULL;

    response_body = malloc(256);
    require_action_string(response_body != NULL, exit, err = kNoMemoryErr, "malloc() error");

    response_html = malloc(512);
    require_action_string(response_html != NULL, exit, err = kNoMemoryErr, "malloc() error");

    memset(response_body, 0, 256);
    memset(response_html, 0, 512);

    //云端获取vercode
    err = fog_v2_device_generate_device_vercode();
    if(err == kNoErr)
    {
        if(fog_v2_is_have_superuser() == true)
        {
            sprintf(response_html, fog_local_response_http, strlen(fog_local_response_vercode_is_bind), fog_local_response_vercode_is_bind);
        }else
        {
            sprintf(response_body, fog_local_response_vercode_json,  get_fog_des_g()->device_id, get_fog_des_g()->devicepw, get_fog_des_g()->vercode);
            sprintf(response_html, fog_local_response_http, strlen(response_body), response_body);
        }
    }else
    {
        sprintf(response_html, fog_local_response_http, strlen(fog_local_response_vercode_error), fog_local_response_vercode_error);
    }

    send_len = send(client_fd, response_html, strlen(response_html), 0);

    tcp_server_log("send_len:%d, response phone:\r\n%s", send_len, response_html);

    exit:
    if(response_body != NULL)
    {
        free(response_body);
        response_body = NULL;
    }

    if(response_html != NULL)
    {
        free(response_html);
        response_html = NULL;
    }

    return err;
}



//客户端处理线程
void fog_tcp_client_thread(mico_thread_arg_t arg)
{
    OSStatus err = kGeneralErr;
    int fd = (int)(*(uint32_t *)arg);
    int len = 0;
    fd_set readfds;
    char *buf = NULL;
    struct timeval t;

    buf = (char*)malloc(CLIENT_BUFF_SIZE);
    require_action(buf, exit, err = kNoMemoryErr);

    t.tv_sec = 3; //3s超时时间
    t.tv_usec = 0;

    fog_v2_tcp_server_has_client = true;

    while(1)
    {
        FD_ZERO(&readfds);
        FD_SET(fd, &readfds);

        err = select(fd + 1, &readfds, NULL, NULL, &t);
        require_action_string(err > 0, exit, err = kConnectionErr, "tcp_client_thread select() error"); //超时也认为有问题

        if( FD_ISSET( fd, &readfds ) ) /*one client has data*/
        {
            memset(buf, 0, CLIENT_BUFF_SIZE);
            len = recv( fd, buf, CLIENT_BUFF_SIZE, 0 );
            require_action( len >= 0, exit, err = kConnectionErr );    //返回值为0 表示客户端主动断开连接

            if(len == 0)
            {
                err = kConnectionErr;
                tcp_server_log("client disconnected, fd: %d", fd);
            }

            tcp_server_log("fd: %d, recv data %d from client, data:\r\n%s", fd, len, buf);

            //数据过滤
            require_string(check_recv_data(buf) == true, exit, "recv tcp client data error");

            err = get_vercode_from_fog(fd);
            if(err == kNoErr)
            {
                mico_thread_msleep(100); //延时200ms
            }

            goto exit;
        }
    }

 exit:
    tcp_server_log( "TCP client thread exit with err: %d", err );

    if( buf != NULL )
    {
        free( buf );
        buf = NULL;
    }

    SocketClose( &fd );

    fog_v2_tcp_server_has_client = false;

    mico_rtos_delete_thread( NULL );
}

//TCP server监听线程
void fog_tcp_server_thread( mico_thread_arg_t arg )
{
    OSStatus err = kNoErr;
    struct sockaddr_in server_addr, client_addr;
    socklen_t sockaddr_t_size = sizeof( client_addr );
    char  client_ip_str[16] = {0};
    int tcp_listen_fd = -1, client_fd = -1;
    fd_set readfds;
    int tcp_server_close_event_fd = -1;

    fog_v2_is_tcp_server_start = true;

    memset(&server_addr, 0, sizeof(server_addr));
    memset(&client_addr, 0, sizeof(client_addr));

    tcp_server_close_event_fd = mico_create_event_fd( fog_v2_close_tcp_server);
    require_action( tcp_server_close_event_fd >= 0, exit, err = kGeneralErr );

    tcp_listen_fd = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
    require_action(IsValidSocket( tcp_listen_fd ), exit, err = kNoResourcesErr );

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;/* Accept conenction request on all network interface */
    server_addr.sin_port = htons( FOG_V2_TCP_SERVER_PORT );/* Server listen on port: 8002 */

    err = bind( tcp_listen_fd, (struct sockaddr *)&server_addr, sizeof( server_addr ) );
    require_noerr( err, exit );

    err = listen( tcp_listen_fd, 0);
    require_noerr( err, exit );

    while(1)
    {
        FD_ZERO( &readfds );
        FD_SET( tcp_listen_fd, &readfds );
        FD_SET( tcp_server_close_event_fd, &readfds );

        require( select(Max(tcp_listen_fd, tcp_server_close_event_fd) + 1, &readfds, NULL, NULL, NULL) >= 0, exit );

        if(FD_ISSET(tcp_server_close_event_fd, &readfds))
        {
            err = kNoErr;
            goto exit;
        }

        if(FD_ISSET(tcp_listen_fd, &readfds))
        {
            client_fd = accept( tcp_listen_fd, (struct sockaddr *)&client_addr, &sockaddr_t_size );

            if(fog_v2_tcp_server_has_client == true)
            {
                tcp_server_log("client thread exit! Refuse new request!");
                SocketClose( &client_fd );
            }

            if( IsValidSocket( client_fd ) )
            {
                strcpy( client_ip_str, inet_ntoa( client_addr.sin_addr ) );
                tcp_server_log( "TCP Client %s:%d connected, fd: %d", client_ip_str, client_addr.sin_port, client_fd );

                err = mico_rtos_create_thread( NULL, MICO_APPLICATION_PRIORITY, " FOG TCP Client", fog_tcp_client_thread, 0x800, (uint32_t)(&client_fd) ); //堆栈空间至少需要4K
                if(err != kNoErr)
                {
                    tcp_server_log("[ERROR]mico_rtos_create_thread create error!");
                    SocketClose( &client_fd );
                }
            }
        }
    }

 exit:
    if( err != kNoErr )
    {
        tcp_server_log( "Server listerner thread exit with err: %d", err );
    }

    SocketClose( &tcp_listen_fd );

    mico_rtos_deinit_event_fd( tcp_server_close_event_fd );

    mico_rtos_deinit_semaphore( &fog_v2_close_tcp_server );
    fog_v2_close_tcp_server = NULL;

    tcp_server_log( "fog v2 tcp sever closed!");

    fog_v2_is_tcp_server_start = false;
    mico_rtos_delete_thread(NULL );
}

//start fog tcp server
OSStatus fog_tcp_server_start( void )
{
    OSStatus err = kNoErr;

    if(fog_v2_tcp_server_is_start() == true)
    {
        tcp_server_log( "fog v2 tcp sever is already start!");
        return kNoErr;
    }

    tcp_server_log( "fog v2 tcp sever start!");

    err = mico_rtos_init_semaphore( &fog_v2_close_tcp_server, 1 ); //0/1 binary semaphore || 0/N semaphore
    require_noerr( err, exit );

    /* Start TCP server listener thread*/
    err = mico_rtos_create_thread( NULL, MICO_APPLICATION_PRIORITY, "FOG TCP SERVER", fog_tcp_server_thread, 0x800, (uint32_t)NULL );
    require_noerr_string( err, exit, "ERROR: Unable to start the tcp server thread." );

 exit:
     if(err != kNoErr)
     {
         if(fog_v2_close_tcp_server != NULL)
         {
             mico_rtos_deinit_semaphore( &fog_v2_close_tcp_server );
         }
     }

    return err;
}

//tcp server close
OSStatus fog_tcp_server_close( void )
{
    if(fog_v2_tcp_server_is_start() == true)
    {
        if(fog_v2_close_tcp_server != NULL)
        {
            tcp_server_log("set sem to close tcp server!");
            return mico_rtos_set_semaphore(&fog_v2_close_tcp_server);
        }else
        {
            tcp_server_log("tcp server sem is error!");
            return kGeneralErr;
        }
    }else
    {
        tcp_server_log("tcp server has already closed!");
        return kNoErr;
    }
}
