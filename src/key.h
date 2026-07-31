#ifndef __KEY_H__
#define __KEY_H__

#include "key_button.h"

/* ── 启明6T2: KEY1=PB14, KEY2=PC02, 低电平按下(上拉) ── */
#define KEY1_PIN     BSP_IO_PORT_11_PIN_14
#define KEY1_ACTIVE  BSP_IO_LEVEL_LOW

#define KEY2_PIN     BSP_IO_PORT_12_PIN_02
#define KEY2_ACTIVE  BSP_IO_LEVEL_LOW

/* ── 中断模式开关: 1=中断, 0=纯轮询 ── */
#define KEY_USE_IRQ  1

#define KEY_COUNT  2

extern key_button_t g_key[KEY_COUNT];
#define KEY_1  (&g_key[0])
#define KEY_2  (&g_key[1])

void key_init(void);
void key_scan(void);

/* ── 中断触发计数 ── */
extern volatile uint8_t g_key_irq[KEY_COUNT];

#endif
