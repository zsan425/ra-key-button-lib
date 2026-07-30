#include "led.h"

led_t g_led[LED_COUNT];

void led_init_all(void)
{
		R_IOPORT_Open (&g_ioport_ctrl, g_ioport.p_cfg);

    led_init(&g_led[0], LED1_PIN, LED_ACTIVE_LEVEL);
    led_init(&g_led[1], LED2_PIN, LED_ACTIVE_LEVEL);
}
