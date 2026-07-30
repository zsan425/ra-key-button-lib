#include "bsp_led.h"

/* ── 初始化: 设置引脚和有效电平 ── */
void led_init(led_t *led, bsp_io_port_pin_t pin, bsp_io_level_t active_level)
{
    if (led == NULL) return;
    led->pin          = pin;
    led->active_level = active_level;
}

/* ── 点亮 ── */
void led_on(const led_t *led)
{
    if (led == NULL) return;
    R_IOPORT_PinWrite(&g_ioport_ctrl, led->pin, led->active_level);
}

/* ── 熄灭（写反电平）── */
void led_off(const led_t *led)
{
    bsp_io_level_t off;

    if (led == NULL) return;
    off = (led->active_level == BSP_IO_LEVEL_LOW)
          ? BSP_IO_LEVEL_HIGH : BSP_IO_LEVEL_LOW;
    R_IOPORT_PinWrite(&g_ioport_ctrl, led->pin, off);
}

/* ── 翻转: 读当前电平 → 写反电平 ── */
void led_toggle(const led_t *led)
{
    bsp_io_level_t cur;

    if (led == NULL) return;
    R_IOPORT_PinRead(&g_ioport_ctrl, led->pin, &cur);
    cur = (cur == BSP_IO_LEVEL_LOW) ? BSP_IO_LEVEL_HIGH : BSP_IO_LEVEL_LOW;
    R_IOPORT_PinWrite(&g_ioport_ctrl, led->pin, cur);
}

/* ── 写指定状态: 1=亮 0=灭 ── */
void led_write(const led_t *led, uint8_t on)
{
    if (led == NULL) return;
    on ? led_on(led) : led_off(led);
}
