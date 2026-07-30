#include "key.h"
#include "hal_data.h"

key_button_t g_key[KEY_COUNT];
volatile uint32_t g_sys_tick = 0;

void SysTick_Handler(void)
{
    g_sys_tick++;
}

void key_init(void)
{
    SysTick_Config(SystemCoreClock / 1000);

    key_button_init(&g_key[0], KEY1_PIN, KEY1_ACTIVE);
    key_button_init(&g_key[1], KEY2_PIN, KEY2_ACTIVE);
}

void key_scan(void)
{
    uint8_t i;
    for (i = 0; i < KEY_COUNT; i++) {
        key_button_tick(&g_key[i], g_sys_tick);
    }
}
