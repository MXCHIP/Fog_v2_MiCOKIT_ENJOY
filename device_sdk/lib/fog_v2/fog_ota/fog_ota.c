#include "fog_v2_config.h"
#include "mico.h"
#include "debug.h"
#include "url.h"
#include "string.h"
#include "HTTPUtils.h"
#include "SocketUtils.h"
#include "StringUtils.h"

#define app_log(M, ...)             custom_log("FOG_OTA", M, ##__VA_ARGS__)


typedef struct _http_context_t{
  char *content;
  uint32_t content_length;
} http_context_t;


void fog_v2_ota(void);

static OSStatus parse_ota_respose(char *ota_res, char *ota_url, int32_t ota_url_len);
static OSStatus onReceivedData( struct _HTTPHeader_t * inHeader, uint32_t inPos, uint8_t * inData, size_t inLen, void * inUserContext );
static void fog_v2_ota_finish(void);

void fog_ota_thread(mico_thread_arg_t args);

static mico_semaphore_t ota_sem = NULL;
static uint32_t ota_reveive_index = 0;
static uint32_t ota_file_len = 0;
static unsigned char ota_file_md5[128] = {0};
static bool http_read_file_success = false;

const char *device_ota_download = "GET /%s HTTP/1.1\r\n\
Host: %s\r\n\
Connection: Keepalive\r\n\
Range: bytes=%d-\r\n\
Accept-Encoding: identity\r\n\r\n";

//字符串中内容小写转大写 IAR的string.h中无此函数 手动实现
static unsigned char* user_strupr(unsigned char* szMsg)
{
    unsigned char *pcMsg = NULL;

    for (pcMsg = szMsg; ('\0' != *pcMsg); pcMsg++)
    {
        if (('a' <= *pcMsg) && (*pcMsg <= 'z'))
        {
            *pcMsg += ('A' - 'a');
        }
    }

    return szMsg;
}

//src:输入    dest:输出     dest_size:最大输出长度
bool user_str2hex(unsigned char *src, uint8_t *dest, uint32_t dest_size)
{
    unsigned char hb = 0;
    unsigned char lb = 0;
    uint32_t i = 0, j = 0;
    uint32_t src_size = strlen((const char *)src);

    if ( (src_size % 2 != 0) || (src_size <= 0))
        return false;

    src = user_strupr( src );

    for ( i = 0; i < src_size; i ++ )
    {
        if(i > dest_size * 2)
            return false;

        hb = src[i];
        if ( hb >= 'A' && hb <= 'F' )
            hb = hb - 'A' + 10;
        else if ( hb >= '0' && hb <= '9' )
            hb = hb - '0';
        else
            return false;

        i++;
        lb = src[i];
        if ( lb >= 'A' && lb <= 'F' )
            lb = lb - 'A' + 10;
        else if ( lb >= '0' && lb <= '9' )
            lb = lb - '0';
        else
            return false;

        dest[j++] = (hb << 4) | (lb);
    }

    return true;
}


static OSStatus parse_ota_respose(char *ota_res, char *ota_url, int32_t ota_url_len)
{
    OSStatus err = kGeneralErr;
    json_object *http_body_json_obj = NULL, *data_json_obj = NULL, *file_json_obj = NULL, *product_id_json_obj = NULL, *file_url_json_obj = NULL, *file_md5_json_obj = NULL;
    const char *product_id_string = NULL, *ota_url_string = NULL, *ota_md5_string = NULL;
    int ret = 0;
    FOG_DES_S *in_fog_des = NULL;

    in_fog_des = get_fog_des_g();

    require_string(in_fog_des != NULL, exit, "in_fog_des is NULL");
    require_string(ota_res != NULL, exit, "ota_res is NULL");
    require_string(ota_url != NULL, exit, "ota_url is NULL");

    require_string(((*ota_res == '{') && (*(ota_res + strlen(ota_res) - 1) == '}')), exit, "ota_res JSON format error");

    http_body_json_obj = json_tokener_parse(ota_res);
    require_string(http_body_json_obj != NULL, exit, "json_tokener_parse error");

    data_json_obj = json_object_object_get(http_body_json_obj, "data");
    require_string(data_json_obj != NULL, exit, "get data error!");

    file_json_obj = json_object_object_get(data_json_obj, "File");
    require_string(file_json_obj != NULL, exit, "get File error!");


    //获取产品ID
    product_id_json_obj = json_object_object_get(file_json_obj, "productid");
    require_string(product_id_json_obj != NULL, exit, "get productid error!");

    product_id_string = json_object_get_string(product_id_json_obj);
    require_string(product_id_string != NULL, exit, "get product_id_json_obj error!");

    ret = strcmp(product_id_string, in_fog_des->product_id);
    require_string(ret == 0, exit, "ota failed, product is not right");    //校验产品ID

    //获取url
    file_url_json_obj = json_object_object_get(file_json_obj, "fileurl");
    require_string(file_url_json_obj != NULL, exit, "get fileurl error!");

    ota_url_string = json_object_get_string(file_url_json_obj);
    require_string(ota_url_string != NULL, exit, "get file_url_json_obj error!");

    require_string(strlen(ota_url_string) <= ota_url_len, exit, "ota_url_len is too small");
    memcpy(ota_url, ota_url_string, strlen(ota_url_string));

    //获取md5
    file_md5_json_obj = json_object_object_get(file_json_obj, "md5");
    require_string(file_md5_json_obj != NULL, exit, "get code error!");

    ota_md5_string = json_object_get_string(file_md5_json_obj);
    require_string(ota_md5_string != NULL, exit, "get file_url_json_obj error!");

    require_string(strlen(ota_md5_string) <= sizeof(ota_file_md5), exit, "ota_md5_len is too small");
    memset(ota_file_md5, 0, sizeof(ota_file_md5));
    memcpy(ota_file_md5, ota_md5_string, strlen(ota_md5_string));

    err = kNoErr;
    app_log("ota_url:%s", ota_url);
    app_log("ota_file_md5:%s", ota_file_md5);

 exit:

    json_object_put(http_body_json_obj);     //只需要销毁根节点
    http_body_json_obj = NULL;

    if(err != kNoErr)
    {
        app_log("OTA response:%s", ota_res);
    }

    return err;
}

//判断path路径来决定是否需要更新    kGeneralErr:不需要  kNoErr:需要
bool process_path(char *path)
{
    char *version_p = NULL;
    char *end_p = NULL;
    char firmware[64] = {0};
    int result = 0;

    sprintf(firmware, "%s%s.bin", FOG_V2_REPORT_VER, FOG_V2_REPORT_VER_NUM);

    version_p = strstr(path, FOG_V2_REPORT_VER);
    if(version_p == NULL)
    {
        app_log("OTA file name error!!!");
        return false;
    }

    end_p = strchr(version_p, '.');
    if(end_p == NULL)
    {
        app_log("OTA file name error!");
        return false;
    }


// 若str1=str2，则返回零；若str1<str2，则返回负数；若str1>str2，则返回正数。
   result = strcmp(version_p, firmware);

   if(result <= 0)
   {
       app_log("[NO OTA]fog version:%s, local version:%s", version_p, firmware);
       return false;
   }else
   {
       app_log("[OK OTA]fog version:%s, local version:%s", version_p, firmware);

       memset(get_fog_des_g()->firmware, 0, sizeof(get_fog_des_g()->firmware));
       memcpy(get_fog_des_g()->firmware, version_p, end_p - version_p);

       app_log("new version:%s", get_fog_des_g()->firmware);

       return true;
   }
}

//软件版本号回滚到系统设置
static void firmware_version_rollback(void)
{
    memset(get_fog_des_g()->firmware, 0, sizeof(get_fog_des_g()->firmware));
    memcpy(get_fog_des_g()->firmware, FOG_V2_REPORT_VER, strlen(FOG_V2_REPORT_VER));
    memcpy(get_fog_des_g()->firmware + strlen(get_fog_des_g()->firmware), FOG_V2_REPORT_VER_NUM, strlen(FOG_V2_REPORT_VER_NUM));   //设置设备软件版本号

    mico_system_context_update(mico_system_context_get());

    return;
}

//fog v2 OTA
void fog_v2_ota(void)
{
    char *ota_res = NULL, *ota_url = NULL;
    OSStatus err = kGeneralErr;
    url_field_t *url = NULL;
    bool need_update = false;

    ota_res = malloc(OTA_RES_LEN_MAX);
    require_string(ota_res != NULL, exit, "malloc error");

    ota_url = malloc(OTA_URL_LEN_MAX);
    require_string(ota_url != NULL, exit, "malloc error");

    memset(ota_res, 0, OTA_RES_LEN_MAX);
    memset(ota_url, 0, OTA_URL_LEN_MAX);

    err = fog_v2_ota_check(ota_res, OTA_RES_LEN_MAX, &need_update);
    require_noerr( err, exit );

    if(need_update == false)
    {
        app_log("NO OTA EXIST!");
        goto exit;
    }

    err = parse_ota_respose(ota_res, ota_url, OTA_URL_LEN_MAX);
    require_noerr( err, exit );

    //解析url
    url = url_parse(ota_url);
    require_string( url != NULL, exit, "url_parse() error");

    //创建新线程进行升级
    if(strcmp(url->schema, "https") == 0)
    {
        //...
    }else if(strcmp(url->schema, "http") == 0)
    {
        app_log("http schema is http !!!");
         goto exit;
    }else
    {
        app_log("http schema error!");
        goto exit;
    }

    if(false == process_path(url->path))     //判断bin文件名称
    {
        app_log("[NOTICE]ota file name error or version is too low!");
        goto exit;
    }

    err = mico_rtos_init_semaphore( &ota_sem, 1 );//0/1 binary semaphore || 0/N semaphore
    require_noerr( err, exit );

    /* Create a new thread */
    err = mico_rtos_create_thread( NULL, MICO_APPLICATION_PRIORITY, "FOG_V2_OTA", fog_ota_thread, 0x2800, (uint32_t)url );
    require_noerr_string( err, exit, "ERROR: Unable to start the fog_init thread" );

    mico_rtos_get_semaphore( &ota_sem, MICO_WAIT_FOREVER );//wait until get semaphore

    if(http_read_file_success == true)
    {
        fog_v2_ota_finish();
    }

 exit:
    if(http_read_file_success == false)
    {
        firmware_version_rollback();
    }

    if( ota_sem != NULL )
    {
        mico_rtos_deinit_semaphore( &ota_sem );
    }

    url_free(url); //释放url

    if(ota_res != NULL)
    {
        free(ota_res);
        ota_res = NULL;
    }

    if(ota_url != NULL)
    {
        free(ota_url);
        ota_url = NULL;
    }

    return;
}

//OSStatus fog_mico_ota_updated(int filelen, uint16_t crc)
//{
//#ifdef MICO_KERNEL
//    extern int switch_active_firmware(void);
//    switch_active_firmware();
//#else
//	mico_logic_partition_t* ota_partition = MicoFlashGetInfo( MICO_PARTITION_OTA_TEMP );
//    mico_Context_t* context = NULL;
//
//	context = mico_system_context_get( );
//    memset(&context->flashContentInRam.bootTable, 0, sizeof(boot_table_t));
//    context->flashContentInRam.bootTable.length = filelen;
//    context->flashContentInRam.bootTable.start_address = ota_partition->partition_start_addr;
//    context->flashContentInRam.bootTable.type = 'A';
//    context->flashContentInRam.bootTable.upgrade_type = 'U';
//    context->flashContentInRam.bootTable.crc = crc;
//    mico_system_context_update( mico_system_context_get( ) );
//#endif
//	return kNoErr;
//}


static void fog_v2_ota_finish(void)
{
    OSStatus err = kGeneralErr;
	md5_context ctx;
    uint8_t md5_calc[16] = {0};
    uint8_t md5_recv[16] = {0};
    uint16_t crc = 0;
    CRC16_Context crc16_contex;
    uint8_t *bin_buf = NULL;
    uint32_t read_index = 0;
    uint32_t file_len = ota_file_len;
    uint32_t need_read_len = 0;
    bool ret = 0;

    require_string(MicoFlashGetInfo( MICO_PARTITION_OTA_TEMP )->partition_owner != MICO_FLASH_NONE, exit, "OTA storage is not exist");

	InitMd5(&ctx);
    CRC16_Init( &crc16_contex );

    bin_buf = malloc(BIN_BUFF_LEN);
    require_string(bin_buf != NULL, exit, "malloc bin_buff failed");

    while(1)
	{
        if(file_len - read_index >=  BIN_BUFF_LEN)
        {
            need_read_len = BIN_BUFF_LEN;
        }else
        {
            need_read_len = file_len - read_index;
        }

		err = MicoFlashRead(MICO_PARTITION_OTA_TEMP, &read_index, bin_buf, need_read_len);
        require_noerr(err, exit);

		Md5Update(&ctx, bin_buf, need_read_len);
        CRC16_Update( &crc16_contex, bin_buf, need_read_len );

        if((read_index == ota_file_len) && (read_index != 0))
        {
            break;
        }
	}

	Md5Final(&ctx, md5_calc);
    CRC16_Final( &crc16_contex, &crc );

    app_log("FLASH READ: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
            md5_calc[0],md5_calc[1],md5_calc[2],md5_calc[3],
            md5_calc[4],md5_calc[5],md5_calc[6],md5_calc[7],
            md5_calc[8],md5_calc[9],md5_calc[10],md5_calc[11],
            md5_calc[12],md5_calc[13],md5_calc[14],md5_calc[15]);

    //str2hex(ota_file_md5, md5_recv, sizeof(md5_recv));
    ret = user_str2hex(ota_file_md5, md5_recv, sizeof(md5_recv));
    require_action_string(ret == true, exit, err = kGeneralErr, "user_str2hex() is error");

    if ( memcmp( md5_recv, md5_calc, sizeof(md5_recv) ) == 0 )
    {
        fog_v2_ota_upload_log( );
        fog_des_clean( );

        err = mico_ota_switch_to_new_fw( ota_file_len, crc );
        require_noerr( err, exit );

        app_log( "OTA SUCCESS. Rebooting...\r\n" );

        while ( 1 )
        {
            mico_system_power_perform( mico_system_context_get( ), eState_Software_Reset );
            mico_thread_sleep(100);
        }
    }else
    {
        app_log("ERROR!! MD5 Error.");
        app_log("HTTP RECV:   %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
                 md5_recv[0],md5_recv[1],md5_recv[2],md5_recv[3],
                 md5_recv[4],md5_recv[5],md5_recv[6],md5_recv[7],
                 md5_recv[8],md5_recv[9],md5_recv[10],md5_recv[11],
                 md5_recv[12],md5_recv[13],md5_recv[14],md5_recv[15]);
    }

 exit:
    if(bin_buf != NULL)
    {
        free(bin_buf);
        bin_buf = NULL;
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
static int ota_set_tcp_keepalive(int socket, int send_timeout, int recv_timeout, int idle, int interval, int count)
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

//OTA默认只支持SSL类型
void fog_ota_thread(mico_thread_arg_t args)
{
    url_field_t *url = (url_field_t *)args;
    char *host_name = url->host;
    char *path = url->path;
    OSStatus err = kGeneralErr;
    int ota_fd = -1;
    int ssl_errno = 0;
    int ret = 0;
    mico_ssl_t client_ssl = NULL;
    fd_set readfds;
    char ipstr[20] = {0};
    struct sockaddr_in addr;
    HTTPHeader_t *httpHeader = NULL;
    http_context_t context = { NULL, 0 };
    struct timeval t = {0, OTA_YIELD_TMIE*1000};
    char ota_http_requeset[512] = {0}; //512Byte 足够使用了
    bool file_len_right = false;

    ota_reveive_index = 0;   //ota已接收文件长度清零

    require_string(MicoFlashGetInfo( MICO_PARTITION_OTA_TEMP )->partition_owner != MICO_FLASH_NONE, exit, "OTA storage is not exist");

    app_log("erase MICO_PARTITION_OTA_TEMP falsh start");
    err = MicoFlashErase(MICO_PARTITION_OTA_TEMP, 0, MicoFlashGetInfo( MICO_PARTITION_OTA_TEMP )->partition_length);
    require_noerr(err, quit_thread);
    app_log("erase MICO_PARTITION_OTA_TEMP falsh success");

    http_read_file_success = false;

 OTA_START:
    memset(ipstr, 0, sizeof(ipstr));

    app_log("usergethostbyname start!");
    err = usergethostbyname(host_name, (uint8_t *)ipstr, sizeof(ipstr));
    require_action( err == kNoErr, exit, app_log("usergethostbyname() error"));
    app_log("OTA server address: host:%s, ip: %s", host_name, ipstr);

    /*HTTPHeaderCreateWithCallback set some callback functions */
    httpHeader = HTTPHeaderCreateWithCallback( OTA_RESPONSE_BODY_MAX_LEN, onReceivedData, NULL, &context );
    require_action( httpHeader, exit, err = kNoMemoryErr );

    ota_fd = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
    require_action(IsValidSocket( ota_fd ), exit, err = kNoResourcesErr );

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr( ipstr );
    addr.sin_port = htons(OTA_PORT_SSL); //SSL端口 443

    ret = ota_set_tcp_keepalive(ota_fd, OTA_SEND_TIME_OUT, OTA_RECV_TIME_OUT, OTA_KEEP_IDLE_TIME, OTA_KEEP_INTVL_TIME, OTA_KEEP_COUNT);
    if(ret < 0)
    {
        app_log("user_set_tcp_keepalive() error");
        goto exit;
    }

    err = connect( ota_fd, (struct sockaddr *)&addr, sizeof(addr) );
    require_action(err == kNoErr, exit, err = kGeneralErr );

    //ssl_version_set(TLS_V1_2_MODE);    //设置SSL版本
    ssl_set_client_version(TLS_V1_2_MODE);

    client_ssl = ssl_connect( ota_fd, 0, NULL, &ssl_errno );
    require_action( client_ssl != NULL, quit_thread, {err = kGeneralErr; app_log("OTA ssl_connnect error, errno = %d", ssl_errno);} );

    app_log("#####OTA connect#####:num_of_chunks:%d, free:%d", MicoGetMemoryInfo()->num_of_chunks, MicoGetMemoryInfo()->free_memory);

    memset(ota_http_requeset, 0, sizeof(ota_http_requeset));
    sprintf(ota_http_requeset, device_ota_download, path, host_name, ota_reveive_index);
    if(ota_reveive_index == 0)
    {
        file_len_right = true;
    }else
    {
        file_len_right = false;
    }

    ret = ssl_send( client_ssl, ota_http_requeset, strlen((const char *)ota_http_requeset) );       /* Send HTTP Request */
    if(ret > 0)
    {
       app_log("OTA ssl_send success [%d] [%d]", strlen((const char *)ota_http_requeset) ,ret);
       app_log("\r\n%s", ota_http_requeset);
    }else
    {
       app_log("OTA ssl_send error, ret = %d", ret);
       err = kGeneralErr;
       goto exit;
    }

    FD_ZERO( &readfds );
    FD_SET( ota_fd, &readfds );

    ret = select( ota_fd + 1, &readfds, NULL, NULL, &t );
    if(ret <= 0)
    {
       app_log("select error, ret = %d", ret);
       err = kGeneralErr;
       goto exit;
    }

    if( FD_ISSET( ota_fd, &readfds ) )
    {
        /*parse header*/
        err = SocketReadHTTPSHeader( client_ssl, httpHeader );
        switch ( err )
        {
         case kNoErr:
            {
                //只有code正确才解析返回数据,错误情况下解析容易造成内存溢出
                if((httpHeader->statusCode == 200) || (httpHeader->statusCode == 206))
                {
                    if(file_len_right == true)
                    {
                        ota_file_len = httpHeader->contentLength;  //实际文件长度
                    }

                    PrintHTTPHeader( httpHeader );
                    err = SocketReadHTTPSBody( client_ssl, httpHeader );    /*get body data*/
                    require_noerr( err, exit );
                }else
                {
                    app_log( "[ERR]fog http response error, statusCode = %d !!!", httpHeader->statusCode);
                    break;
                }

                break;
            }
         case EWOULDBLOCK:
            {
                app_log("SocketReadHTTPSHeader EWOULDBLOCK");
                break;
            }
         case kNoSpaceErr:
            {
                app_log("SocketReadHTTPSHeader kNoSpaceErr");
                break;
            }
         case kConnectionErr:
            {
                app_log("SocketReadHTTPSHeader kConnectionErr");
                break;
            }
         default:
            {
                app_log("ERROR: HTTP Header parse error: %d", err);
                break;
            }
        }
    }

 exit:
    if( client_ssl != NULL)
    {
        ssl_close( client_ssl );
        client_ssl = NULL;
    }

    SocketClose( &ota_fd );

    HTTPHeaderDestory( &httpHeader );

    if(err != kNoErr)
    {
        mico_thread_msleep(300);
        goto OTA_START;
    }

 quit_thread:
     if( client_ssl != NULL)
     {
         ssl_close( client_ssl );
         client_ssl = NULL;
     }

     SocketClose( &ota_fd );

     HTTPHeaderDestory( &httpHeader );

     if(err == kNoErr)
     {
         http_read_file_success = true;
     }else{
         http_read_file_success = false;
     }

    mico_rtos_set_semaphore( &ota_sem );
    mico_rtos_delete_thread(NULL);
    return;
}

void print_hex(uint8_t *data, uint32_t len)
{
    uint32_t i = 0;

    for(i = 0; i < len; i++)
    {
        printf("%02X", *data);
        data ++;
    }

    printf("\r\n");
}


/*one request may receive multi reply*/
static OSStatus onReceivedData( struct _HTTPHeader_t * inHeader, uint32_t inPos, uint8_t * inData, size_t inLen, void * inUserContext )
{
    OSStatus err = kNoErr;
    mico_logic_partition_t  *ota_partition = MicoFlashGetInfo(MICO_PARTITION_OTA_TEMP);


    app_log("ota_file_len:%ld, inPos:%ld, inLen:%d, ota_reveive_index:%ld", ota_file_len, inPos, inLen, ota_reveive_index);

    if(ota_file_len != 0)
    {
        if((ota_reveive_index + inLen) > ota_file_len)
        {
            app_log("[ERROR]ota_file_len:%ld, ota_reveive_index:%ld, inLen:%d", ota_file_len, ota_reveive_index, inLen);
        }else if((ota_reveive_index + inLen) == ota_file_len)
        {
            app_log("[SUCCESS]ota_file_len:%ld, ota_reveive_index:%ld, inLen:%d", ota_file_len, ota_reveive_index, inLen);
        }
    }

    //copy数据到flash
    require_string(ota_partition->partition_owner != MICO_FLASH_NONE, exit, "OTA storage is not exist");

    err = MicoFlashWrite(MICO_PARTITION_OTA_TEMP, &ota_reveive_index, (uint8_t *) inData, inLen);  //写成功之后内部会对ota_reveive_len 加上写入的长度
	require_noerr(err, exit);

 exit:
    return err;
}
