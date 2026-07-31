#include "eeprom.h"
#include <string.h>

i2c_master_event_t g_i2c_callback_event;

void i2c_callback(i2c_master_callback_args_t *p_args)
{
    g_i2c_callback_event = p_args->event;
}

/* ── 等待 TX_COMPLETE（500ms 超时，局部变量不跨函数共享）── */
static int i2c_wait_tx(void)
{
    unsigned int t = 500;
    while (t && g_i2c_callback_event != I2C_MASTER_EVENT_TX_COMPLETE) {
        R_BSP_SoftwareDelay(1, BSP_DELAY_UNITS_MILLISECONDS);
        t--;
    }
    return (t > 0) ? 0 : -1;
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
    if (i2c_wait_tx()) return FSP_ERR_TIMEOUT;

    R_BSP_SoftwareDelay(5, BSP_DELAY_UNITS_MILLISECONDS);
    return FSP_SUCCESS;
}

fsp_err_t eeprom_read_byte(uint8_t addr, uint8_t *data)
{
    R_IIC_B_MASTER_Write(&g_i2c_ctrl, &addr, 1, false);
    if (i2c_wait_tx()) return FSP_ERR_TIMEOUT;

    R_BSP_SoftwareDelay(1, BSP_DELAY_UNITS_MILLISECONDS);
    R_IIC_B_MASTER_Read(&g_i2c_ctrl, data, 1, false);
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
        if (i2c_wait_tx()) return FSP_ERR_TIMEOUT;

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
    if (i2c_wait_tx()) return FSP_ERR_TIMEOUT;

    R_BSP_SoftwareDelay(1, BSP_DELAY_UNITS_MILLISECONDS);
    R_IIC_B_MASTER_Read(&g_i2c_ctrl, buf, (uint32_t)len, false);
    return FSP_SUCCESS;
}
