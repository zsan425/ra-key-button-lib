#ifndef __EEPROM_H__
#define __EEPROM_H__

#include "hal_data.h"

/* ── API ── */
void eeprom_init(void);

/* 单字节读写 */
fsp_err_t eeprom_write_byte(uint8_t addr, uint8_t data);
fsp_err_t eeprom_read_byte(uint8_t addr, uint8_t *data);

/* 多字节读写（自动跨页） */
fsp_err_t eeprom_write_buf(uint8_t addr, uint8_t *buf, uint16_t len);
fsp_err_t eeprom_read_buf(uint8_t addr, uint8_t *buf, uint16_t len);

#endif
