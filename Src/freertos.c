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
#include "spi.h"
#include "mqtt.h"
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
// 瀹氫箟 RS485 鎺ユ敹缂撳啿鍖?
extern uint8_t rx3_buffer[1]; 
extern uint8_t modbus_rx_buf[128]; 
extern uint16_t modbus_rx_index;
extern uint8_t modbus_frame_received;
/* USER CODE END Variables */
osThreadId StartTaskHandle;
osThreadId Task_ModbusHandle;
osThreadId Task_MQTTHandle;
osThreadId Task_WebServerHandle;
osMessageQId ModbusDataQueueHandle;
osMutexId mySpiMutexHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
// 澹版槑寮曠敤澶栭儴鍙橀噺
extern UART_HandleTypeDef huart3;
extern osMessageQId ModbusDataQueueHandle;
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);
void StartTask_Modbus(void const * argument);
void StartTask_MQTT(void const * argument);
void StartTask_WebServer(void const * argument);

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
  /* Create the mutex(es) */
  /* definition and creation of mySpiMutex */
  osMutexDef(mySpiMutex);
  mySpiMutexHandle = osMutexCreate(osMutex(mySpiMutex));

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

  /* definition and creation of Task_WebServer */
  osThreadDef(Task_WebServer, StartTask_WebServer, osPriorityNormal, 0, 512);
  Task_WebServerHandle = osThreadCreate(osThread(Task_WebServer), NULL);

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
/**
* @brief Function implementing the Task_MQTT thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask_MQTT */
void StartTask_MQTT(void const * argument)
{
  /* USER CODE BEGIN StartTask_MQTT */
  uint8_t ch395_ver = 0;
  uint16_t received_val; 
  char json_buffer[64];      
  uint8_t mqtt_tx_buf[256]; 
  uint16_t mqtt_len = 0;
  
  // 鎺ユ敹缂撳啿鍖?
  uint8_t rx_buf[256]; // 鍔犲ぇ涓?鐐?
  uint16_t rx_len = 0;
  
  uint8_t socket_index = 0; 
  uint8_t sock_status[2]; 
  
  // 鐘舵?佹満: 0=鏈繛鎺?, 1=宸茶繛鎺CP, 2=宸茶闃匨QTT
  uint8_t mqtt_state = 0; 
  
  // ================= 1. 缃戠粶鍙傛暟 =================
  uint8_t target_ip[] = {192, 168, 123, 88}; 
  uint16_t target_port = 1883; 

  uint8_t self_ip[] = {192, 168, 123, 200};
  uint8_t gateway_ip[] = {192, 168, 123, 88}; 
  uint8_t mask_addr[] = {255, 255, 255, 0}; 
  uint8_t mac_addr[] = {0x00, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE};

  // ================= 2. 鍒濆鍖? =================
  printf("CH395Q: Resetting...\r\n");
  ch395_hardware_reset(); osDelay(200);
  
  printf("CH395Q: Config Net Params...\r\n");
  ch395_cmd_set_macaddr(mac_addr);
  ch395_cmd_set_ipaddr(self_ip);
  ch395_cmd_set_gw_ipaddr(gateway_ip);
  ch395_cmd_set_maskaddr(mask_addr); 
  osDelay(50); 
  
  printf("--- SPI Hardware Test Start ---\r\n");

// 1. 测试指令: CMD_CHECK_EXIST (0x06)
// 逻辑: 发送 0x55，芯片应该按位取反返回 0xAA (二进制 01010101 -> 10101010)
uint8_t check_val = 0x55; 
uint8_t result = 0;

CH395_CS_Low();
uint8_t cmd = 0x06; // CMD_CHECK_EXIST
HAL_SPI_Transmit(&hspi1, &cmd, 1, 100);       // 发送命令
HAL_SPI_TransmitReceive(&hspi1, &check_val, &result, 1, 100); // 发送数据并读取接收
CH395_CS_High();

printf("Test Result: Send 0x55, Read 0x%02X\r\n", result);

if (result == 0xAA) {
    printf(">> HARDWARE SUCCESS: CH395 is Alive!\r\n");
} else {
    printf(">> HARDWARE FAIL: SPI Error or Chip Dead.\r\n");
    // 如果读到 00，通常是 MISO 线没接好，或者 SPI 模式(CPOL/CPHA)不对
    // 如果读到 FF，通常是片选没拉低，或者 MISO 被其他设备干扰
}
printf("--- SPI Hardware Test End ---\r\n");
  
  ch395_cmd_init(); osDelay(200);
  ch395_enable_ping(1); 

  printf("CH395Q: Waiting for Link...\r\n");
  while(ch395_cmd_get_phy_status() == 1) { osDelay(100); }
  printf("CH395Q: Link UP!\r\n");

  // ================= 3. 杩炴帴 Broker =================
  ch395_set_socket_prot_type(socket_index, PROTO_TYPE_TCP);
  ch395_set_socket_desip(socket_index, target_ip);
  ch395_set_socket_desport(socket_index, target_port);
  
  printf("CH395Q: Connect to MQTT...\r\n");
  ch395_open_socket(socket_index); 
  osDelay(20);
  ch395_tcp_connect(socket_index);

  // ================= 4. 涓诲惊鐜? =================
  for(;;)
  {
    // A. 蹇呴』鍋氱殑锛氭竻鐞嗕腑鏂? + 璇诲彇鏁版嵁
    uint8_t int_stat = ch395_get_socket_int(socket_index);
    
    // 璇诲彇鏁版嵁 (杩欓噷瀹炵幇浜嗕笅琛屾帶鍒惰В鏋?)
    if(ch395_get_recv_length(socket_index) > 0)
    {
        rx_len = ch395_get_recv_length(socket_index);
        ch395_get_recv_data(socket_index, rx_len, rx_buf);
        rx_buf[rx_len] = 0; // 琛ラ浂锛岀‘淇濇渶鍚庢湁涓?涓粨鏉熺

        // ========================================================
        // !!! 鏍稿績淇锛氭竻娲楁暟鎹? !!!
        // MQTT 鎶ユ枃涓寘鍚? 0x00 (濡備富棰橀暱搴﹂珮浣?)锛屼細瀵艰嚧 strstr 鎻愬墠鎴柇銆?
        // 鎴戜滑鎶婁腑闂寸殑 0x00 鏇挎崲鎴愮┖鏍硷紝璁? strstr 鑳界┛閫忚繃鍘绘壘鍒? JSON銆?
        // ========================================================
        for(int i = 0; i < rx_len; i++)
        {
            if(rx_buf[i] == 0x00) 
            {
                rx_buf[i] = ' '; // 鏇挎崲鎴愮┖鏍?
            }
        }
        
        // 鎵撳嵃娓呮礂鍚庣殑鏁版嵁
        printf("DEBUG Recv: [%s]\r\n", rx_buf);
        
        // --- 鏅鸿兘妯＄硦瑙ｆ瀽 ---
        // 1. 鍏堟壘鍒? "led" 鍦ㄥ摢
        char *pLed = strstr((char*)rx_buf, "led");
        
        if(pLed != NULL)
        {
            // 2. !!! 鍏抽敭淇敼 !!!
            // 鍙湪 "led" 鍚庨潰瀵绘壘 '0' 鎴? '1'
            // 杩欐牱灏变笉浼氳鎶ュご鐨? '0' 缁欒瀵间簡锛?
            
            // 鍏堟壘 '1' (寮?鐏?) 鈥斺?? 浼樺厛鍒ゆ柇寮?鐏紝閫昏緫鏇村畨鍏?
            if(strchr(pLed, '1') != NULL)
            {
                 printf("CMD: LED ON (Set Low)\r\n");
                 HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, GPIO_PIN_RESET); // 浜伅
            }
            // 鍐嶆壘 '0' (鍏崇伅)
            else if(strchr(pLed, '0') != NULL) 
            {
                 printf("CMD: LED OFF (Set High)\r\n");
                 HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, GPIO_PIN_SET);   // 鐏伅
            }
        }
    }

    ch395_cmd_get_socket_status(socket_index, sock_status);
    
    // 鐘舵?? 4 = TCP 宸插缓绔?
    if(sock_status[1] == TCP_ESTABLISHED)
    {
        // 闃舵 1: 鍙戦?? CONNECT
        if(mqtt_state == 0)
        {
             printf("Sending MQTT CONNECT...\r\n");
             mqtt_len = MQTT_Get_Connect_Packet(mqtt_tx_buf, "STM32_Graduate");
             ch395_send_data(socket_index, mqtt_tx_buf, mqtt_len);
             mqtt_state = 1; 
             osDelay(1000); 
        }
        // 闃舵 2: 鍙戦?? SUBSCRIBE (璁㈤槄 device/control)
        else if(mqtt_state == 1)
        {
             printf("Subscribing to 'device/control'...\r\n");
             mqtt_len = MQTT_Get_Subscribe_Packet(mqtt_tx_buf, "device/control");
             ch395_send_data(socket_index, mqtt_tx_buf, mqtt_len);
             mqtt_state = 2; // 鍏ㄩ儴鍑嗗灏辩华锛?
             osDelay(1000);
        }
        
        // 闃舵 3: 姝ｅ父涓氬姟 (鍙戝竷鏁版嵁)
        if(mqtt_state == 2 && xQueueReceive(ModbusDataQueueHandle, &received_val, 100) == pdTRUE)
        {
             // 鍙湁褰撴病鏈夋敹鍒版帶鍒舵寚浠ゆ椂锛孡ED鎵嶄綔涓哄彂閫佹寚绀虹伅缈昏浆
             // 杩欓噷涓轰簡婕旂ず鎺у埗鏁堟灉锛屽彲浠ュ厛鎶婅嚜鍔ㄧ炕杞敞閲婃帀锛屾垨鑰呬繚鐣?
             //HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_5); 
             
             memset(json_buffer, 0, sizeof(json_buffer)); 
             sprintf(json_buffer, "{\"temp\": %d}", received_val);
             
             // 鎵撳寘 PUBLISH 鎶ユ枃
             mqtt_len = MQTT_Get_Publish_Packet(mqtt_tx_buf, "device/data", json_buffer);
             
             // 鍙戦??
             printf("MQTT: Publish %s\r\n", json_buffer);
             ch395_send_data(socket_index, mqtt_tx_buf, mqtt_len);
        }
    }
    else if(sock_status[1] == TCP_CLOSED)
    {
        printf("TCP Closed. Reconnecting...\r\n");
        mqtt_state = 0; // 閲嶇疆鐘舵?侊紝閲嶆柊璧扮櫥褰曟祦绋?
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

/* USER CODE BEGIN Header_StartTask_WebServer */
/**
* @brief Function implementing the Task_WebServer thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask_WebServer */
void StartTask_WebServer(void const * argument)
{
  /* USER CODE BEGIN StartTask_WebServer */
  
  uint8_t socket_index = 1; // 既然MQTT用了0，我们Web用1
  uint16_t len = 0;
  uint8_t rx_buffer[512];   // 接收缓冲区
  char html_buffer[512];    // 发送缓冲区
  
  // 1. 延时等待 CH395 初始化完毕 (假设 main.c 里已经 Reset 过了)
  osDelay(1000); 

  // 2. 配置 Socket 1 为 TCP 服务器，端口 80
  CH395_SetSocket_Type(socket_index, PROTO_TYPE_TCP);
  CH395_SetSocket_Port(socket_index, 80);
  CH395_OpenSocket(socket_index);
  CH395_TCP_Listen(socket_index); // 进入监听状态，等待浏览器连接

  /* Infinite loop */
  for(;;)
  {
    // 3. 轮询是否有数据到来
    // (CH395中断方式更好，但轮询适合新手理解)
    len = CH395_Get_RecvLen(socket_index);
    
    if (len > 0) 
    {
        // 限制读取长度，防止溢出
        if(len > 511) len = 511;
        
        // 4. 读取浏览器发来的 HTTP 请求
        CH395_Get_Data(socket_index, rx_buffer, len);
        rx_buffer[len] = '\0'; // 添加字符串结束符
        
        // 5. 判断是不是 GET 请求
        if (strstr((char*)rx_buffer, "GET") != NULL) 
        {
            // 6. 准备要发送的 HTML 网页
            // 这里我们先做一个最简单的网页，显示当前状态
            sprintf(html_buffer, 
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/html\r\n"
                "Connection: close\r\n\r\n"
                "<html><body>"
                "<h1>STM32 RTU Web Config</h1>"
                "<form>"
                "MQTT IP: <input type='text' value='192.168.1.100'><br>"
                "Port: <input type='text' value='1883'><br>"
                "<input type='submit' value='Save'>"
                "</form>"
                "</body></html>"
            );
            
            // 7. 发送回去
            CH395_Send_Data(socket_index, (uint8_t*)html_buffer, strlen(html_buffer));
            
            // 8. 稍作延时后断开连接 (HTTP短连接)
            osDelay(50); 
            CH395_TCP_Disconnect(socket_index);
            
            // 9. 重新进入监听，等待下一次刷新
            // CH395 某些模式下断开后需要重新执行 Listen，具体视固件版本
            // 保险起见，再次执行监听
            CH395_OpenSocket(socket_index); 
            CH395_TCP_Listen(socket_index); 
        }
    }
    
    // 没数据时，释放CPU给别的任务 (如MQTT)
    osDelay(20);
  }
  /* USER CODE END StartTask_WebServer */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
