#include "eeprom.h"
#include <string.h>

i2c_master_event_t g_i2c_callback_event;
static unsigned int  timeout_ms = 500;

void i2c_callback(i2c_master_callback_args_t *p_args)
{
    g_i2c_callback_event = p_args->event;
}

void eeprom_init(void)
{
    R_IIC_B_MASTER_Open(&g_i2c_ctrl, &g_i2c_cfg);
    R_BSP_SoftwareDelay(10, BSP_DELAY_UNITS_MILLISECONDS);
}

fsp_err_t eeprom_write_byte(uint8_t addr, uint8_t data)
{
    uint8_t buf[2];
    buf[0] = addr;
    buf[1] = data;

    R_IIC_B_MASTER_Write(&g_i2c_ctrl, buf, 2, false);
    while ((I2C_MASTER_EVENT_TX_COMPLETE != g_i2c_callback_event) && timeout_ms) {
        R_BSP_SoftwareDelay(1, BSP_DELAY_UNITS_MILLISECONDS);
        timeout_ms--;
    }
    timeout_ms = 500;
    R_BSP_SoftwareDelay(5, BSP_DELAY_UNITS_MILLISECONDS);
    return FSP_SUCCESS;
}

fsp_err_t eeprom_read_byte(uint8_t addr, uint8_t *data)
{
    /* Step1: 写地址后发 STOP（不用 RESTART），只设地址指针不写数据 */
    R_IIC_B_MASTER_Write(&g_i2c_ctrl, &addr, 1, false);
    while ((I2C_MASTER_EVENT_TX_COMPLETE != g_i2c_callback_event) && timeout_ms) {
        R_BSP_SoftwareDelay(1, BSP_DELAY_UNITS_MILLISECONDS);
        timeout_ms--;
    }
    timeout_ms = 500;
    R_BSP_SoftwareDelay(1, BSP_DELAY_UNITS_MILLISECONDS);

    /* Step2: 从当前地址读（地址已在 step1 设好） */
    R_IIC_B_MASTER_Read(&g_i2c_ctrl, data, 1, false);
    R_BSP_SoftwareDelay(1, BSP_DELAY_UNITS_MILLISECONDS);
    return FSP_SUCCESS;
}

fsp_err_t eeprom_write_buf(uint8_t addr, uint8_t *buf, uint16_t len)
{
    uint16_t pos, chunk;
    uint8_t  tmp[10];

    if (!buf || !len)               return FSP_ERR_INVALID_ARGUMENT;
    if ((uint32_t)addr + len > 256) return FSP_ERR_INVALID_ARGUMENT;

    pos = 0;
    while (pos < len)
    {
        uint8_t  cur  = (uint8_t)(addr + pos);
        uint16_t left = 8 - (cur & 0x07);
        chunk = (len - pos < left) ? (len - pos) : left;

        tmp[0] = cur;
        memcpy(tmp + 1, buf + pos, chunk);

        R_IIC_B_MASTER_Write(&g_i2c_ctrl, tmp, (uint32_t)(chunk + 1), false);
        while ((I2C_MASTER_EVENT_TX_COMPLETE != g_i2c_callback_event) && timeout_ms) {
            R_BSP_SoftwareDelay(1, BSP_DELAY_UNITS_MILLISECONDS);
            timeout_ms--;
        }
        timeout_ms = 500;
        R_BSP_SoftwareDelay(5, BSP_DELAY_UNITS_MILLISECONDS);
        pos += chunk;
    }
    return FSP_SUCCESS;
}

fsp_err_t eeprom_read_buf(uint8_t addr, uint8_t *buf, uint16_t len)
{
    if (!buf || !len)               return FSP_ERR_INVALID_ARGUMENT;
    if ((uint32_t)addr + len > 256) return FSP_ERR_INVALID_ARGUMENT;

    R_IIC_B_MASTER_Write(&g_i2c_ctrl, &addr, 1, false);
    while ((I2C_MASTER_EVENT_TX_COMPLETE != g_i2c_callback_event) && timeout_ms) {
        R_BSP_SoftwareDelay(1, BSP_DELAY_UNITS_MILLISECONDS);
        timeout_ms--;
    }
    timeout_ms = 500;
    R_BSP_SoftwareDelay(1, BSP_DELAY_UNITS_MILLISECONDS);
    R_IIC_B_MASTER_Read(&g_i2c_ctrl, buf, (uint32_t)len, false);
    return FSP_SUCCESS;
}
