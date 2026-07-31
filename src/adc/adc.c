#include "adc.h"

/* ── 扫描完成标志 ── */
static volatile int g_scan_done = 0;

/* ── 回调 ── */
void adc0_callback(adc_callback_args_t *p_args)
{
    (void)p_args;
    g_scan_done = 1;
}

/* ── 初始化 ── */
void adc_init(void)
{
    R_ADC_B_Open(&g_adc0_ctrl, &g_adc0_cfg);
    R_ADC_B_ScanCfg(&g_adc0_ctrl, &g_adc0_scan_cfg);
    R_ADC_B_ScanGroupStart(&g_adc0_ctrl, ADC_GROUP_MASK_ALL);
}

/* ── 读电位器电压 ── */
double adc_read_voltage(void)
{
    uint16_t raw = adc_read_raw(ADC_CHANNEL_15);
    return (double)raw * 3.3 / 4095.0;
}

/* ── 读指定通道原始值 ── */
uint16_t adc_read_raw(uint8_t channel)
{
    uint16_t data;

    R_ADC_B_ScanGroupStart(&g_adc0_ctrl, ADC_GROUP_MASK_ALL);
    g_scan_done = 0;
    while (!g_scan_done);

    R_ADC_B_Read(&g_adc0_ctrl, (adc_channel_t)channel, &data);
    return data;
}
