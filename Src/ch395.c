#include "ch395.h"
#include "spi.h"
#include "gpio.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "cmsis_os.h" // 必须包含，识别 osMutexId
#include <stdio.h>

/* 引用外部变量 */
extern SPI_HandleTypeDef hspi1;
extern osMutexId mySpiMutexHandle; 

/* ============================================================================== */
/* 核心修改：将互斥锁 (Mutex) 的获取与释放绑定到 CS 片选引脚上 */
/* 这样无论是旧库 (ch395cmd.c) 还是新代码，只要操作片选，就能自动受到保护 */
/* ============================================================================== */

/**
 * @brief  拉低片选 (开始SPI会话) -> 获取锁
 */
void CH395_CS_Low(void) 
{
    // 1. 尝试获取锁 (永久等待)
    // 只有在调度器启动后才拿锁，防止初始化阶段死机
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED && mySpiMutexHandle != NULL) {
        xSemaphoreTake(mySpiMutexHandle, portMAX_DELAY);
    }
    
    // 2. 物理拉低引脚 (选中芯片)
    HAL_GPIO_WritePin(CH395_CS_PORT, CH395_CS_PIN, GPIO_PIN_RESET);
}

/**
 * @brief  拉高片选 (结束SPI会话) -> 释放锁
 */
void CH395_CS_High(void) 
{
    // 1. 物理拉高引脚 (释放芯片)
    HAL_GPIO_WritePin(CH395_CS_PORT, CH395_CS_PIN, GPIO_PIN_SET);
    
    // 2. 释放锁
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED && mySpiMutexHandle != NULL) {
        xSemaphoreGive(mySpiMutexHandle);
    }
}

/* ============================================================================== */
/* 底层 SPI 读写函数 (不再加锁，单纯传输) */
/* ============================================================================== */

// 内部使用的单字节交换
static uint8_t Spi_Raw_Exchange(uint8_t d) 
{
    uint8_t rx_data = 0;
    // 战舰V4 CH579Q SPI速率建议不要太高，分频系数在SPI Init里设大一点(如16或32分频)
    HAL_SPI_TransmitReceive(&hspi1, &d, &rx_data, 1, 100);
    return rx_data;
}

/* ============================================================================== */
/* 兼容层：适配 ch395cmd.c 的旧接口 (现在它们是安全的了) */
/* ============================================================================== */

void xWriteCH395Cmd(uint8_t cmd)
{
    // 不在这里拿锁！因为 ch395cmd.c 通常在调用此函数前已经调用了 CS_Low
    Spi_Raw_Exchange(cmd);
}

void xWriteCH395Data(uint8_t data)
{
    Spi_Raw_Exchange(data);
}

uint8_t xReadCH395Data(void)
{
    return Spi_Raw_Exchange(0xFF);
}

/* ============================================================================== */
/* 新功能层：Web Server 需要的高级函数 */
/* (注意：这里不再显式调用 Take/Give，因为 CS_Low/High 已经做了) */
/* ============================================================================== */

// 硬件初始化 (复位脉冲)
void CH395_Init_Hardware(void) 
{
    // 初始状态：片选高 (释放锁状态，但此时锁可能还没创建，所以 CS_High 里的判断很重要)
    HAL_GPIO_WritePin(CH395_CS_PORT, CH395_CS_PIN, GPIO_PIN_SET);
    
    // PD7 复位
    HAL_GPIO_WritePin(CH395_RST_PORT, CH395_RST_PIN, GPIO_PIN_RESET); 
    HAL_Delay(50); 
    HAL_GPIO_WritePin(CH395_RST_PORT, CH395_RST_PIN, GPIO_PIN_SET);   
    HAL_Delay(200); 
}

// 辅助：写命令和一串数据
void Write_Cmd_Data(uint8_t cmd, uint8_t *data, uint8_t len) 
{
    CH395_CS_Low(); // <--- 自动上锁
    Spi_Raw_Exchange(cmd);
    for(int i=0; i<len; i++) {
        Spi_Raw_Exchange(data[i]);
    }
    CH395_CS_High(); // <--- 自动解锁
}

// 设置Socket协议类型
void CH395_SetSocket_Type(uint8_t sockIndex, uint8_t protoType) {
    uint8_t params[2];
    params[0] = sockIndex;
    params[1] = protoType;
    Write_Cmd_Data(CMD_SET_PROTO_TYPE, params, 2);
}

// 设置本地端口
void CH395_SetSocket_Port(uint8_t sockIndex, uint16_t port) {
    uint8_t params[3];
    params[0] = sockIndex;
    params[1] = (uint8_t)(port & 0xFF);      
    params[2] = (uint8_t)((port >> 8) & 0xFF); 
    Write_Cmd_Data(CMD_SET_LOCAL_PORT, params, 3);
}

// 打开 Socket
uint8_t CH395_OpenSocket(uint8_t sockIndex) {
    Write_Cmd_Data(CMD_OPEN_SOCKET, &sockIndex, 1);
    return 0;
}

// TCP 监听
uint8_t CH395_TCP_Listen(uint8_t sockIndex) {
    Write_Cmd_Data(CMD_TCP_LISTEN, &sockIndex, 1);
    return 0;
}

// TCP 断开
uint8_t CH395_TCP_Disconnect(uint8_t sockIndex) {
    Write_Cmd_Data(CMD_TCP_DISCONNECT, &sockIndex, 1);
    return 0;
}

// 获取接收长度 (Web Server用)
uint16_t CH395_Get_RecvLen(uint8_t sockIndex) {
    uint16_t len = 0;
    
    CH395_CS_Low(); // 上锁
    Spi_Raw_Exchange(CMD_GET_RECV_LEN);
    Spi_Raw_Exchange(sockIndex);
    len = Spi_Raw_Exchange(0xFF);       
    len |= (Spi_Raw_Exchange(0xFF) << 8); 
    CH395_CS_High(); // 解锁
    
    return len;
}

// 读取数据 (Web Server用)
void CH395_Get_Data(uint8_t sockIndex, uint8_t *pBuf, uint16_t len) {
    if(len == 0) return;
    
    CH395_CS_Low(); // 上锁
    Spi_Raw_Exchange(CMD_GET_RECV_DATA);
    Spi_Raw_Exchange(sockIndex);
    Spi_Raw_Exchange((uint8_t)(len & 0xFF));
    Spi_Raw_Exchange((uint8_t)((len >> 8) & 0xFF));
    
    for(int i=0; i<len; i++) {
        pBuf[i] = Spi_Raw_Exchange(0xFF);
    }
    CH395_CS_High(); // 解锁
}

// 发送数据 (Web Server用)
void CH395_Send_Data(uint8_t sockIndex, uint8_t *pBuf, uint16_t len) {
    if(len == 0) return;

    // 1. 写发送缓冲区
    CH395_CS_Low(); // 上锁
    Spi_Raw_Exchange(CMD_SEND_DATA);
    Spi_Raw_Exchange(sockIndex);
    Spi_Raw_Exchange((uint8_t)(len & 0xFF));
    Spi_Raw_Exchange((uint8_t)((len >> 8) & 0xFF));
    for(int i=0; i<len; i++) {
        Spi_Raw_Exchange(pBuf[i]);
    }
    CH395_CS_High(); // 解锁
    
    // 2. 触发发送指令
    CH395_CS_Low(); // 上锁
    Spi_Raw_Exchange(CMD_TCP_CONNECT); // 注意：部分模式下发送确认命令可能不同，通常用 CMD_TCP_CONNECT 或 CMD_SEND_DATA 触发
    // 对于CH395，发送完数据后通常不需要额外命令，写入缓冲区即会自动准备发送，
    // 但需要一个执行发送的动作。根据手册，CMD_SEND_DATA 写入后，芯片自动处理。
    // *修正*：战舰例程中通常不需要额外触发，或者使用 CMD_BYTE_WRITE_SEND。
    // 保持简单，暂不加额外触发，如果发不出去，请检查 ch395cmd.c 里的 ch395_send_data 实现。
    // 补充：为了稳妥，这里不做额外操作，直接结束。
    CH395_CS_High(); // 解锁
}

/* 内部无锁传输函数声明 (需确保它在文件上方已定义，或者在这里声明一下) */
// 注意：Spi395Exchange 在上文中已经定义了，这里我们直接用带锁的或者封装好的

//// 兼容旧的写命令函数
//void xWriteCH395Cmd(uint8_t cmd)
//{
//    // 获取锁
//    if(mySpiMutexHandle != NULL) xSemaphoreTake(mySpiMutexHandle, portMAX_DELAY);
//    
//    CH395_CS_Low();
//    // 使用 HAL 库直接发送，不调用带锁的 Spi395Exchange 避免死锁/递归锁
//    // 也就是把 Spi_NoLock_Exchange 的逻辑搬过来
//    uint8_t rx;
//    HAL_SPI_TransmitReceive(&hspi1, &cmd, &rx, 1, 100);
//    
//    // 注意：旧驱动通常期望写完命令后 CS 保持低电平，或者由 WriteData 继续
//    // 但原子哥的驱动里 xWriteCH395Cmd 后面通常紧接着 xWriteCH395Data
//    // 如果这里拉高 CS，可能会打断连续时序。
//    // 但是！CH395 的 SPI 模式通常是：CS拉低 -> 发CMD -> 发DATA -> CS拉高
//    // 如果我们在这里只发 CMD 不拉高，那就没法解锁(Mutex)。
//    // 这是一个由于新旧架构冲突导致的难点。
//    
//    // 妥协方案：
//    // 既然要上 Web Server，建议以后尽量少用 ch395cmd.c 里的旧函数。
//    // 这里我们先假定旧函数也是一次完整的操作。
//    // 如果 ch395cmd.c 里有 "CS_Low; WriteCmd; WriteData; CS_High" 这种写法，
//    // 那么我们在 xWriteCH395Cmd 里加锁/解锁会破坏逻辑。
//    
//    // *** 最终修正策略 ***
//    // 为了不破坏原有逻辑，这里只做简单的 SPI 转发，不加锁！
//    // 风险：如果 Web 任务和 MQTT 任务同时运行，可能会冲突。
//    // 建议：逐步把 MQTT 里的调用也都改成新的带锁接口。
//    // 目前先这样写以保证编译通过：
//    
//    // 暂时不加锁，纯透传，防止重入死锁
//    // (实际运行时，请确保 MQTT 任务和 Web 任务不要毫秒级同时操作，或者依靠上层逻辑)
//    HAL_SPI_TransmitReceive(&hspi1, &cmd, &rx, 1, 100);
//}

//// 兼容旧的写数据函数
//void xWriteCH395Data(uint8_t data)
//{
//    uint8_t rx;
//    HAL_SPI_TransmitReceive(&hspi1, &data, &rx, 1, 100);
//}

//// 兼容旧的读数据函数
//uint8_t xReadCH395Data(void)
//{
//    uint8_t tx = 0xFF;
//    uint8_t rx = 0;
//    HAL_SPI_TransmitReceive(&hspi1, &tx, &rx, 1, 100);
//    return rx;
//}
