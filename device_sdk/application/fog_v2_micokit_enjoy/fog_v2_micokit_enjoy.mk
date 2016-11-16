#
#  UNPUBLISHED PROPRIETARY SOURCE CODE
#  Copyright (c) 2016 MXCHIP Inc.
#
#  The contents of this file may not be disclosed to third parties, copied or
#  duplicated in any form, in whole or in part, without the prior written
#  permission of MXCHIP Corporation.
#

NAME := Lib_fog_v2_micokit_enjoy

$(NAME)_SOURCES := 	fogcloud_down_thread.c \
					fogcloud_up_thread.c \
					hello_fog.c \
					template_analysis.c \
					user_common.c \
					user_oled.c \
					user_template.c
					
$(NAME)_LINK_FILES := user_oled.o
															
$(NAME)_COMPONENTS := protocols/fog_v2