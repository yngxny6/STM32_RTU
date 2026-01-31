#include "ch395.h"
#include "spi.h"   // 必须包含 CubeMX 生成的 spi.h
#include "gpio.h"
#include <stdio.h>

/* 引用外部的 SPI 句柄，在 main.c 或 spi.c 中定义 */
extern SPI_HandleTypeDef hspi1;

/**
  * @brief  控制 CS 片选引脚为高电平
  */
void CH395_CS_High(void)
{
    HAL_GPIO_WritePin(CH395_CS_PORT, CH395_CS_PIN, GPIO_PIN_SET);
}

/**
  * @brief  控制 CS 片选引脚为低电平
  */
void CH395_CS_Low(void)
{
    HAL_GPIO_WritePin(CH395_CS_PORT, CH395_CS_PIN, GPIO_PIN_RESET);
}

/**
  * @brief  SPI 交换一个字节 (发送并接收)
  */
uint8_t Spi395Exchange(uint8_t d)
{
    uint8_t rx_data = 0;
    // 使用 HAL 库的 SPI 读写函数，超时时间设为 10ms
    if (HAL_SPI_TransmitReceive(&hspi1, &d, &rx_data, 1, 10) != HAL_OK)
    {
        // 如果出错，可以打印个错误日志，或者重置 SPI
        // printf("SPI Error!\r\n"); 
        return 0xFF;
    }
    return rx_data;
}

/**
  * @brief  写命令到 CH395
  */
void xWriteCH395Cmd(uint8_t cmd)
{
    CH395_CS_High(); // 习惯性先拉高，防止时序错乱
    CH395_CS_Low();  // 拉低片选，开始传输
    Spi395Exchange(cmd);
    // 注意：写完命令后保持 CS 为低，或者根据具体指令时序决定
    // 大部分 CH395 操作是：CS低 -> 写命令 -> 写数据/读数据 -> CS高
    // 这里我们仅发送命令，CS 的释放由上层函数或连续操作决定
    // 但为了适配原子哥的逻辑，通常命令和数据是连续的，这里暂时不拉高 CS
    // *修正*：根据原版驱动逻辑，写单字节命令后通常需要后续操作，
    // 如果是单字节指令，上层函数会负责拉高 CS。
}

/**
  * @brief  写数据到 CH395
  */
void xWriteCH395Data(uint8_t data)
{
    Spi395Exchange(data);
}

/**
  * @brief  从 CH395 读数据
  */
uint8_t xReadCH395Data(void)
{
    return Spi395Exchange(0xFF); // 发送空字节(0xFF)以产生时钟读取数据
}

/**
  * @brief  初始化 CH395 相关的硬件
  * (其实在 main.c 的 MX_GPIO_Init 和 MX_SPI1_Init 已经做过了，这里做个双保险)
  */
void CH395_Init_Hardware(void)
{
    // 拉高 CS，确保从机不被误选中
    CH395_CS_High();
    
    // 可以在这里复位一下 CH395
    // PD7 是复位引脚
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_7, GPIO_PIN_RESET); // 复位
    HAL_Delay(50); // 这里的延时如果是 RTOS 运行前调用用 HAL_Delay，运行后建议用 osDelay
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_7, GPIO_PIN_SET);   // 结束复位
    HAL_Delay(200); // 等待芯片复位完成
}
