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
  *                             www.st.com/SLA0044
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
// 定义 RS485 接收缓冲区
extern uint8_t rx3_buffer[1]; 
extern uint8_t modbus_rx_buf[128]; 
extern uint16_t modbus_rx_index;
extern uint8_t modbus_frame_received;
/* USER CODE END Variables */
osThreadId StartTaskHandle;
osThreadId Task_ModbusHandle;
osThreadId Task_MQTTHandle;
osMessageQId ModbusDataQueueHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
// 声明引用外部变量
extern UART_HandleTypeDef huart3;
extern osMessageQId ModbusDataQueueHandle;
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);
void StartTask_Modbus(void const * argument);
void StartTask_MQTT(void const * argument);

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
  // Modbus 读取保持寄存器指令: 01 03 00 00 00 01 84 0A (读地址1的寄存器0，共1个)
  uint8_t modbus_query[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x01, 0x84, 0x0A};
  uint16_t sensor_value = 0;

  for(;;)
  {
    // 1. 发送查询指令
    HAL_UART_Transmit(&huart3, modbus_query, 8, 100);
    printf("Modbus: Sent Query...\r\n");

    // 2. 延时等待数据回传 (简单粗暴法)
    osDelay(200); 

    // 3. 检查是否收到数据
    if(modbus_frame_received == 1)
    {
        modbus_frame_received = 0;
        // 解析数据: 第4和第5字节是数据 (索引3和4)
        // 假设回传: 01 03 02 [High] [Low] CRC CRC
        uint8_t high_byte = modbus_rx_buf[3];
        uint8_t low_byte = modbus_rx_buf[4];
        sensor_value = (high_byte << 8) | low_byte;

        printf("Modbus: Got Value = %d\r\n", sensor_value);

        // 4. 发送到队列给 MQTT 任务
        xQueueSend(ModbusDataQueueHandle, &sensor_value, 0);
    }
    else
    {
        printf("Modbus: Timeout!\r\n");
    }

    osDelay(1000); // 1秒采集一次
  }
}

// ================= MQTT 协议封装工具函数 =================

// 1. 生成 MQTT 连接报文 (CONNECT)
// 返回值：报文总长度
uint16_t MQTT_Get_Connect_Packet(uint8_t *buff, char *client_id)
{
    uint16_t len = strlen(client_id);
    uint16_t index = 0;

    // Fixed Header (0x10 = CONNECT)
    buff[index++] = 0x10; 
    // Remaining Length (后面数据的长度)
    // 简单计算：VarHeader(10) + ClientID_Len(2) + ClientID
    uint8_t remain_len = 10 + 2 + len;
    buff[index++] = remain_len;

    // Variable Header
    // Protocol Name: "MQTT"
    buff[index++] = 0x00; buff[index++] = 0x04;
    buff[index++] = 'M'; buff[index++] = 'Q'; buff[index++] = 'T'; buff[index++] = 'T';
    // Protocol Level (4 = v3.1.1)
    buff[index++] = 0x04;
    // Connect Flags (0x02 = Clean Session)
    buff[index++] = 0x02;
    // Keep Alive (60s)
    buff[index++] = 0x00; buff[index++] = 60;

    // Payload (Client ID)
    buff[index++] = (len >> 8);
    buff[index++] = (len & 0xFF);
    memcpy(&buff[index], client_id, len);
    index += len;

    return index;
}

// 2. 生成 MQTT 发布报文 (PUBLISH)
// topic: 主题 (如 "dev/temp")
// msg: 消息内容 (如 "{\"val\":25}")
uint16_t MQTT_Get_Publish_Packet(uint8_t *buff, char *topic, char *msg)
{
    uint16_t topic_len = strlen(topic);
    uint16_t msg_len = strlen(msg);
    uint16_t index = 0;
    
    // Fixed Header (0x30 = PUBLISH)
    buff[index++] = 0x30;
    
    // Remaining Length calculation
    // TopicLen(2) + Topic + Payload
    uint16_t remain_len = 2 + topic_len + msg_len;
    
    // MQTT剩余长度编码 (支持 < 128字节的小包)
    // 如果数据太长(>127)，这里需要复杂的算法，但传个温度JSON足够了
    if(remain_len > 127) {
        buff[index++] = (remain_len % 128) | 0x80;
        buff[index++] = (remain_len / 128);
    } else {
        buff[index++] = remain_len;
    }

    // Variable Header: Topic Name
    buff[index++] = (topic_len >> 8);
    buff[index++] = (topic_len & 0xFF);
    memcpy(&buff[index], topic, topic_len);
    index += topic_len;

    // Payload: Message
    memcpy(&buff[index], msg, msg_len);
    index += msg_len;

    return index;
}

// 3. 生成 MQTT 订阅报文 (SUBSCRIBE)
// topic: 要订阅的主题 (如 "device/control")
uint16_t MQTT_Get_Subscribe_Packet(uint8_t *buff, char *topic)
{
    uint16_t topic_len = strlen(topic);
    uint16_t index = 0;
    
    // Fixed Header (0x82 = SUBSCRIBE)
    buff[index++] = 0x82;
    // Remaining Length: PacketID(2) + TopicLen(2) + Topic + QoS(1)
    uint8_t remain_len = 2 + 2 + topic_len + 1;
    buff[index++] = remain_len;
    
    // Variable Header: Packet ID (随便填，比如 0x0001)
    buff[index++] = 0x00; buff[index++] = 0x01;
    
    // Payload: Topic
    buff[index++] = (topic_len >> 8);
    buff[index++] = (topic_len & 0xFF);
    memcpy(&buff[index], topic, topic_len);
    index += topic_len;
    
    // QoS (0)
    buff[index++] = 0x00;
    
    return index;
}

/**
* @brief Function implementing the Task_MQTT thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE BEGIN Header_StartTask_MQTT */
void StartTask_MQTT(void const * argument)
{
  /* USER CODE BEGIN StartTask_MQTT */
  uint8_t ch395_ver = 0;
  uint16_t received_val; 
  char json_buffer[64];      
  uint8_t mqtt_tx_buf[256]; 
  uint16_t mqtt_len = 0;
  
  // 接收缓冲区
  uint8_t rx_buf[256]; // 加大一点
  uint16_t rx_len = 0;
  
  uint8_t socket_index = 0; 
  uint8_t sock_status[2]; 
  
  // 状态机: 0=未连接, 1=已连接TCP, 2=已订阅MQTT
  uint8_t mqtt_state = 0; 
  
  // ================= 1. 网络参数 =================
  uint8_t target_ip[] = {192, 168, 123, 88}; 
  uint16_t target_port = 1883; 

  uint8_t self_ip[] = {192, 168, 123, 200};
  uint8_t gateway_ip[] = {192, 168, 123, 88}; 
  uint8_t mask_addr[] = {255, 255, 255, 0}; 
  uint8_t mac_addr[] = {0x00, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE};

  // ================= 2. 初始化 =================
  printf("CH395Q: Resetting...\r\n");
  ch395_hardware_reset(); osDelay(200);
  
  printf("CH395Q: Config Net Params...\r\n");
  ch395_cmd_set_macaddr(mac_addr);
  ch395_cmd_set_ipaddr(self_ip);
  ch395_cmd_set_gw_ipaddr(gateway_ip);
  ch395_cmd_set_maskaddr(mask_addr); 
  osDelay(50); 
  
  ch395_cmd_init(); osDelay(200);
  ch395_enable_ping(1); 

  printf("CH395Q: Waiting for Link...\r\n");
  while(ch395_cmd_get_phy_status() == 1) { osDelay(100); }
  printf("CH395Q: Link UP!\r\n");

  // ================= 3. 连接 Broker =================
  ch395_set_socket_prot_type(socket_index, PROTO_TYPE_TCP);
  ch395_set_socket_desip(socket_index, target_ip);
  ch395_set_socket_desport(socket_index, target_port);
  
  printf("CH395Q: Connect to MQTT...\r\n");
  ch395_open_socket(socket_index); 
  osDelay(20);
  ch395_tcp_connect(socket_index);

  // ================= 4. 主循环 =================
  for(;;)
  {
    // A. 必须做的：清理中断 + 读取数据
    uint8_t int_stat = ch395_get_socket_int(socket_index);
    
    // 读取数据 (这里实现了下行控制解析)
    if(ch395_get_recv_length(socket_index) > 0)
    {
        rx_len = ch395_get_recv_length(socket_index);
        ch395_get_recv_data(socket_index, rx_len, rx_buf);
        rx_buf[rx_len] = 0; // 补零，确保最后有一个结束符

        // ========================================================
        // !!! 核心修复：清洗数据 !!!
        // MQTT 报文中包含 0x00 (如主题长度高位)，会导致 strstr 提前截断。
        // 我们把中间的 0x00 替换成空格，让 strstr 能穿透过去找到 JSON。
        // ========================================================
        for(int i = 0; i < rx_len; i++)
        {
            if(rx_buf[i] == 0x00) 
            {
                rx_buf[i] = ' '; // 替换成空格
            }
        }
        
        // 打印清洗后的数据
        printf("DEBUG Recv: [%s]\r\n", rx_buf);
        
        // --- 智能模糊解析 ---
        // 1. 先找到 "led" 在哪
        char *pLed = strstr((char*)rx_buf, "led");
        
        if(pLed != NULL)
        {
            // 2. !!! 关键修改 !!!
            // 只在 "led" 后面寻找 '0' 或 '1'
            // 这样就不会被报头的 '0' 给误导了！
            
            // 先找 '1' (开灯) —— 优先判断开灯，逻辑更安全
            if(strchr(pLed, '1') != NULL)
            {
                 printf("CMD: LED ON (Set Low)\r\n");
                 HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, GPIO_PIN_RESET); // 亮灯
            }
            // 再找 '0' (关灯)
            else if(strchr(pLed, '0') != NULL) 
            {
                 printf("CMD: LED OFF (Set High)\r\n");
                 HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, GPIO_PIN_SET);   // 灭灯
            }
        }
    }

    ch395_cmd_get_socket_status(socket_index, sock_status);
    
    // 状态 4 = TCP 已建立
    if(sock_status[1] == TCP_ESTABLISHED)
    {
        // 阶段 1: 发送 CONNECT
        if(mqtt_state == 0)
        {
             printf("Sending MQTT CONNECT...\r\n");
             mqtt_len = MQTT_Get_Connect_Packet(mqtt_tx_buf, "STM32_Graduate");
             ch395_send_data(socket_index, mqtt_tx_buf, mqtt_len);
             mqtt_state = 1; 
             osDelay(1000); 
        }
        // 阶段 2: 发送 SUBSCRIBE (订阅 device/control)
        else if(mqtt_state == 1)
        {
             printf("Subscribing to 'device/control'...\r\n");
             mqtt_len = MQTT_Get_Subscribe_Packet(mqtt_tx_buf, "device/control");
             ch395_send_data(socket_index, mqtt_tx_buf, mqtt_len);
             mqtt_state = 2; // 全部准备就绪！
             osDelay(1000);
        }
        
        // 阶段 3: 正常业务 (发布数据)
        if(mqtt_state == 2 && xQueueReceive(ModbusDataQueueHandle, &received_val, 100) == pdTRUE)
        {
             // 只有当没有收到控制指令时，LED才作为发送指示灯翻转
             // 这里为了演示控制效果，可以先把自动翻转注释掉，或者保留
             //HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_5); 
             
             memset(json_buffer, 0, sizeof(json_buffer)); 
             sprintf(json_buffer, "{\"temp\": %d}", received_val);
             
             // 打包 PUBLISH 报文
             mqtt_len = MQTT_Get_Publish_Packet(mqtt_tx_buf, "device/data", json_buffer);
             
             // 发送
             printf("MQTT: Publish %s\r\n", json_buffer);
             ch395_send_data(socket_index, mqtt_tx_buf, mqtt_len);
        }
    }
    else if(sock_status[1] == TCP_CLOSED)
    {
        printf("TCP Closed. Reconnecting...\r\n");
        mqtt_state = 0; // 重置状态，重新走登录流程
        ch395_open_socket(socket_index);
        osDelay(20);
        ch395_tcp_connect(socket_index);
        osDelay(2000);
    }
    else
    {
        osDelay(100);
    }
  }
  /* USER CODE END StartTask_MQTT */
}
/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
