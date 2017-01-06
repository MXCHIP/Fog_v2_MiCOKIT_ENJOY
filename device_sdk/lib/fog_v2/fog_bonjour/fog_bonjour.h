#ifndef __FOG_BONJOUR_H_
#define __FOG_BONJOUR_H_

#include "fog_v2_config.h"
#include "../fog_http/fog_v2.h"

#define MDNS_TEXT_MAX_SIZE      (1024)

extern OSStatus start_fog_bonjour(bool is_uncheck, FOG_DES_S *in_fog_des);
extern void stop_fog_bonjour(void);

#endif

