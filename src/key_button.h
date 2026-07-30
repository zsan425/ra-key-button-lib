#ifndef __KEY_BUTTON_H__
#define __KEY_BUTTON_H__

#include "hal_data.h"
#include <stdint.h>

/* ── 事件位掩码 ── */
#define KEY_EVENT_PRESS         0x01   /* 按下（消抖后触发）     */
#define KEY_EVENT_RELEASE       0x02   /* 释放（消抖后触发）     */
#define KEY_EVENT_CLICK         0x04   /* 单击                   */
#define KEY_EVENT_DOUBLE_CLICK  0x08   /* 双击                   */
#define KEY_EVENT_LONG_PRESS    0x10   /* 长按（触发一次）       */
#define KEY_EVENT_LONG_REPEAT   0x20   /* 连发（长按后周期触发） */

/* ── 配置参数（均可运行时修改）── */
typedef struct {
    uint16_t debounce_ms;          /* 消抖时间，默认 20          */
    uint16_t long_press_ms;        /* 长按阈值，默认 1000，0=禁用 */
    uint16_t repeat_interval_ms;   /* 连发间隔，默认 200，0=禁用  */
    uint16_t double_click_ms;      /* 双击窗口，默认 400，0=禁用  */
} key_button_cfg_t;

/* ── 状态机状态 ── */
typedef enum {
    KEY_ST_IDLE = 0,
    KEY_ST_PRESS_DEBOUNCE,
    KEY_ST_PRESSED,
    KEY_ST_RELEASE_DEBOUNCE,
    KEY_ST_WAIT_DOUBLE,
} key_button_state_t;

/* ── 按键句柄 ── */
typedef struct {
    key_button_cfg_t   cfg;
    bsp_io_port_pin_t  pin;
    bsp_io_level_t     active_level;

    /* 内部状态，不要手动改 */
    key_button_state_t state;
    uint8_t  stable_level;
    uint8_t  raw_last;
    uint32_t debounce_tick;
    uint32_t press_tick;
    uint32_t release_tick;
    uint8_t  click_cnt;
    uint8_t  long_triggered : 1;
    uint32_t repeat_next_tick;
    uint8_t  events;
} key_button_t;

/* ── API ── */
void     key_button_init(key_button_t *btn, bsp_io_port_pin_t pin,
                         bsp_io_level_t active_level);
void     key_button_tick(key_button_t *btn, uint32_t sys_tick);
uint8_t  key_button_get_event(key_button_t *btn);
uint8_t  key_button_is_pressed(const key_button_t *btn);
uint32_t key_button_get_hold_ms(const key_button_t *btn, uint32_t sys_tick);

#endif
