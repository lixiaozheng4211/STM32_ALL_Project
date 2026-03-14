#include "ESP8266.h"
#include "main.h"
#include <string.h>
#include <stdio.h>

/*=========================
 * 内部：收集串口数据到 rx_buf，直到出现期望字符串或超时
 * 返回 ESP_OK / ESP_TIMEOUT
 * 如果 err_str 不为 NULL 且响应中包含它，返回 ESP_ERR
 *=========================*/
static ESP_Status esp_wait_for(ESP8266_t *esp,
                               const char *expect,
                               const char *err_str,
                               uint32_t timeout_ms) {
  uint32_t start = HAL_GetTick();
  uint8_t ch;

  esp->rx_len = 0;
  memset(esp->rx_buf, 0, ESP_RX_BUF_SIZE);

  while ((HAL_GetTick() - start) < timeout_ms) {
    while (UART_IO_ReadByte(esp->uart, &ch)) {
      if (esp->rx_len < ESP_RX_BUF_SIZE - 1) {
        esp->rx_buf[esp->rx_len++] = (char) ch;
        esp->rx_buf[esp->rx_len] = '\0';
      }
    }

    if (strstr(esp->rx_buf, expect)) {
      return ESP_OK;
    }
    if (err_str && strstr(esp->rx_buf, err_str)) {
      return ESP_ERR;
    }

    HAL_Delay(1);
  }

  return ESP_TIMEOUT;
}

/*=========================
 * 内部：发送 AT 指令（自动加 \r\n）
 *=========================*/
static void esp_send_cmd(ESP8266_t *esp, const char *cmd) {
  UART_IO_Send(esp->uart, (const uint8_t *) cmd, (uint16_t) strlen(cmd));
  UART_IO_Send(esp->uart, (const uint8_t *) "\r\n", 2);
}

/*=========================
 * 公开接口实现
 *=========================*/

void ESP8266_Init(ESP8266_t *esp, UART_IO_t *uart,
                  void (*on_data)(const char *data, uint16_t len)) {
  memset(esp, 0, sizeof(ESP8266_t));
  esp->uart = uart;
  esp->on_data = on_data;
}

ESP_Status ESP8266_Reset(ESP8266_t *esp) {
  esp_send_cmd(esp, "AT+RST");
  /* 等待模块输出 ready */
  if (esp_wait_for(esp, "ready", NULL, ESP_CONN_TIMEOUT_MS) != ESP_OK) {
    return ESP_TIMEOUT;
  }
  HAL_Delay(500);

  /* 关闭回显 */
  esp_send_cmd(esp, "ATE0");
  esp_wait_for(esp, "OK", NULL, ESP_CMD_TIMEOUT_MS);

  /* Station 模式 */
  esp_send_cmd(esp, "AT+CWMODE=1");
  return esp_wait_for(esp, "OK", "ERROR", ESP_CMD_TIMEOUT_MS);
}

ESP_Status ESP8266_ConnectWiFi(ESP8266_t *esp, const char *ssid, const char *pwd) {
  char cmd[128];
  snprintf(cmd, sizeof(cmd), "AT+CWJAP=\"%s\",\"%s\"", ssid, pwd);
  esp_send_cmd(esp, cmd);
  /* CWJAP 成功返回 WIFI CONNECTED + WIFI GOT IP + OK */
  return esp_wait_for(esp, "GOT IP", "FAIL", ESP_CONN_TIMEOUT_MS);
}

ESP_Status ESP8266_ConnectTCP(ESP8266_t *esp, const char *host, uint16_t port) {
  char cmd[128];
  snprintf(cmd, sizeof(cmd), "AT+CIPSTART=\"TCP\",\"%s\",%u", host, port);
  esp_send_cmd(esp, cmd);
  return esp_wait_for(esp, "CONNECT", "ERROR", ESP_CONN_TIMEOUT_MS);
}

ESP_Status ESP8266_Disconnect(ESP8266_t *esp) {
  esp_send_cmd(esp, "AT+CIPCLOSE");
  return esp_wait_for(esp, "OK", "ERROR", ESP_CMD_TIMEOUT_MS);
}

ESP_Status ESP8266_Send(ESP8266_t *esp, const char *data, uint16_t len) {
  char cmd[32];
  snprintf(cmd, sizeof(cmd), "AT+CIPSEND=%u", len);
  esp_send_cmd(esp, cmd);

  /* 等待 > 提示符，表示可以发数据了 */
  if (esp_wait_for(esp, ">", "ERROR", ESP_CMD_TIMEOUT_MS) != ESP_OK) {
    return ESP_ERR;
  }

  /* 发送实际数据（不加 \r\n） */
  UART_IO_Send(esp->uart, (const uint8_t *) data, len);

  /* 等待 SEND OK */
  return esp_wait_for(esp, "SEND OK", "SEND FAIL", ESP_CMD_TIMEOUT_MS);
}

/*=========================
 * 主循环里调用
 * 把串口收到的数据攒起来，检测 +IPD 并触发回调
 *
 * +IPD 格式：+IPD,<len>:<data>
 *=========================*/
/*=========================
 * NTP 获取时间
 *
 * 流程：
 * 1. AT+CIPSNTPCFG 配置时区和 NTP 服务器
 * 2. 等待同步完成（最多 10s）
 * 3. AT+CIPSNTPTIME? 查询时间字符串
 * 4. 解析返回值
 *
 * 返回格式：+CIPSNTPTIME:Thu Jan  1 08:00:10 2026
 *=========================*/
ESP_Status ESP8266_GetTime(ESP8266_t *esp, int8_t timezone, ESP_Time_t *out) {
  char cmd[64];
  char *p;

  /* 配置 SNTP：时区 + 三个备用服务器 */
  snprintf(cmd, sizeof(cmd),
           "AT+CIPSNTPCFG=1,%d,\"pool.ntp.org\",\"time.nist.gov\"",
           timezone);
  esp_send_cmd(esp, cmd);
  if (esp_wait_for(esp, "OK", "ERROR", ESP_CMD_TIMEOUT_MS) != ESP_OK) {
    return ESP_ERR;
  }

  /* 等待 NTP 同步，最多 10 秒 */
  uint32_t start = HAL_GetTick();
  while ((HAL_GetTick() - start) < 10000) {
    esp_send_cmd(esp, "AT+CIPSNTPTIME?");
    if (esp_wait_for(esp, "+CIPSNTPTIME:", "ERROR", ESP_CMD_TIMEOUT_MS) == ESP_OK) {
      /* 检查是否已同步（未同步时返回 Jan  1 1970） */
      if (strstr(esp->rx_buf, "1970") == NULL) {
        break;
      }
    }
    HAL_Delay(1000);
  }

  /* 再查一次，拿到最终结果 */
  esp_send_cmd(esp, "AT+CIPSNTPTIME?");
  if (esp_wait_for(esp, "+CIPSNTPTIME:", "ERROR", ESP_CMD_TIMEOUT_MS) != ESP_OK) {
    return ESP_TIMEOUT;
  }

  /*
   * 解析格式：+CIPSNTPTIME:Thu Jan  1 08:00:10 2026\r\n
   * 字段位置：  跳过前缀    wday mon  day hh:mm:ss year
   */
  p = strstr(esp->rx_buf, "+CIPSNTPTIME:");
  if (p == NULL) return ESP_ERR;
  p += strlen("+CIPSNTPTIME:");

  /* 月份名称表 */
  const char *months[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
  };

  char wday[4], mon[4];
  int day, hh, mm, ss, year;

  /* 格式：Www Mmm DD HH:MM:SS YYYY */
  if (sscanf(p, "%3s %3s %d %d:%d:%d %d",
             wday, mon, &day, &hh, &mm, &ss, &year) != 7) {
    return ESP_ERR;
  }

  out->year = (uint16_t) year;
  out->day = (uint8_t) day;
  out->hour = (uint8_t) hh;
  out->minute = (uint8_t) mm;
  out->second = (uint8_t) ss;

  out->month = 0;
  for (uint8_t i = 0; i < 12; i++) {
    if (strncmp(mon, months[i], 3) == 0) {
      out->month = (uint8_t) (i + 1);
      break;
    }
  }

  return (out->month > 0) ? ESP_OK : ESP_ERR;
}

/*=========================
 * 主循环里调用
 * 把串口收到的数据攒起来，检测 +IPD 并触发回调
 *=========================*/
void ESP8266_Process(ESP8266_t *esp) {
  uint8_t ch;

  while (UART_IO_ReadByte(esp->uart, &ch)) {
    if (esp->rx_len < ESP_RX_BUF_SIZE - 1) {
      esp->rx_buf[esp->rx_len++] = (char) ch;
      esp->rx_buf[esp->rx_len] = '\0';
    } else {
      /* 缓冲区满，丢弃旧数据 */
      esp->rx_len = 0;
    }
  }

  /* 检测 +IPD */
  char *ipd = strstr(esp->rx_buf, "+IPD,");
  if (ipd == NULL) return;

  /* 解析长度 */
  uint16_t data_len = 0;
  char *colon = strchr(ipd, ':');
  if (colon == NULL) return; /* 数据还没收完 */

  data_len = (uint16_t)atoi(ipd + 5); /* +IPD, 后面是长度 */

  /* 确认数据已经全部收到 */
  char *data_start = colon + 1;
  uint16_t received = (uint16_t) (esp->rx_buf + esp->rx_len - data_start);
  if (received < data_len) return; /* 还没收完，等下次 */

  /* 触发回调 */
  if (esp->on_data) {
    esp->on_data(data_start, data_len);
  }

  /* 清掉已处理的数据，保留后面多余的 */
  uint16_t consumed = (uint16_t) (data_start + data_len - esp->rx_buf);
  uint16_t remaining = esp->rx_len - consumed;
  if (remaining > 0) {
    memmove(esp->rx_buf, esp->rx_buf + consumed, remaining);
  }
  esp->rx_len = remaining;
  esp->rx_buf[esp->rx_len] = '\0';
}
