# 瑞萨 RA 通用外设库

面向对象的嵌入式外设驱动库，适用于瑞萨 RA6T2 / 野火启明6T2。

## 模块

| 模块 | 文件 | 功能 |
|------|------|------|
| **按键** | `key_button.h/.c` `key.h/.c` | 单击/双击/长按/连发，消抖状态机 |
| **LED** | `bsp_led.h/.c` `led.h/.c` | ON/OFF/TOGGLE，只设引脚即可 |
| **串口** | `uart.h/.c` | 调试串口，中断接收 + 行缓冲 + 命令处理 |
| **EEPROM** | `eeprom.h/.c` | AT24C02 I2C 读写，自动跨页 |

## 架构

```
底层（通用驱动）         应用层（注册实体）
─────────────          ────────────
key_button.h/.c   →    key.h/.c
bsp_led.h/.c      →    led.h/.c
（串口/EEPROM 只有一层，不拆分）
```

## 快速使用

```c
#include "led/led.h"
#include "key_button/key.h"
#include "uart/uart.h"
#include "eeprom/eeprom.h"

void hal_entry(void)
{
    led_init_all();
    key_init();
    uart_init();
    eeprom_init();

    printf("系统启动\r\n");

    while (1) {
        key_scan();
        // ... 处理按键事件 ...

        if (uart_cmd_ready) {
            uart_process_cmd();   // 非阻塞命令处理
        }
    }
}
```

## 平台

- MCU: Renesas RA6T2 (Cortex-M33)
- FSP: 5.4.0
- 开发板: 野火启明6T2
