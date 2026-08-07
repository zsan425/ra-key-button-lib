#include "hal_data.h"

FSP_CPP_HEADER
void R_BSP_WarmStart(bsp_warm_start_event_t event);
FSP_CPP_FOOTER

#include "led/led.h"
#include "key_button/key.h"
#include "uart/uart.h"
#include "eeprom/eeprom.h"
#include "adc/adc.h"
#include "dht11/dht11.h"

void hal_entry(void)
{
    led_init_all();
    led_off(LED_1);
    led_off(LED_2);

    key_init();
    uart_init();
    eeprom_init();
    adc_init();
    dht11_init();

    printf("\r\n=== 启明6T2 调试串口 ===\r\n");
    printf("输入 help 查看指令\r\n");

    while (1)
    {
        /* ── 按键 ── */
        key_scan();
        uint8_t ev1 = key_button_get_event(KEY_1);
        uint8_t ev2 = key_button_get_event(KEY_2);
			
        if (ev1 & KEY_EVENT_CLICK)        led_toggle(LED_1);
        if (ev1 & KEY_EVENT_DOUBLE_CLICK) led_toggle(LED_2);
        if (ev1 & KEY_EVENT_LONG_PRESS) { led_off(LED_1); led_off(LED_2); }

        if (ev2 & KEY_EVENT_CLICK)       led_on(LED_2);
        if (ev2 & KEY_EVENT_LONG_REPEAT) led_toggle(LED_2);

        /* ── 串口命令（非阻塞）── */
        if (uart_cmd_ready) {
            uart_process_cmd();
        }
    }

#if BSP_TZ_SECURE_BUILD
    R_BSP_NonSecureEnter();
#endif
}

void R_BSP_WarmStart(bsp_warm_start_event_t event)
{
    if (BSP_WARM_START_RESET == event) {
#if BSP_FEATURE_FLASH_LP_VERSION != 0
        R_FACI_LP->DFLCTL = 1U;
#endif
    }
    if (BSP_WARM_START_POST_C == event) {
        R_IOPORT_Open(&g_ioport_ctrl, g_ioport.p_cfg);
    }
}

#if BSP_TZ_SECURE_BUILD
BSP_CMSE_NONSECURE_ENTRY void template_nonsecure_callable() {}
#endif
