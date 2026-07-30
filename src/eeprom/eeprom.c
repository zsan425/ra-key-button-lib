#include "eeprom.h"
#include <string.h>

/* ── 回调（hal_data.h 已声明）── */
void i2c_callback(i2c_master_callback_args_t *p_args)
{
    (void)p_args;
}

/* ── 初始化 ── */
void eeprom_init(void)
{
    R_IIC_B_MASTER_Open(&g_i2c_ctrl, &g_i2c_cfg);
}

/* ── 写 1 字节 ── */
fsp_err_t eeprom_write_byte(uint8_t addr, uint8_t data)
{
    return eeprom_write_buf(addr, &data, 1);
}

/* ── 读 1 字节 ── */
fsp_err_t eeprom_read_byte(uint8_t addr, uint8_t *data)
{
    return eeprom_read_buf(addr, data, 1);
}

/* ── 写多字节 ── */
fsp_err_t eeprom_write_buf(uint8_t addr, uint8_t *buf, uint16_t len)
{
    fsp_err_t err;
    uint16_t  pos, chunk;
    uint8_t   tmp[10];

    if (!buf || !len)                     return FSP_ERR_INVALID_ARGUMENT;
    if ((uint32_t)addr + len > 256)       return FSP_ERR_INVALID_ARGUMENT;

    pos = 0;
    while (pos < len)
    {
        uint8_t  cur  = (uint8_t)(addr + pos);
        uint16_t left = 8 - (cur & 0x07);
        chunk = (len - pos < left) ? (len - pos) : left;

        tmp[0] = cur;
        memcpy(tmp + 1, buf + pos, chunk);

        err = R_IIC_B_MASTER_Write(&g_i2c_ctrl, tmp, (uint32_t)(chunk + 1), false);
        if (err != FSP_SUCCESS) return err;

        R_BSP_SoftwareDelay(5, BSP_DELAY_UNITS_MILLISECONDS);
        pos += chunk;
    }
    return FSP_SUCCESS;
}

/* ── 读多字节 ── */
fsp_err_t eeprom_read_buf(uint8_t addr, uint8_t *buf, uint16_t len)
{
    fsp_err_t err;

    if (!buf || !len)                     return FSP_ERR_INVALID_ARGUMENT;
    if ((uint32_t)addr + len > 256)       return FSP_ERR_INVALID_ARGUMENT;

    err = R_IIC_B_MASTER_Write(&g_i2c_ctrl, &addr, 1, true);
    if (err != FSP_SUCCESS) return err;

    return R_IIC_B_MASTER_Read(&g_i2c_ctrl, buf, (uint32_t)len, false);
}
