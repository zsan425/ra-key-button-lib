#ifndef __LED_H__
#define __LED_H__

#include "bsp_led.h"

/* ── 启明6T2: LED1=PD1, LED2=PD3, 低电平亮 ── */
#define LED1_PIN         BSP_IO_PORT_13_PIN_01
#define LED2_PIN         BSP_IO_PORT_13_PIN_03
#define LED_ACTIVE_LEVEL BSP_IO_LEVEL_LOW

#define LED_COUNT  2

extern led_t g_led[LED_COUNT];
#define LED_1  (&g_led[0])
#define LED_2  (&g_led[1])

void led_init_all(void);

#endif
