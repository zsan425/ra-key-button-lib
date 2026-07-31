#include "uart.h"
#include "led/led.h"
#include "eeprom/eeprom.h"
#include "adc/adc.h"
#include "dht11/dht11.h"
#include <string.h>

/* ── 发送完成 ── */
static volatile int g_tx_done = 1;

/* ── 行缓冲 ── */
char          uart_cmd_buf[CMD_BUF_SIZE];
volatile int  uart_cmd_ready = 0;
static int    cmd_idx = 0;

/* ── 回调: TX完成 + 行缓冲接收 ── */
void uart_callback(uart_callback_args_t *p_args)
{
    if (p_args->event == UART_EVENT_TX_COMPLETE) {
        g_tx_done = 1;

    } else if (p_args->event == UART_EVENT_RX_CHAR) {
        char ch = (char)p_args->data;

        if (ch == '\r' || ch == '\n') {
            if (cmd_idx > 0) {
                uart_cmd_buf[cmd_idx] = '\0';
                uart_cmd_ready = 1;
                cmd_idx = 0;
            }
        } else if (cmd_idx < CMD_BUF_SIZE - 1) {
            uart_cmd_buf[cmd_idx++] = ch;
        }
    }
}

/* ── 初始化 ── */
void uart_init(void)
{
    R_SCI_B_UART_Open(&debug_uart9_ctrl, &debug_uart9_cfg);
}

/* ── printf 重定向 ── */
int fputc(int ch, FILE *f)
{
    (void)f;
    g_tx_done = 0;
    R_SCI_B_UART_Write(&debug_uart9_ctrl, (uint8_t *)&ch, 1);
    while (!g_tx_done);
    return ch;
}

/* ── 命令处理 ── */
void uart_process_cmd(void)
{
    uart_cmd_ready = 0;

    printf("CMD: %s\r\n", uart_cmd_buf);

    if      (strcmp(uart_cmd_buf, "led1 on")  == 0) led_on(LED_1);
    else if (strcmp(uart_cmd_buf, "led1 off") == 0) led_off(LED_1);
    else if (strcmp(uart_cmd_buf, "led2 on")  == 0) led_on(LED_2);
    else if (strcmp(uart_cmd_buf, "led2 off") == 0) led_off(LED_2);
    else if (strcmp(uart_cmd_buf, "eepower")  == 0) {
        uint8_t m[4] = {0xDE, 0xAD, 0xBE, 0xEF};
        eeprom_write_buf(0x00, m, 4);
        R_BSP_SoftwareDelay(100, BSP_DELAY_UNITS_MILLISECONDS);  /* 确保写入完成 */
        /* 立即回读验证 */
        uint8_t v[4];
        eeprom_read_buf(0x00, v, 4);
        printf("verify: %02X %02X %02X %02X\r\n", v[0],v[1],v[2],v[3]);
        printf("断电后重启输 eecheck\r\n");
    }
    else if (strcmp(uart_cmd_buf, "eecheck")  == 0) {
        uint8_t m[4];
        eeprom_read_buf(0x00, m, 4);
        printf("%s (%02X%02X%02X%02X)\r\n",
               (m[0]==0xDE&&m[1]==0xAD&&m[2]==0xBE&&m[3]==0xEF)?"POWER-LOSS PASS":"FAIL",
               m[0],m[1],m[2],m[3]); 
    }
    else if (strcmp(uart_cmd_buf, "pot")       == 0) {
        uint16_t raw = adc_read_raw(ADC_CHANNEL_15);
        printf("POT: %.6fV (%d/4095)\r\n", (double)raw * 3.3 / 4095.0, raw);
    }
    else if (strcmp(uart_cmd_buf, "lm35")      == 0)
        printf("LM35: %.2f°C\r\n", adc_read_temp(ADC_CHANNEL_5, ADC_TEMP_LM35));
    else if (strcmp(uart_cmd_buf, "dht11")    == 0) {
        uint8_t h, t;
        if (dht11_read(&h, &t) == 0)
            printf("DHT11: %d%%RH  %d°C\r\n", h, t);
        else
            printf("DHT11: no response\r\n");
    }
    else if (strcmp(uart_cmd_buf, "help")     == 0) {
        printf("led1/2 on/off  eeprom eetest eepower eecheck  pot lm35 dht11  help\r\n");
    }
    else if (strcmp(uart_cmd_buf, "eeprom")  == 0) {
        uint8_t d;
        eeprom_read_byte(0x50, &d);
        printf("before: 0x50=0x%02X\r\n", d);
        eeprom_write_byte(0x50, 0xA5);
        R_BSP_SoftwareDelay(10, BSP_DELAY_UNITS_MILLISECONDS);
        eeprom_read_byte(0x50, &d);
        printf("after:  0x50=0x%02X %s\r\n", d, d==0xA5?"PASS":"FAIL");
    }
    else if (strcmp(uart_cmd_buf, "eetest")  == 0) {
        uint8_t w[8], r[8];
        uint8_t i, pass = 1;
        /* 写递增序列 0x00~0x07 到地址 0x10 */
        for (i = 0; i < 8; i++) w[i] = i;
        eeprom_write_buf(0x10, w, 8);
        R_BSP_SoftwareDelay(50, BSP_DELAY_UNITS_MILLISECONDS);
        /* 回读比对 */
        eeprom_read_buf(0x10, r, 8);
        printf("addr: 0x10 0x11 0x12 0x13 0x14 0x15 0x16 0x17\r\n");
        printf("write:"); for (i=0;i<8;i++) printf("  %02X ", w[i]);
        printf("\r\nread: "); for (i=0;i<8;i++) { printf("  %02X ", r[i]); if (r[i]!=w[i]) pass=0; }
        printf("\r\n%s\r\n", pass?"PASS":"FAIL");
    }
    else printf("? %s\r\n", uart_cmd_buf);
}
