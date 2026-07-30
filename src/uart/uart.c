#include "uart.h"
#include "led/led.h"
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
    else if (strcmp(uart_cmd_buf, "help")     == 0) {
        printf("led1 on/off  led2 on/off  help\r\n");
    }
    else printf("? %s\r\n", uart_cmd_buf);
}
