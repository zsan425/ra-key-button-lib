#include "key_button.h"

#define DEBOUNCE_IDLE  0xFFFFFFFFUL

/* ── 读引脚，归一化为 0(松开) / 1(按下) ── */
static uint8_t read_pin(const key_button_t *btn)
{
    bsp_io_level_t level;
    R_IOPORT_PinRead(&g_ioport_ctrl, btn->pin, &level);
    return (level == btn->active_level) ? 1 : 0;
}

/* ── 初始化: 设引脚 + 默认参数 + 清零状态 ── */
void key_button_init(key_button_t *btn, bsp_io_port_pin_t pin,
                     bsp_io_level_t active_level)
{
    if (btn == NULL) return;

    btn->pin          = pin;
    btn->active_level = active_level;

    btn->cfg.debounce_ms         = 20;
    btn->cfg.long_press_ms       = 1000;
    btn->cfg.repeat_interval_ms  = 200;
    btn->cfg.double_click_ms     = 400;

    btn->state           = KEY_ST_IDLE;
    btn->stable_level    = 0;
    btn->raw_last        = 0;
    btn->debounce_tick   = DEBOUNCE_IDLE;
    btn->press_tick      = 0;
    btn->release_tick    = 0;
    btn->click_cnt       = 0;
    btn->long_triggered  = 0;
    btn->repeat_next_tick = 0;
    btn->events          = 0;

    /* 读取当前真实电平，防止上电时按键已按下产生虚假事件 */
    btn->stable_level = read_pin(btn);
    btn->raw_last     = btn->stable_level;
}

/* ── 核心: 状态机扫描，每 1~5ms 调用一次 ── */
void key_button_tick(key_button_t *btn, uint32_t sys_tick)
{
    uint8_t  raw;
    uint32_t elapsed;

    if (btn == NULL) return;

    /* 读 GPIO */
    raw = read_pin(btn);

    /* 电平变化 → 启动消抖 */
    if (raw != btn->raw_last) {
        btn->raw_last      = raw;
        btn->debounce_tick = sys_tick;
    }

    do {
        /* 消抖未完成 */
        if (btn->debounce_tick == DEBOUNCE_IDLE) break;
        if ((sys_tick - btn->debounce_tick) < btn->cfg.debounce_ms) break;

        /* 消抖完成但电平没变 → 干扰 */
        if (raw == btn->stable_level) {
            btn->debounce_tick = DEBOUNCE_IDLE;
            break;
        }

        /* 确认电平变化 */
        btn->stable_level  = raw;
        btn->debounce_tick = DEBOUNCE_IDLE;

        if (raw == 1) {
            /* ── 按下 ── */
            btn->press_tick     = sys_tick;
            btn->long_triggered = 0;
            btn->events        |= KEY_EVENT_PRESS;

            if (btn->state == KEY_ST_WAIT_DOUBLE && btn->click_cnt == 1)
                btn->click_cnt = 2;   /* 双击窗口内再次按下 */

            btn->state = KEY_ST_PRESSED;

        } else {
            /* ── 释放 ── */
            btn->events |= KEY_EVENT_RELEASE;
            elapsed = sys_tick - btn->press_tick;

            if (btn->cfg.long_press_ms > 0 && btn->long_triggered) {
                /* 长按后释放 → 不产生单击 */
                btn->click_cnt = 0;
                btn->state     = KEY_ST_IDLE;

            } else if (btn->cfg.double_click_ms > 0 &&
                       (btn->cfg.long_press_ms == 0 || elapsed < btn->cfg.long_press_ms)) {
                btn->release_tick = sys_tick;

                if (btn->click_cnt >= 1) {
                    /* 第二下 → 双击 */
                    btn->click_cnt = 0;
                    btn->events   |= KEY_EVENT_DOUBLE_CLICK;
                    btn->state     = KEY_ST_IDLE;
                } else {
                    /* 第一下 → 等双击窗口 */
                    btn->click_cnt = 1;
                    btn->state     = KEY_ST_WAIT_DOUBLE;
                }

            } else {
                /* 双击禁用 → 直接单击 */
                btn->events |= KEY_EVENT_CLICK;
                btn->click_cnt = 0;
                btn->state     = KEY_ST_IDLE;
            }
        }
    } while (0);

    /* ── 周期性检查 ── */
    /* 长按检测 */
    if (btn->stable_level == 1 && btn->cfg.long_press_ms > 0) {
        elapsed = sys_tick - btn->press_tick;

        if (!btn->long_triggered && elapsed >= btn->cfg.long_press_ms) {
            btn->long_triggered = 1;
            btn->events        |= KEY_EVENT_LONG_PRESS;
            if (btn->cfg.repeat_interval_ms > 0)
                btn->repeat_next_tick = sys_tick + btn->cfg.repeat_interval_ms;
        }

        if (btn->long_triggered && btn->cfg.repeat_interval_ms > 0) {
            if ((int32_t)(sys_tick - btn->repeat_next_tick) >= 0) {
                btn->events         |= KEY_EVENT_LONG_REPEAT;
                btn->repeat_next_tick = sys_tick + btn->cfg.repeat_interval_ms;
            }
        }
    }

    /* 双击窗口超时 → 确认单击 */
    if (btn->state == KEY_ST_WAIT_DOUBLE && btn->click_cnt == 1) {
        if ((sys_tick - btn->release_tick) >= btn->cfg.double_click_ms) {
            btn->events |= KEY_EVENT_CLICK;
            btn->click_cnt = 0;
            btn->state     = KEY_ST_IDLE;
        }
    }
}

/* ── 取出事件并清零 ── */
uint8_t key_button_get_event(key_button_t *btn)
{
    uint8_t ev;
    if (btn == NULL) return 0;
    ev = btn->events;
    btn->events = 0;
    return ev;
}

/* ── 是否正在按住 ── */
uint8_t key_button_is_pressed(const key_button_t *btn)
{
    if (btn == NULL) return 0;
    return btn->stable_level;
}

/* ── 已按住时长 (ms) ── */
uint32_t key_button_get_hold_ms(const key_button_t *btn, uint32_t sys_tick)
{
    if (btn == NULL || btn->stable_level == 0) return 0;
    return sys_tick - btn->press_tick;
}
