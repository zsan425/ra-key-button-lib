#include "hal_data.h"

FSP_CPP_HEADER
void R_BSP_WarmStart(bsp_warm_start_event_t event);
FSP_CPP_FOOTER

#include "led/led.h"
#include "key_button/key.h"

void hal_entry(void)
{
    led_init_all();
    led_off(LED_1);
    led_off(LED_2);

    key_init();

    while (1)
    {
        key_scan();

        uint8_t ev1 = key_button_get_event(KEY_1);
        uint8_t ev2 = key_button_get_event(KEY_2);

        /* KEY1: 单击翻转 LED1, 双击翻转 LED2, 长按全灭 */
        if (ev1 & KEY_EVENT_CLICK)        led_toggle(LED_1);
        if (ev1 & KEY_EVENT_DOUBLE_CLICK) led_toggle(LED_2);
        if (ev1 & KEY_EVENT_LONG_PRESS) { led_off(LED_1); led_off(LED_2); }

        /* KEY2: 单击亮 LED2, 按住连发翻转 LED2 */
        if (ev2 & KEY_EVENT_CLICK)       led_on(LED_2);
        if (ev2 & KEY_EVENT_LONG_REPEAT) led_toggle(LED_2);
    }

#if BSP_TZ_SECURE_BUILD
    R_BSP_NonSecureEnter();
#endif
}

/*******************************************************************************************************************//**
 * This function is called at various points during the startup process.  This implementation uses the event that is
 * called right before main() to set up the pins.
 *
 * @param[in]  event    Where at in the start up process the code is currently at
 **********************************************************************************************************************/
void R_BSP_WarmStart (bsp_warm_start_event_t event)
{
    if (BSP_WARM_START_RESET == event)
    {
#if BSP_FEATURE_FLASH_LP_VERSION != 0

        /* Enable reading from data flash. */
        R_FACI_LP->DFLCTL = 1U;

        /* Would normally have to wait tDSTOP(6us) for data flash recovery. Placing the enable here, before clock and
         * C runtime initialization, should negate the need for a delay since the initialization will typically take more than 6us. */
#endif
    }

    if (BSP_WARM_START_POST_C == event)
    {
        /* C runtime environment and system clocks are setup. */

        /* Configure pins. */
        R_IOPORT_Open(&g_ioport_ctrl, g_ioport.p_cfg);
    }
}

#if BSP_TZ_SECURE_BUILD

BSP_CMSE_NONSECURE_ENTRY void template_nonsecure_callable ();

/* Trustzone Secure Projects require at least one nonsecure callable function in order to build (Remove this if it is not required to build). */
BSP_CMSE_NONSECURE_ENTRY void template_nonsecure_callable ()
{

}
#endif
