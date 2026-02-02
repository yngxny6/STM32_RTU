#ifndef __CH395_H
#define __CH395_H

#include "main.h"
#include "ch395inc.h"

/* ================= 1. 硬件引脚定义 (适配你的战舰V4) ================= */
#define CH395_CS_PORT     GPIOG
#define CH395_CS_PIN      GPIO_PIN_9
#define CH395_INT_PORT    GPIOG
#define CH395_INT_PIN     GPIO_PIN_6
#define CH395_RST_PORT    GPIOD
#define CH395_RST_PIN     GPIO_PIN_7

/* ================= 2. CH395 命令码 (Web Server 必需) ================= */
#define CMD_SET_IP_ADDR     0x0B  // 设置IP地址
#define CMD_SET_GW_IP_ADDR  0x0D  // 设置网关
#define CMD_SET_MASK_ADDR   0x0C  // 设置掩码
#define CMD_INIT_CH395      0x04  // 初始化
#define CMD_GET_PHY_STATUS  0x54  // 获取PHY状态
#define CMD_SET_PROTO_TYPE  0x30  // 设置Socket协议类型
#define CMD_SET_LOCAL_PORT  0x32  // 设置Socket本地端口
#define CMD_OPEN_SOCKET     0x2E  // 打开Socket
#define CMD_TCP_LISTEN      0x36  // TCP监听
#define CMD_TCP_CONNECT     0x34  // TCP连接
#define CMD_TCP_DISCONNECT  0x35  // TCP断开
#define CMD_GET_RECV_LEN    0x3B  // 获取接收缓冲区长度
#define CMD_GET_RECV_DATA   0x27  // 读取接收缓冲区数据
#define CMD_SEND_DATA       0x28  // 发送数据

// 协议类型定义
//#define PROTO_TYPE_TCP      0x03

/* ================= 3. 函数声明 (新旧共存) ================= */

// --- A. 你原有的旧接口 (保留给 ch395cmd.c 使用) ---
void xWriteCH395Cmd(uint8_t cmd);
void xWriteCH395Data(uint8_t data);
uint8_t xReadCH395Data(void);
void CH395_CS_High(void);
void CH395_CS_Low(void);

// --- B. 我新增的高级接口 (给 Web Server 使用) ---
void CH395_Init_Hardware(void);
void CH395_SetSocket_Type(uint8_t sockIndex, uint8_t protoType);
void CH395_SetSocket_Port(uint8_t sockIndex, uint16_t port);
uint8_t CH395_OpenSocket(uint8_t sockIndex);
uint8_t CH395_TCP_Listen(uint8_t sockIndex);
uint8_t CH395_TCP_Disconnect(uint8_t sockIndex);
uint16_t CH395_Get_RecvLen(uint8_t sockIndex);
void CH395_Get_Data(uint8_t sockIndex, uint8_t *pBuf, uint16_t len);
void CH395_Send_Data(uint8_t sockIndex, uint8_t *pBuf, uint16_t len);

/* ================= 4. 兼容性宏 (翻译官) ================= */
/* 这里的宏是为了让原子哥/官方的 ch395cmd.c 能继续工作 */

// 兼容拼写错误 (High -> Hign)
#define ch395_scs_hign      CH395_CS_High() 
#define ch395_scs_low       CH395_CS_Low()

// 映射读写函数
#define ch395_write_cmd(x)  xWriteCH395Cmd(x)
#define ch395_write_data(x) xWriteCH395Data(x)
#define ch395_read_data()   xReadCH395Data()

// 映射初始化
#define ch395_gpio_init()   CH395_Init_Hardware()

// 硬件复位宏
#define ch395_hardware_reset() do{ \
    HAL_GPIO_WritePin(CH395_RST_PORT, CH395_RST_PIN, GPIO_PIN_RESET); \
    HAL_Delay(50); \
    HAL_GPIO_WritePin(CH395_RST_PORT, CH395_RST_PIN, GPIO_PIN_SET); \
    HAL_Delay(200); \
}while(0)

#endif
