#include "mico.h"
#include "micokit_ext.h"

#define app_log(M, ...)     custom_log("MAIN_FOG", M, ##__VA_ARGS__)
#define app_log_trace()     custom_log_trace("MAIN_FOG")


#define SYS_LED_TRIGGER_INTERVAL  100
#define SYS_LED_TRIGGER_INTERVAL_AFTER_EASYLINK  500

static mico_timer_t _Led_EL_timer;
static void _led_EL_Timeout_handler( void* arg );


static void _led_EL_Timeout_handler( void* arg )
{
  (void)(arg);
  MicoGpioOutputTrigger((mico_gpio_t)MICO_SYS_LED);
}

void mico_system_delegate_config_will_start( void )
{
#ifdef USE_MiCOKit_EXT
  char oled_show_line[OLED_DISPLAY_MAX_CHAR_PER_ROW+1] = {'\0'};
#endif

  /*Led trigger*/
  mico_init_timer(&_Led_EL_timer, SYS_LED_TRIGGER_INTERVAL, _led_EL_Timeout_handler, NULL);
  mico_start_timer(&_Led_EL_timer);
#ifdef USE_MiCOKit_EXT
  memset(oled_show_line, '\0', OLED_DISPLAY_MAX_CHAR_PER_ROW+1);
  snprintf(oled_show_line, OLED_DISPLAY_MAX_CHAR_PER_ROW+1, "%s", (char *)"Config...       ");
  OLED_ShowString(OLED_DISPLAY_COLUMN_START, OLED_DISPLAY_ROW_2, (char *)oled_show_line);
  OLED_ShowString(OLED_DISPLAY_COLUMN_START, OLED_DISPLAY_ROW_3, (char *)"  Awaiting      ");
  OLED_ShowString(OLED_DISPLAY_COLUMN_START, OLED_DISPLAY_ROW_4, (char *)"    ssid/key    ");
#endif

  return;
}

void mico_system_delegate_config_will_stop( void )
{
#ifdef USE_MiCOKit_EXT
  char oled_show_line[OLED_DISPLAY_MAX_CHAR_PER_ROW+1] = {'\0'};
#endif

  mico_stop_timer(&_Led_EL_timer);
  mico_deinit_timer( &_Led_EL_timer );
  MicoSysLed(true);

#ifdef USE_MiCOKit_EXT
  memset(oled_show_line, '\0', OLED_DISPLAY_MAX_CHAR_PER_ROW+1);
  snprintf(oled_show_line, OLED_DISPLAY_MAX_CHAR_PER_ROW+1, "%s", (char *)"Config          ");
  OLED_ShowString(OLED_DISPLAY_COLUMN_START, OLED_DISPLAY_ROW_2, (char *)oled_show_line);
  OLED_ShowString(OLED_DISPLAY_COLUMN_START, OLED_DISPLAY_ROW_3, (char *)"    Stop.       ");
  OLED_ShowString(OLED_DISPLAY_COLUMN_START, OLED_DISPLAY_ROW_4, (char *)"                ");
#endif

  return;
}


void mico_system_delegate_config_recv_ssid ( char *ssid, char *key )
{
#ifdef USE_MiCOKit_EXT
  char oled_show_line[OLED_DISPLAY_MAX_CHAR_PER_ROW+1] = {'\0'};
#endif

  mico_stop_timer(&_Led_EL_timer);
  mico_deinit_timer( &_Led_EL_timer );
  mico_init_timer(&_Led_EL_timer, SYS_LED_TRIGGER_INTERVAL_AFTER_EASYLINK, _led_EL_Timeout_handler, NULL);
  mico_start_timer(&_Led_EL_timer);

#ifdef USE_MiCOKit_EXT
  memset(oled_show_line, '\0', OLED_DISPLAY_MAX_CHAR_PER_ROW+1);
  snprintf(oled_show_line, OLED_DISPLAY_MAX_CHAR_PER_ROW+1, "%s", (char *)"Got ssid/key    ");
  OLED_ShowString(OLED_DISPLAY_COLUMN_START, OLED_DISPLAY_ROW_2, (char *)oled_show_line);
  memset(oled_show_line, '\0', OLED_DISPLAY_MAX_CHAR_PER_ROW+1);
  snprintf(oled_show_line, OLED_DISPLAY_MAX_CHAR_PER_ROW+1, "%16s", ssid);
  OLED_ShowString(OLED_DISPLAY_COLUMN_START, OLED_DISPLAY_ROW_3, (char *)oled_show_line);
  memset(oled_show_line, '\0', OLED_DISPLAY_MAX_CHAR_PER_ROW+1);
  snprintf(oled_show_line, OLED_DISPLAY_MAX_CHAR_PER_ROW+1, "%16s", key);
  OLED_ShowString(OLED_DISPLAY_COLUMN_START, OLED_DISPLAY_ROW_4, (char *)oled_show_line);
#endif
}

USED void mico_system_delegate_config_success( mico_config_source_t source )
{
#ifdef USE_MiCOKit_EXT
  char oled_show_line[OLED_DISPLAY_MAX_CHAR_PER_ROW+1] = {'\0'};
#endif

  app_log( "Wi-Fi configed by: %d", source );
  MicoSysLed(true);

#ifdef USE_MiCOKit_EXT
  memset(oled_show_line, '\0', OLED_DISPLAY_MAX_CHAR_PER_ROW+1);
  if(CONFIG_BY_AIRKISS == source){
    snprintf(oled_show_line, OLED_DISPLAY_MAX_CHAR_PER_ROW+1, "%s", (char *)"Airkiss success ");
  }else if(CONFIG_BY_SOFT_AP == source){
    snprintf(oled_show_line, OLED_DISPLAY_MAX_CHAR_PER_ROW+1, "%s", (char *)"SoftAP success  ");
  }else if(CONFIG_BY_WAC == source){
    snprintf(oled_show_line, OLED_DISPLAY_MAX_CHAR_PER_ROW+1, "%s", (char *)"WAC success     ");
  }else if( (CONFIG_BY_EASYLINK_V2 == source) || (CONFIG_BY_EASYLINK_PLUS == source) ||
           (CONFIG_BY_EASYLINK_MINUS == source) ){
    snprintf(oled_show_line, OLED_DISPLAY_MAX_CHAR_PER_ROW+1, "%s", (char *)"Easylink success");
  }else{
    snprintf(oled_show_line, OLED_DISPLAY_MAX_CHAR_PER_ROW+1, "%s", (char *)"Unknown         ");
  }
  OLED_ShowString(OLED_DISPLAY_COLUMN_START, OLED_DISPLAY_ROW_2, (char *)oled_show_line);
  OLED_ShowString(OLED_DISPLAY_COLUMN_START, OLED_DISPLAY_ROW_3, (char *)"                ");
  OLED_ShowString(OLED_DISPLAY_COLUMN_START, OLED_DISPLAY_ROW_4, (char *)"                ");
#endif

  return;
}
