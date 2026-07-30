# EEPROM (AT24C02) 调试踩坑总结

## 最终状态：✅ 读写正常

| 测试命令 | 功能 | 结果 |
|---------|------|------|
| `eeprom` | 单字节写入+回读验证 | PASS |
| `eetest` | 8 字节批量写入+回读比对 | PASS |

---

## 踩坑 1：跳线（3 小时）

### 现象
I2C Write 返回 `err=8`（FSP_ERR_IN_USE），Read 返回 `err=0` 但读到全 0x00。

### 根因
启明6T2 开发板的 I2C 引脚（PD12/PD13）通过**跳线帽**连接到 EEPROM。跳线没接时，SCL/SDA 悬空，I2C 外设检测到总线异常状态，拒绝启动传输。

### 教训
- **先查硬件连接，再查软件**。示波器/万用表量 SCL/SDA 是否有波形，比改代码高效得多。
- 拿到一块新板子，第一件事是看原理图上的跳线表。

---

## 踩坑 2：异步回调 ≠ 同步等待（2 小时）

### 现象
调用 `R_IIC_B_MASTER_Write` 后立即调 `R_IIC_B_MASTER_Read`，Read 返回 `err=8`。

### 根因
FSP 的 I2C 驱动是**异步**的——Write 函数只负责启动传输，实际数据在中断里发。回调函数 `i2c_callback` 在传输完成时触发。如果在回调触发前调用下一个 I2C 函数，驱动还在忙，返回 FSP_ERR_IN_USE。

### 正确做法
```c
// 每次 I2C 操作后，必须等待回调完成
R_IIC_B_MASTER_Write(&g_i2c_ctrl, buf, len, false);
while (g_i2c_callback_event != I2C_MASTER_EVENT_TX_COMPLETE) {
    R_BSP_SoftwareDelay(1, BSP_DELAY_UNITS_MILLISECONDS);  // 等回调
}
```

### 教训
**FSP 所有带回调参数的驱动都是异步的**（UART、I2C、SPI、ADC…）。写入后必须等回调，不能连续调用。

---

## 踩坑 3：RESTART 信号 Bug（核心问题，4 小时）

### 现象
数据写入后回读，读到的值总是偏移了 1 个字节——写 `0xA5` 到地址 `0x00`，回读 `0x00` 返回 `0xFF`，而 `0x01` 返回 `0xA5`。

### 排查过程
1. 怀疑 EEPROM 地址格式（7-bit vs 8-bit）→ 试了 0x50 和 0xA0，0xA0 更差
2. 怀疑多字节写入时序 → 验证 1-byte / 2-byte 写入，偏移规律一致
3. 怀疑读函数 → 对比野火官方 009 例程，写法完全一致
4. **关键发现**：把读函数的 `restart=true` 改成 `restart=false`（发 STOP 而非 RESTART），数据立刻正确

### 根因
FSP 5.4.0 的 `R_IIC_B_MASTER_Write(..., restart=true)` 在硬件上没有正确生成 I2C RESTART 信号，导致后续的 `R_IIC_B_MASTER_Read` 读到了错误的地址。

#### 错误写法（restart=true）:
```c
// 发地址后用 RESTART 衔接读操作
R_IIC_B_MASTER_Write(&g_i2c_ctrl, &addr, 1, true);   // 设地址 + RESTART
R_IIC_B_MASTER_Read(&g_i2c_ctrl, data, 1, false);     // 读 —— 却读到了旧地址！
```

#### 正确写法（restart=false）:
```c
// 发地址后发 STOP，再单独发起读
R_IIC_B_MASTER_Write(&g_i2c_ctrl, &addr, 1, false);   // 设地址 + STOP
while (等待回调);
R_IIC_B_MASTER_Read(&g_i2c_ctrl, data, 1, false);     // 独立读 —— 正确！
```

### 教训
1. **不要假设 HAL 库的每个参数都正常工作**。`restart=true` 在文档里写得很清楚，但实际硬件行为可能因 FSP 版本而异（5.5.0 没问题，5.4.0 有 bug）。
2. **对比已知可用的工程**。野火 009 例程（e2studio + FSP 5.5.0）能跑，我们（Keil + FSP 5.4.0）不行——版本差异是第一怀疑对象。
3. **穷举测试比推理更快**。在确定"偏移+1"规律后，直接试 `restart=false`，5 分钟就定位了。之前的地址格式、字节序、时序分析花了 3 小时没有进展。
4. **EEPROM 读操作的两种方式都可行**：
   - Random Read：Write(addr, restart=true) + Read — 需要 RESTART 支持
   - Current Address Read：Write(addr, restart=false) + STOP + Read — 不需要 RESTART，分两步更稳健

---

## 经验总结

| 维度 | 经验 |
|------|------|
| **硬件** | 先查跳线、上拉电阻、WP 引脚。EEPROM 的 WP 必须接 GND 才能写 |
| **时序** | FSP 异步驱动每次操作后必须等回调。不能用 `R_BSP_SoftwareDelay` 代替 |
| **调试** | 规律性 bug（如偏移+1）用穷举法：换参数、换顺序、换 API 调用方式 |
| **版本** | HAL 库的 bug 存在版本差异。FSP 5.4.0 RESTART 有问题，5.5.0 修复了 |
| **心态** | 调了 6 小时，根因就是改一个 `true` 为 `false`。嵌入式调试就是这样 |
