#ifndef __DHT11_H__
#define __DHT11_H__

#include "hal_data.h"

/* ── DHT11 引脚: PD13 ── */
#define DHT11_PIN  BSP_IO_PORT_13_PIN_13

/* ── 单次读取 ── */
int dht11_read(uint8_t *humi, uint8_t *temp);   /* 返回 0=OK, -1=失败 */

#endif
