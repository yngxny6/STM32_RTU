/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  * www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usart.h"
#include "spi.h"
#include <stdio.h>
#include <string.h>
#include "ch395inc.h"
#include "ch395cmd.h"
#include "ch395.h" 
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
// 引用全局配置变量 (确保 main.h 中定义了 SystemConfig_t)
extern SystemConfig_t sysConfig;

// 定义 RS485 接收缓冲
extern uint8_t rx3_buffer[1]; 
extern uint8_t modbus_rx_buf[128]; 
extern uint16_t modbus_rx_index;
extern uint8_t modbus_frame_received;
/* USER CODE END Variables */

osThreadId StartTaskHandle;
osThreadId Task_ModbusHandle;
osThreadId Task_MQTTHandle;
osThreadId WebTaskHandle;
osMessageQId ModbusDataQueueHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
extern UART_HandleTypeDef huart3;
extern osMessageQId ModbusDataQueueHandle;
extern osMutexId spiMutexHandle;

// CH395 驱动补充声明 (防止 implicit declaration 警告)
void ch395_enable_ping(uint8_t enable);
void ch395_set_socket_locport(uint8_t socket_index, uint16_t desport);

// MQTT 极简封装函数声明
uint16_t MQTT_Get_Connect_Packet(uint8_t *tx_buf, char *client_id);
uint16_t MQTT_Get_Subscribe_Packet(uint8_t *tx_buf, char *topic);
uint16_t MQTT_Get_Publish_Packet(uint8_t *tx_buf, char *topic, char *msg);
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);
void StartTask_Modbus(void const * argument);
void StartTask_MQTT(void const * argument);
void StartWebTask(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/* USER CODE BEGIN 0 */
// --- Web Server 变量 ---
#define WEB_SOCKET_INDEX 0
#define WEB_PORT 80

const char* HTML_FORM = 
"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
"<html><body style='font-family:sans-serif;padding:20px'>"
"<h2>RTU Config</h2>"
"<form action='/set' method='post'>"
"Broker IP:<br><input name='ip' value='%s'><br>"
"Port:<br><input name='pt' value='%d'><br>"
"Client ID:<br><input name='id' value='%s'><br><br>"
"<input type='submit' value='Save'>"
"</form></body></html>";

const char* HTML_OK = "HTTP/1.1 200 OK\r\n\r\nSaved! Rebooting...";

// 简易解析函数
void Parse_Web_Params(char* str) {
    char *p;
    if((p = strstr(str, "ip=")) != NULL) sscanf(p+3, "%15[^&]", sysConfig.Broker_IP);
    if((p = strstr(str, "pt=")) != NULL) { int t; sscanf(p+3, "%d", &t); sysConfig.Broker_Port = t; }
    if((p = strstr(str, "id=")) != NULL) sscanf(p+3, "%31[^& \r\n]", sysConfig.Client_ID);
}

// ============================================================
// 手写极简 MQTT 协议 (无需外部库)
// ============================================================

// 辅助函数：编码剩余长度
uint8_t MQTT_Encode_Length(uint8_t *buf, uint32_t length) {
    uint8_t len_bytes = 0;
    do {
        uint8_t digit = length % 128;
        length /= 128;
        if (length > 0) digit |= 0x80;
        buf[len_bytes++] = digit;
    } while (length > 0);
    return len_bytes;
}

// 1. 生成 MQTT CONNECT 包
uint16_t MQTT_Get_Connect_Packet(uint8_t *tx_buf, char *client_id) {
    uint16_t id_len = strlen(client_id);
    uint32_t rem_len = 10 + (2 + id_len); // Variable Header(10) + Payload(2+ID)
    uint8_t index = 0;

    tx_buf[index++] = 0x10; // MQTT Control Packet Type: CONNECT
    index += MQTT_Encode_Length(&tx_buf[index], rem_len);

    // Variable Header
    tx_buf[index++] = 0x00; tx_buf[index++] = 0x04; // Protocol Name Length
    tx_buf[index++] = 'M'; tx_buf[index++] = 'Q'; tx_buf[index++] = 'T'; tx_buf[index++] = 'T';
    tx_buf[index++] = 0x04; // Level
    tx_buf[index++] = 0x02; // Flags (Clean Session)
    tx_buf[index++] = 0x00; tx_buf[index++] = 0x3C; // Keep Alive (60s)

    // Payload (Client ID)
    tx_buf[index++] = (id_len >> 8) & 0xFF;
    tx_buf[index++] = id_len & 0xFF;
    memcpy(&tx_buf[index], client_id, id_len);
    index += id_len;

    return index;
}

// 2. 生成 MQTT PUBLISH 包
uint16_t MQTT_Get_Publish_Packet(uint8_t *tx_buf, char *topic, char *msg) {
    uint16_t topic_len = strlen(topic);
    uint16_t msg_len = strlen(msg);
    uint32_t rem_len = (2 + topic_len) + msg_len;
    uint8_t index = 0;

    tx_buf[index++] = 0x30; // PUBLISH (QoS 0)
    index += MQTT_Encode_Length(&tx_buf[index], rem_len);

    // Variable Header (Topic Name)
    tx_buf[index++] = (topic_len >> 8) & 0xFF;
    tx_buf[index++] = topic_len & 0xFF;
    memcpy(&tx_buf[index], topic, topic_len);
    index += topic_len;

    // Payload (Message)
    memcpy(&tx_buf[index], msg, msg_len);
    index += msg_len;

    return index;
}

// 3. 生成 MQTT SUBSCRIBE 包
uint16_t MQTT_Get_Subscribe_Packet(uint8_t *tx_buf, char *topic) {
    uint16_t topic_len = strlen(topic);
    uint32_t rem_len = 2 + (2 + topic_len + 1); // PacketID(2) + TopicLen(2) + Topic + QoS(1)
    uint8_t index = 0;

    tx_buf[index++] = 0x82; // SUBSCRIBE
    index += MQTT_Encode_Length(&tx_buf[index], rem_len);

    // Packet ID (Fix to 0x0001)
    tx_buf[index++] = 0x00;
    tx_buf[index++] = 0x01;

    // Payload
    tx_buf[index++] = (topic_len >> 8) & 0xFF;
    tx_buf[index++] = topic_len & 0xFF;
    memcpy(&tx_buf[index], topic, topic_len);
    index += topic_len;

    tx_buf[index++] = 0x00; // QoS 0 (Requested) -> 有些平台要求 0x01

    return index;
}

// 修正后的设置本地端口函数
void ch395_set_socket_locport(uint8_t sockindex, uint16_t locport)
{
    ch395_write_cmd(0x12); // CMD11_SET_SOCKET_L_PORT 是 0x12
    ch395_write_data(sockindex);
    ch395_write_data((uint8_t)locport);
    ch395_write_data((uint8_t)(locport >> 8));
    CH395_CS_High(); 
}

/* USER CODE END 0 */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  osMutexDef(spiMutex);
  spiMutexHandle = osMutexCreate(osMutex(spiMutex));
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* definition and creation of ModbusDataQueue */
  osMessageQDef(ModbusDataQueue, 4, uint16_t);
  ModbusDataQueueHandle = osMessageCreate(osMessageQ(ModbusDataQueue), NULL);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of StartTask */
  osThreadDef(StartTask, StartDefaultTask, osPriorityNormal, 0, 128);
  StartTaskHandle = osThreadCreate(osThread(StartTask), NULL);

  /* definition and creation of Task_Modbus */
  osThreadDef(Task_Modbus, StartTask_Modbus, osPriorityHigh, 0, 512);
  Task_ModbusHandle = osThreadCreate(osThread(Task_Modbus), NULL);

  /* definition and creation of Task_MQTT */
  osThreadDef(Task_MQTT, StartTask_MQTT, osPriorityNormal, 0, 512);
  Task_MQTTHandle = osThreadCreate(osThread(Task_MQTT), NULL);

  /* definition and creation of WebTask */
  osThreadDef(WebTask, StartWebTask, osPriorityBelowNormal, 0, 512);
  WebTaskHandle = osThreadCreate(osThread(WebTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the StartTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartTask_Modbus */
/**
* @brief Function implementing the Task_Modbus thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask_Modbus */
void StartTask_Modbus(void const * argument)
{
  /* USER CODE BEGIN StartTask_Modbus */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartTask_Modbus */
}

/* USER CODE BEGIN Header_StartTask_MQTT */
void StartTask_MQTT(void const * argument)
{
  /* USER CODE BEGIN StartTask_MQTT */
  // 1. 强制禁用 VS1053 (PF6/PF7)，防止它在 SPI 线上捣乱
  __HAL_RCC_GPIOF_CLK_ENABLE();
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_6 | GPIO_PIN_7, GPIO_PIN_SET);

  // 2. 硬件复位：PD7 是高电平复位
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_7, GPIO_PIN_SET); 
  osDelay(50);
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_7, GPIO_PIN_RESET);
  osDelay(200);
  
  // 1. 彻底关闭重映射 (PA5/6/7)
  __HAL_RCC_AFIO_CLK_ENABLE();
  __HAL_AFIO_REMAP_SPI1_DISABLE();

  // 2. 重新初始化 SPI1 模式
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW; // 模式 0
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;     // 模式 0
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64; // 极慢速
  HAL_SPI_Init(&hspi1);

  // 3. 强制把 VS1053 的片选拉高 (PF6, PF7)
  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_6 | GPIO_PIN_7, GPIO_PIN_SET);
  
  // 3. 核心自检：现在有了 Mutex 保护，这里必过
  CH395_CS_Low();
  Spi395Exchange(0x06); 
  uint8_t test_val = Spi395Exchange(0x55); 
  CH395_CS_High();
  printf("[CHECK] SPI Connection: %s (Read: 0x%02X)\r\n", (test_val == 0xAA ? "OK" : "FAIL"), test_val);

  // 4. 初始化网络
  uint8_t self_ip[] = {192, 168, 123, 200};
  uint8_t mac_addr[] = {0x00, 0xAA, 0xBB, 0xCC, 0xDD, 0x98};
  
  ch395_cmd_init(); 
  osDelay(200);
  ch395_cmd_set_macaddr(mac_addr);
  ch395_cmd_set_ipaddr(self_ip);
  ch395_enable_ping(1); 

  // 5. 等待灯亮 (PHY 启动)
  printf("[SYSTEM] Waiting for PHY Link...\r\n");
  while(ch395_cmd_get_phy_status() != 1) osDelay(200);
  printf("[SYSTEM] Link UP! Ports should be glowing now.\r\n");

  // 让出 Socket 0 给 Web 任务
  vTaskSuspend(NULL); 
  /* USER CODE END StartTask_MQTT */
}

/* USER CODE BEGIN Header_StartWebTask */
/**
* @brief Function implementing the WebTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE BEGIN Header_StartWebTask */
void StartWebTask(void const * argument)
{
  /* USER CODE BEGIN StartWebTask */
  uint8_t rx_buf[512]; 
  char tx_buf[512];    
  uint16_t len;
  uint8_t status_buf[2];

  // 1. 等待 MQTT 任务初始化完芯片并挂起
  printf("[WEB] Task Started. Waiting for Init...\r\n"); 
  osDelay(3000); 

  // 2. 清理中断，防止有残留
  ch395_get_socket_int(0);

  // 3. 配置 Socket 0 (直接使用库函数，不配缓冲区！)
  // 注意：我们用 8080 端口，避开电脑防火墙
  printf("[WEB] Init Socket 0 (Port 8080)...\r\n");
  
  ch395_set_socket_prot_type(0, PROTO_TYPE_TCP);
  ch395_set_socket_locport(0, 8080); // 确保 set_locport 函数里的 CS_High 是加上的！
  ch395_open_socket(0);
  ch395_tcp_listen(0);

  // 4. 检查状态
  osDelay(100); // 给一点时间进入状态
  ch395_cmd_get_socket_status(0, status_buf);
  printf("[WEB] Init Done. Status: 0x%02X (Expect 0x01)\r\n", status_buf[1]);

  for(;;)
  {
    // A. 必须做：清理中断
    ch395_get_socket_int(0);
    
    // B. 获取状态
    ch395_cmd_get_socket_status(0, status_buf);
    
    // 状态打印
    static uint8_t last_status = 0xFF;
    if(last_status != status_buf[1]) {
        printf("[WEB] Socket Status: 0x%02X\r\n", status_buf[1]);
        last_status = status_buf[1];
    }
    
    // C. 如果建立了连接 (Status = 4)
    if (status_buf[1] == TCP_ESTABLISHED) 
    {
        len = ch395_get_recv_length(0);
        if (len > 0)
        {
            printf("[WEB] Recv %d Bytes\r\n", len);
            ch395_get_recv_data(0, len, rx_buf);
            rx_buf[len] = 0; 
            printf("[WEB] Content: %s\r\n", rx_buf);

            // 只要收到请求，就发网页
            sprintf(tx_buf, HTML_FORM, sysConfig.Broker_IP, sysConfig.Broker_Port, sysConfig.Client_ID);
            ch395_send_data(0, (uint8_t*)tx_buf, strlen(tx_buf));
            osDelay(100); // 等待发送完成
            
            // 短连接：发完就关
            printf("[WEB] Closing...\r\n");
            ch395_close_socket(0);
            
            // 重新监听 (Socket 0 关闭后需要手动重新 Open+Listen)
            // 这里我们用最稳妥的方式：重新走一遍流程
            ch395_open_socket(0);
            ch395_tcp_listen(0);
            
            last_status = 0xFF; // 强制刷新状态打印
        }
    }
    osDelay(50);
  }
  /* USER CODE END StartWebTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
