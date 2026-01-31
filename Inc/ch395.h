#ifndef __CH395_H__
#define __CH395_H__

#include "main.h"

/* ================= 硬件引脚定义 (适配战舰V4) ================= */
#define CH395_CS_PORT    GPIOG
#define CH395_CS_PIN     GPIO_PIN_9
#define CH395_INT_PORT   GPIOG
#define CH395_INT_PIN    GPIO_PIN_6
#define CH395_RST_PORT   GPIOD
#define CH395_RST_PIN    GPIO_PIN_7

/* ================= 你的 ch395.c 里的函数声明 ================= */
void CH395_Init_Hardware(void);
void xWriteCH395Cmd(uint8_t cmd);
void xWriteCH395Data(uint8_t data);
uint8_t xReadCH395Data(void);
void CH395_CS_High(void);
void CH395_CS_Low(void);

/* =============================================================== */
/* !!! 关键修复：添加“翻译官”，让 ch395cmd.c 能看懂你的驱动 !!! */
/* =============================================================== */

/* 1. 兼容片选操作 (原子哥代码里把 High 拼成了 Hign，必须兼容它) */
/* 注意：这里定义成函数调用，因为你 ch395cmd.c 里写的是 ch395_scs_hign; */
#define ch395_scs_hign      CH395_CS_High() 
#define ch395_scs_low       CH395_CS_Low()

/* 2. 兼容读写函数名 */
#define ch395_write_cmd(x)  xWriteCH395Cmd(x)
#define ch395_write_data(x) xWriteCH395Data(x)
#define ch395_read_data()   xReadCH395Data()

/* 3. 兼容初始化和复位 */
#define ch395_gpio_init()   CH395_Init_Hardware()
/* 硬件复位宏 */
#define ch395_hardware_reset() do{ \
    HAL_GPIO_WritePin(CH395_RST_PORT, CH395_RST_PIN, GPIO_PIN_RESET); \
    HAL_Delay(50); \
    HAL_GPIO_WritePin(CH395_RST_PORT, CH395_RST_PIN, GPIO_PIN_SET); \
    HAL_Delay(200); \
}while(0)

#endif
