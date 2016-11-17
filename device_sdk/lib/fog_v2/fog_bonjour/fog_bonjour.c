#include "mico.h"
#include "StringUtils.h"
#include "fog_bonjour.h"
#include "fog_v2_include.h"

#ifndef FOG_BONJOUR_SERVICE_NAME
    #error "FOG_BONJOUR_SERVICE_NAME is not define"
#endif

#ifndef FOG_BONJOUR_SERVICE_TTL
    #error "FOG_BONJOUR_SERVICE_TTL is not define"
#endif

#ifndef FOG_V2_TCP_SERVER_PORT
    #error "FOG_V2_TCP_SERVER_PORT is not define"
#endif

//开启fog Bonjour通知
//is_uncheck: 是否还未检查完成超级用户    in_fog_des:fog描述符指针
OSStatus start_fog_bonjour(bool is_uncheck, FOG_DES_S *in_fog_des)
{
	char *temp_txt = NULL;
	char *temp_txt2 = NULL;
	OSStatus err;
	net_para_st para;
	mdns_init_t init;
    mico_Context_t *in_context = NULL;

    in_context = mico_system_context_get();

	temp_txt = malloc(MDNS_TEXT_MAX_SIZE);
	require_action(temp_txt, exit, err = kNoMemoryErr);

	memset(&init, 0x0, sizeof(mdns_init_t));

	micoWlanGetIPStatus(&para, Station);

	init.service_name = FOG_BONJOUR_SERVICE_NAME; //mico_config.h文件中定义

	/*   name#xxxxxx.local.  */
	memset(temp_txt, 0, MDNS_TEXT_MAX_SIZE);
	snprintf(temp_txt, 100, "%s#%c%c%c%c%c%c.local.", in_context->flashContentInRam.micoSystemConfig.name, in_context->micoStatus.mac[9], in_context->micoStatus.mac[10], in_context->micoStatus.mac[12], in_context->micoStatus.mac[13], in_context->micoStatus.mac[15], in_context->micoStatus.mac[16]);
	init.host_name = (char*) __strdup(temp_txt);

	/*   name#xxxxxx.   */
	memset(temp_txt, 0, MDNS_TEXT_MAX_SIZE);
	snprintf(temp_txt, 100, "%s#%c%c%c%c%c%c", in_context->flashContentInRam.micoSystemConfig.name, in_context->micoStatus.mac[9], in_context->micoStatus.mac[10], in_context->micoStatus.mac[12], in_context->micoStatus.mac[13], in_context->micoStatus.mac[15], in_context->micoStatus.mac[16]);
	init.instance_name = (char*) __strdup(temp_txt);

	init.service_port = FOG_V2_TCP_SERVER_PORT;

	temp_txt2 = __strdup_trans_dot(in_context->micoStatus.mac);
	sprintf(temp_txt, "MAC=%s.", temp_txt2);
	free(temp_txt2);

	temp_txt2 = __strdup_trans_dot(in_fog_des->firmware);
	sprintf(temp_txt, "%sFirmware Rev=%s.", temp_txt, temp_txt2);
	free(temp_txt2);

	temp_txt2 = __strdup_trans_dot(in_fog_des->product_id);
	sprintf(temp_txt, "%sFogProductId=%s.", temp_txt, temp_txt2);
	free(temp_txt2);

    if(is_uncheck == true)
    {
        sprintf(temp_txt, "%sIsHaveSuperUser=UNCHECK.", temp_txt);
    }else
    {
        if (in_fog_des->is_hava_superuser == true)
            sprintf(temp_txt, "%sIsHaveSuperUser=true.", temp_txt);
        else
            sprintf(temp_txt, "%sIsHaveSuperUser=false.", temp_txt);

    }

	temp_txt2 = __strdup_trans_dot(MicoGetVer());
	sprintf(temp_txt, "%sMICO OS Rev=%s.", temp_txt, temp_txt2);
	free(temp_txt2);

	temp_txt2 = __strdup_trans_dot(MODEL);
	sprintf(temp_txt, "%sModel=%s.", temp_txt, temp_txt2);
	free(temp_txt2);

	temp_txt2 = __strdup_trans_dot(in_fog_des->fog_v2_lib_version);
	sprintf(temp_txt, "%sProtocol=%s.", temp_txt, temp_txt2);
	free(temp_txt2);

	init.txt_record = (char*) __strdup(temp_txt);

	mdns_add_record(init, Station, FOG_BONJOUR_SERVICE_TTL);

	free(init.host_name);
	free(init.instance_name);
	free(init.txt_record);

	exit:
       if (temp_txt != NULL)
       {
            free(temp_txt);
            temp_txt = NULL;
       }
	return err;
}

//停止fog Bonjour通知
void stop_fog_bonjour(void)
{
	mdns_suspend_record(FOG_BONJOUR_SERVICE_NAME, Station, true);
}

