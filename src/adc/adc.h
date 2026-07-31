#ifndef __ADC_H__
#define __ADC_H__

#include "hal_data.h"

/* ── API ── */
void   adc_init(void);
double adc_read_voltage(void);                 /* 读电位器电压 (CH15, 0~3.3V) */
uint16_t adc_read_raw(uint8_t channel);        /* 读指定通道原始值 (0~4095) */

#endif
