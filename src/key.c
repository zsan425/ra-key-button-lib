#include "key.h"
#include "hal_data.h"

key_button_t g_key[KEY_COUNT];
volatile uint32_t g_sys_tick = 0;
volatile uint8_t  g_key_irq[KEY_COUNT];

void SysTick_Handler(void)
{
    g_sys_tick++;
}

#if KEY_USE_IRQ

/* ── KEY1 外部中断 ── */
void key1_irq_callback(external_irq_callback_args_t *p_args)
{
    (void)p_args;
    g_key_irq[0] = 1;
}

/* ── KEY2 外部中断 ── */
void key2_irq_callback(external_irq_callback_args_t *p_args)
{
    (void)p_args;
    g_key_irq[1] = 1;
}

#endif

void key_init(void)
{
    SysTick_Config(SystemCoreClock / 1000);

    key_button_init(&g_key[0], KEY1_PIN, KEY1_ACTIVE);
    key_button_init(&g_key[1], KEY2_PIN, KEY2_ACTIVE);

#if KEY_USE_IRQ
    R_ICU_ExternalIrqOpen(&key1_irq_ctrl, &key1_irq_cfg);
    R_ICU_ExternalIrqEnable(&key1_irq_ctrl);
    R_ICU_ExternalIrqOpen(&key2_irq_ctrl, &key2_irq_cfg);
    R_ICU_ExternalIrqEnable(&key2_irq_ctrl);
#endif
}

void key_scan(void)
{
    uint8_t i;
    for (i = 0; i < KEY_COUNT; i++) {
        key_button_tick(&g_key[i], g_sys_tick);
    }
}
