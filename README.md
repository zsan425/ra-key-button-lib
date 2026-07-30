# 瑞萨 RA 通用按键库

面向对象的按键驱动库，支持单击/双击/长按/连发，消抖 + 状态机，无阻塞。

## 架构

```
底层（通用）          应用层（注册实体）
─────────           ─────────
key_button.h/.c  →  key.h/.c     按键：状态机 + 事件检测
bsp_led.h/.c     →  led.h/.c     LED：ON/OFF/TOGGLE
```

## 快速使用

```c
#include "key.h"
#include "led.h"

void hal_entry(void)
{
    led_init_all();    // 初始化 LED
    key_init();        // 初始化按键 + SysTick

    while (1) {
        key_scan();    // 扫描按键（每圈必调）

        uint8_t ev1 = key_button_get_event(KEY_1);
        uint8_t ev2 = key_button_get_event(KEY_2);

        if (ev1 & KEY_EVENT_CLICK)        led_toggle(LED_1);
        if (ev1 & KEY_EVENT_DOUBLE_CLICK) led_toggle(LED_2);
        if (ev1 & KEY_EVENT_LONG_PRESS) { led_off(LED_1); led_off(LED_2); }

        if (ev2 & KEY_EVENT_CLICK)        led_on(LED_2);
        if (ev2 & KEY_EVENT_LONG_REPEAT)  led_toggle(LED_2);
    }
}
```

## 事件类型

| 事件 | 含义 | 触发时机 |
|------|------|---------|
| `KEY_EVENT_PRESS` | 按下 | 消抖确认后立即 |
| `KEY_EVENT_RELEASE` | 释放 | 消抖确认后立即 |
| `KEY_EVENT_CLICK` | 单击 | 释放后等双击窗口超时 |
| `KEY_EVENT_DOUBLE_CLICK` | 双击 | 两次点击在窗口内 |
| `KEY_EVENT_LONG_PRESS` | 长按 | 按住超时，仅一次 |
| `KEY_EVENT_LONG_REPEAT` | 连发 | 长按后周期性触发 |

## 可调参数

```c
btn.cfg.debounce_ms        = 20;    // 消抖时间（默认20ms）
btn.cfg.long_press_ms      = 1000;  // 长按阈值（默认1000ms, 0=禁用）
btn.cfg.repeat_interval_ms = 200;   // 连发间隔（默认200ms, 0=禁用）
btn.cfg.double_click_ms    = 400;   // 双击窗口（默认400ms, 0=禁用）
```

## 移植

只需改 `key_button.c` 里 `read_pin()` 函数的 GPIO 读取方式：

```c
// 瑞萨 RA FSP:
R_IOPORT_PinRead(&g_ioport_ctrl, btn->pin, &level);

// STM32 HAL:
level = HAL_GPIO_ReadPin(btn->port, btn->pin);

// Arduino:
level = digitalRead(btn->pin);
```

## 平台

- MCU: Renesas RA6T2 (Cortex-M33)
- FSP: 5.4.0
- 开发板: 野火启明6T2
