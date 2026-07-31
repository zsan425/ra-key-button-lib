#ifndef __ADC_H__
#define __ADC_H__

#include "hal_data.h"

/* ── 温度传感器类型 ── */
typedef enum { ADC_TEMP_LM35, ADC_TEMP_TMP36 } adc_temp_type_t;

/* ── API ── */
void      adc_init(void);
double    adc_read_voltage(void);                          /* CH15 电位器电压 */
uint16_t  adc_read_raw(uint8_t channel);                   /* 指定通道原始值 */
double    adc_read_temp(uint8_t channel, adc_temp_type_t t); /* 模拟温度传感器 */

#endif
