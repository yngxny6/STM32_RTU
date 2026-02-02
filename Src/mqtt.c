#include "mqtt.h"
#include "main.h"
#include <string.h>
#include <stdio.h>

/* ===================================================
   它负责把你传入的字符串，按 MQTT 协议打包成二进制数据
   =================================================== */

// 1. 封装连接报文
int MQTT_Get_Connect_Packet(uint8_t *buff, char *client_id)
{
    int len, id_len;
    int p = 0;

    id_len = strlen(client_id);
    len = 10 + 2 + id_len; // 剩余长度
    
    buff[p++] = 0x10;      // 报文类型 CONNECT
    buff[p++] = len;       // 剩余长度 (这里简化处理，假设长度<127)

    // 协议名长度 + 协议名 "MQTT"
    buff[p++] = 0x00; buff[p++] = 0x04;
    buff[p++] = 'M';  buff[p++] = 'Q';  buff[p++] = 'T';  buff[p++] = 'T';
    
    buff[p++] = 0x04;      // 协议级别 4 (3.1.1)
    buff[p++] = 0x02;      // 连接标志 (Clean Session)
    buff[p++] = 0x00; buff[p++] = 0x3C; // Keep Alive 60秒

    // Client ID
    buff[p++] = (id_len >> 8) & 0xFF;
    buff[p++] = id_len & 0xFF;
    memcpy(&buff[p], client_id, id_len);
    p += id_len;

    return p; // 返回打包好的字节数
}

// 2. 封装发布报文
int MQTT_Get_Publish_Packet(uint8_t *buff, char *topic, char *payload)
{
    int topic_len = strlen(topic);
    int payload_len = strlen(payload);
    int len = 2 + topic_len + payload_len;
    int p = 0;

    buff[p++] = 0x30; // 报文类型 PUBLISH
    
    // 简单处理长度 (如果你的JSON很长，这里可能需要优化，但目前够用)
    if(len > 127) {
        buff[p++] = (len & 0x7F) | 0x80;
        buff[p++] = len >> 7;
    } else {
        buff[p++] = len;
    }

    // Topic
    buff[p++] = (topic_len >> 8) & 0xFF;
    buff[p++] = topic_len & 0xFF;
    memcpy(&buff[p], topic, topic_len);
    p += topic_len;

    // Payload (JSON数据)
    memcpy(&buff[p], payload, payload_len);
    p += payload_len;

    return p;
}

// 3. 封装订阅报文
int MQTT_Get_Subscribe_Packet(uint8_t *buff, char *topic)
{
    int topic_len = strlen(topic);
    int len = 2 + 2 + topic_len + 1;
    int p = 0;

    buff[p++] = 0x82; // 报文类型 SUBSCRIBE
    buff[p++] = len;

    buff[p++] = 0x00; buff[p++] = 0x01; // Message ID (随便填个1)

    // Topic
    buff[p++] = (topic_len >> 8) & 0xFF;
    buff[p++] = topic_len & 0xFF;
    memcpy(&buff[p], topic, topic_len);
    p += topic_len;
    
    buff[p++] = 0x00; // QoS 0

    return p;
}
