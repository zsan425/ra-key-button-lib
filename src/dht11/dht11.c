#include "dht11.h"

#define PIN  (13)   /* PD13 → P13 bit 13 */

/* ── DWT 微秒延时（需要先 dwt_init）── */
static void delay_us(uint32_t us)
{
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks = us * (SystemCoreClock / 1000000UL);
    while ((DWT->CYCCNT - start) < ticks);
}

/* ── 引脚方向控制（直写寄存器，比 API 快）── */
static void pin_out(void)  { R_PORT13->PDR |=  (1 << PIN); }
static void pin_in(void)   { R_PORT13->PDR &= ~(1 << PIN); }
static void pin_high(void) { R_PORT13->PODR |=  (1 << PIN); }
static void pin_low(void)  { R_PORT13->PODR &= ~(1 << PIN); }
static int  pin_read(void) { return (R_PORT13->PIDR >> PIN) & 1; }

/* ── 初始化 DWT + 引脚 ── */
void dht11_init(void)
{
    /* 使能 DWT 周期计数器 */
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL        |= DWT_CTRL_CYCCNTENA_Msk;

    /* 引脚初始化为输出高 */
    pin_high();
    pin_out();
}

/* ── 读 1 字节 ── */
static uint8_t read_byte(void)
{
    uint8_t i, val = 0;
    for (i = 0; i < 8; i++)
    {
        uint32_t t;

        /* 等到高脉冲开始（50μs 低电平结束，跳变到高）*/
        t = 5000;
        while (!pin_read() && --t);

        /* 高脉冲中段采样: 高 70μs=1, 高 26~28μs=0 */
        delay_us(40);
        if (pin_read()) val |= (1 << (7 - i));

        /* 等高脉冲结束 */
        t = 5000;
        while (pin_read() && --t);
    }
    return val;
}

/* ── 单次读取 ── */
int dht11_read(uint8_t *humi, uint8_t *temp)
{
    uint8_t data[5];
    int i;

    /* 1. 主机发起始信号: 拉低 18ms, 拉高 40μs */
    pin_out();
    pin_low();
    delay_us(18000);       /* 18ms */
    pin_high();
    delay_us(40);          /* 40μs */

    /* 2. 切换输入，等从机响应 */
    pin_in();

    /* 从机拉低 80μs */
    uint32_t t = 5000;
    while (pin_read() && --t);    /* 等高→低 */
    t = 5000;
    while (!pin_read() && --t);   /* 等低→高 */
    t = 5000;
    while (pin_read() && --t);    /* 等高→低（80μs 结束）*/

    if (t == 0) return -1;        /* 超时 = 无响应 */

    /* 3. 读 40 bit (5 字节) */
    for (i = 0; i < 5; i++)
        data[i] = read_byte();

    /* 4. 校验: 前 4 字节和 = 第 5 字节 */
    uint8_t sum = data[0] + data[1] + data[2] + data[3];
    if (sum != data[4]) return -1;

    *humi = data[0];   /* 湿度整数 */
    *temp = data[2];   /* 温度整数 */
    return 0;
}
