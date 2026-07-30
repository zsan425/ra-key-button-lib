#ifndef __UART_H__
#define __UART_H__

#include "hal_data.h"
#include <stdio.h>

/* ── 命令缓冲 ── */
#define CMD_BUF_SIZE  64
extern volatile int  uart_cmd_ready;
extern char          uart_cmd_buf[];

/* ── 初始化 ── */
void uart_init(void);

/* ── 命令处理（主循环中 uart_cmd_ready==1 时调用）── */
void uart_process_cmd(void);

#endif
