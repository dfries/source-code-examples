#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "user_config.h"
#include "user_interface.h"

#include <wpa2_enterprise.h>

#define user_procTaskPrio        0
#define user_procTaskQueueLen    1
os_event_t    user_procTaskQueue[user_procTaskQueueLen];
static void loop(os_event_t *events);

static void ICACHE_FLASH_ATTR
print_heap(os_event_t *events)
{
    int status = wifi_station_get_connect_status();
    os_printf("heap %u wifi status %u\n", system_get_free_heap_size(), status);
    if(status == STATION_IDLE)
    {
        os_printf("idle, try connecting\n");
        wifi_station_connect();
    }
    static int connect_count;
    if(status == STATION_CONNECTING)
    {
        ++connect_count;
        if(connect_count == 4)
        {
            os_printf("still in connecting, disconnect and try again\n");
            wifi_station_disconnect();
            wifi_station_connect();
            connect_count = 0;
        }
    }
}

//Init function 
void ICACHE_FLASH_ATTR
user_init()
{
    char ssid[32] = SSID;
    char password[64] = SSID_PASSWORD;
    struct station_config stationConf;

    // default appears to be a non-standard 74880 baud
    uart_div_modify(0, UART_CLK_FREQ / 115200);

    //Set station mode
    wifi_set_opmode(STATION_MODE);

    //Set ap settings
    // skip station config, expect the station is already setup in flash
    #if 0
    os_memcpy(&stationConf.ssid, ssid, 32);
    os_memcpy(&stationConf.password, password, 64);
    wifi_station_set_config(&stationConf);
    #endif

    os_printf("heap %u line %u\n", system_get_free_heap_size(), __LINE__);
    wifi_station_set_wpa2_enterprise_auth(true);
    wifi_station_set_reconnect_policy(false);
    wifi_station_set_enterprise_username((u8*)EAP_USER, os_strlen(EAP_USER));
    wifi_station_set_enterprise_password((u8*)EAP_PASSWORD, os_strlen(EAP_PASSWORD));
    os_printf("heap %u line %u\n", system_get_free_heap_size(), __LINE__);

    static os_timer_t heap_timer;
    os_timer_disarm(&heap_timer);
    os_timer_setfn(&heap_timer, &print_heap, NULL);
    os_timer_arm(&heap_timer, 3000, 1);
}
