#
#  UNPUBLISHED PROPRIETARY SOURCE CODE
#  Copyright (c) 2016 MXCHIP Inc.
#
#  The contents of this file may not be disclosed to third parties, copied or
#  duplicated in any form, in whole or in part, without the prior written
#  permission of MXCHIP Corporation.
#

NAME := Lib_fog_v2


$(NAME)_SOURCES :=	fog_bonjour/fog_bonjour.c \
					fog_http/fog_http_client.c \
					fog_http/fog_v2_sub.c \
					fog_http/fog_v2_sub_device_management.c \
					fog_http/fog_v2.c \
					fog_http/fog_v2_user_notification.c \
					fog_mqtt/fog_mqtt.c \
					fog_mqtt/fog_process_mqtt_cmd.c \
					fog_ota/fog_ota.c \
					fog_ota/fog2_ota_notification.c \
					fog_ota/url.c \
					fog_tcp_server/fog_tcp_server.c
					
										
GLOBAL_INCLUDES += 	.

$(NAME)_COMPONENTS := protocols/mqtt	#MICOSDK_#3.2
	