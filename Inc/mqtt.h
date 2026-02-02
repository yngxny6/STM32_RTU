#ifndef __MQTT_H
#define __MQTT_H

#include "main.h"

// 声明这三个你在 freertos.c 里调用的函数
int MQTT_Get_Connect_Packet(uint8_t *buff, char *client_id);
int MQTT_Get_Publish_Packet(uint8_t *buff, char *topic, char *payload);
int MQTT_Get_Subscribe_Packet(uint8_t *buff, char *topic);

#endif
