/*
 * ESP8266 MQTT OneNet 云平台连接示例
 * 独立双缓冲区解析 + 串口调试信息 + IO2状态指示灯版
 */

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// OneNet MQTT 配置
const char* mqtt_server = "mqtts.heclouds.com";
const uint16_t mqtt_port = 1883;
const char* mqtt_user = "Ltoc218Y7V";
const char* mqtt_client = "d1";
const char* mqtt_pass = "version=2018-10-31&res=products%2FLtoc218Y7V%2Fdevices%2Fd1&et=1924833600&method=md5&sign=OtKZqWWDwvpOLO8L25tLFw%3D%3D";

const char* sub_topic = "$sys/Ltoc218Y7V/d1/thing/property/post/reply";
const char* pub_topic = "$sys/Ltoc218Y7V/d1/thing/property/post";

WiFiClient espClient;
PubSubClient mqtt(espClient);

char ssid[64] = {0};
char pass[64] = {0};

#define LED_PIN 2  // ESP01S 的 IO2 引脚（也是板载蓝灯）

// ---------------- 独立的解析函数 ----------------

// 1. 解析 WIFI 指令 (严格以 \n 结尾)
bool parseWifiCmd(char c) {
  static char buf[160];
  static byte idx = 0;

  if (c == '\n' || idx >= sizeof(buf) - 1) {
    buf[idx] = '\0';
    idx = 0;
    if (strncmp(buf, "WIFI:", 5) == 0) {
      char* p = buf + 5;
      char* comma = strchr(p, ',');
      if (comma) {
        *comma = '\0';
        strncpy(ssid, p, sizeof(ssid) - 1);
        strncpy(pass, comma + 1, sizeof(pass) - 1);
        return true;
      }
    }
  } else if (c != '\r') {
    buf[idx++] = c;
  }
  return false;
}

// 2. 解析传感器 JSON 数据 (严格以 \n 结尾)
void parseSensorCmd(char c) {
  static char buf[300];
  static byte idx = 0;

  if (c == '\n' || idx >= sizeof(buf) - 1) {
    buf[idx] = '\0';
    idx = 0;
    
    // 检查帧头
    if (buf[0] == '{') {
      int adc0, adc1, adc2, co2, sw0, sw1;
      float temp, humi;

      // 提取数据
      int matched = sscanf(buf, 
          "{\"adc\":[%d,%d,%d],\"temp\":%f,\"humi\":%f,\"CO2\":%d,\"sw\":[%d,%d]}", 
          &adc0, &adc1, &adc2, &temp, &humi, &co2, &sw0, &sw1);

      if (matched == 8) {
        char payload[350];
        
        // 拼接 OneNet 格式
        snprintf(payload, sizeof(payload), 
          "{\"id\":\"1\",\"params\":{"
          "\"CO2\":{\"value\":%d},"
          "\"bright\":{\"value\":%d},"
          "\"humi\":{\"value\":%d},"
          "\"soil\":{\"value\":%d},"
          "\"temp\":{\"value\":%d},"
          "\"water\":{\"value\":%d}"
          "}}", 
          co2, adc2, (int)humi, adc1, (int)temp, adc0);
          // Serial.println(payload);

        // 发布
        if (mqtt.publish(pub_topic, payload)) {
          Serial.write(0xAA); // 成功回复 0xAA
        } else {
          // Serial.println("ERR: MQTT Publish Failed");
        }
      } else {
        // Serial.println("ERR: JSON Parse Failed");
      }
    }
  } else if (c != '\r') {
    buf[idx++] = c;
  }
}

// ---------------- MQTT 回调与重连 ----------------

void callback(char* topic, byte* payload, unsigned int len) {
  // 暂不处理下行数据
}

void reconnect() {
  int retry = 0;
  while (!mqtt.connected() && retry < 5) {
    if (mqtt.connect(mqtt_client, mqtt_user, mqtt_pass)) {
      mqtt.subscribe(sub_topic);
      Serial.println("OK: MQTT Connected");
    } else {
      Serial.print("ERR: MQTT Fail, rc=");
      Serial.println(mqtt.state());
      delay(3000);
      retry++;
    }
  }
}

// ---------------- 主流程 ----------------

void setup() {
  Serial.begin(115200);
  
  // 初始化指示灯引脚，默认熄灭
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH); 
  
  mqtt.setServer(mqtt_server, mqtt_port);
  mqtt.setCallback(callback);
  mqtt.setKeepAlive(60);
}

void loop() {
  if (!mqtt.connected()) {
    reconnect();
  }
  mqtt.loop();

  while (Serial.available()) {
    char c = Serial.read();
    
    // 先尝试给 WIFI 解析器
    if (parseWifiCmd(c)) {
      // Serial.print("INFO: Got SSID:"); Serial.println(ssid);
      
      // 需求1：已连接直接回 0xFF，未连接则连接后回 0xFF
      if (WiFi.status() == WL_CONNECTED) {
        Serial.write(0xFF);
        digitalWrite(LED_PIN, LOW); // 点亮指示灯
      } else {
        WiFi.begin(ssid, pass);
        int wifi_timeout = 0;
        while (WiFi.status() != WL_CONNECTED && wifi_timeout < 40) { // 最多等20秒
          delay(500);
          wifi_timeout++;
        }
        
        if (WiFi.status() == WL_CONNECTED) {
          // Serial.println("OK: WiFi Connected");
          Serial.write(0xFF);
          digitalWrite(LED_PIN, LOW); // 连上WiFi后，点亮指示灯
        } else {
          // Serial.println("ERR: WiFi Timeout");
          digitalWrite(LED_PIN, HIGH); // 连接超时，确保熄灭
        }
      }
    }
    
    // 无论上面是否成功，都把字符丢给 JSON 解析器（互不干扰）
    parseSensorCmd(c);
  }
}
