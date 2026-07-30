#ifndef __BSP_LED_H__
#define __BSP_LED_H__

#include "hal_data.h"

typedef struct {
    bsp_io_port_pin_t  pin;
    bsp_io_level_t     active_level;   /* 点亮时的电平 */
} led_t;

void led_init(led_t *led, bsp_io_port_pin_t pin, bsp_io_level_t active_level);
void led_on(const led_t *led);
void led_off(const led_t *led);
void led_toggle(const led_t *led);
void led_write(const led_t *led, uint8_t on);

#endif
