#ifndef ESP8266_H
#define ESP8266_H

#include "UART_DMA.h"
#include <stdint.h>

/*=========================
 * 可调参数
 *=========================*/
#define ESP_CMD_TIMEOUT_MS   5000   /* 普通指令超时 */
#define ESP_CONN_TIMEOUT_MS  10000  /* 连接类指令超时 */
#define ESP_RX_BUF_SIZE      512    /* 响应缓冲区大小 */

/*=========================
 * 返回值
 *=========================*/
typedef enum {
    ESP_OK      = 0,
    ESP_ERR     = 1,   /* 指令失败 / ERROR */
    ESP_TIMEOUT = 2,   /* 超时 */
} ESP_Status;

/*=========================
 * 时间结构体
 *=========================*/
typedef struct {
    uint16_t year;
    uint8_t  month;
    uint8_t  day;
    uint8_t  hour;
    uint8_t  minute;
    uint8_t  second;
} ESP_Time_t;
typedef struct {
    UART_IO_t *uart;                  /* 指向 uart_wifi 实例 */
    char       rx_buf[ESP_RX_BUF_SIZE]; /* 接收缓冲区 */
    uint16_t   rx_len;

    /* 收到 +IPD 数据时的回调，可为 NULL */
    void (*on_data)(const char *data, uint16_t len);
} ESP8266_t;

/*=========================
 * 公开接口
 *=========================*/

/* 初始化，传入 uart 实例和数据回调（不需要可传 NULL） */
void       ESP8266_Init(ESP8266_t *esp, UART_IO_t *uart,
                        void (*on_data)(const char *data, uint16_t len));

/* 复位模块，等待 ready */
ESP_Status ESP8266_Reset(ESP8266_t *esp);

/* 连接 WiFi，ssid/pwd 为字符串 */
ESP_Status ESP8266_ConnectWiFi(ESP8266_t *esp, const char *ssid, const char *pwd);

/* 建立 TCP 连接 */
ESP_Status ESP8266_ConnectTCP(ESP8266_t *esp, const char *host, uint16_t port);

/* 断开连接 */
ESP_Status ESP8266_Disconnect(ESP8266_t *esp);

/* 发送数据 */
ESP_Status ESP8266_Send(ESP8266_t *esp, const char *data, uint16_t len);

/* 主循环里调用，处理接收到的数据（触发 on_data 回调） */
void       ESP8266_Process(ESP8266_t *esp);

/* 配置 SNTP 并获取当前时间，timezone: 时区偏移小时数（东八区填8） */
ESP_Status ESP8266_GetTime(ESP8266_t *esp, int8_t timezone, ESP_Time_t *out);

#endif /* ESP8266_H */
