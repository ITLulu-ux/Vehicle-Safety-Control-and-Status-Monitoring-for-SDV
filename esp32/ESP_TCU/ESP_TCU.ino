#include <WiFi.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <ArduinoJson.h>

// ---------------------------------------------------------
// 설정 변수
// ---------------------------------------------------------
const char* ssid = "AndroidHotspot1538";
const char* password = "11223344";
const char* pi_server_url = "http://192.168.0.19:3000/api/vehicle-data"; // 라즈베리파이 수신 주소

// 라즈베리파이로부터 제어 명령을 받기 위한 웹 서버 (포트 80)
WebServer server(80); 

// FreeRTOS 핸들러 선언
SemaphoreHandle_t uartRxSemaphore;
QueueHandle_t rawDataQueue; // ECU4 -> ESP32 데이터 큐
QueueHandle_t jsonTxQueue;  // JSON 변환 완료 큐
QueueHandle_t uartTxQueue;  // ESP32 -> ECU4 제어 명령 큐

// ---------------------------------------------------------
// 인터럽트 함수
// ---------------------------------------------------------
void ARDUINO_ISR_ATTR uartRxISR() {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(uartRxSemaphore, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR();
    }
}

// ---------------------------------------------------------
// 셋업 함수
// ---------------------------------------------------------
void setup() {
    Serial.begin(115200); // 디버깅용 PC 시리얼
    
    // 1. FreeRTOS 객체(세마포어, 큐)를 가장 먼저 생성
    uartRxSemaphore = xSemaphoreCreateBinary();
    rawDataQueue = xQueueCreate(10, 64); // 64바이트 길이의 Raw 문자열 10개
    jsonTxQueue = xQueueCreate(5, 256);  // 256바이트 길이의 JSON 문자열 5개
    uartTxQueue = xQueueCreate(5, 32);   // 32바이트 길이의 하향 제어 문자열 5개

    // 2. Wi-Fi 연결 진행
    Serial.print("\n[SYSTEM] Connecting to Wi-Fi");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\n[SYSTEM] Wi-Fi Connected!");
    Serial.print("[SYSTEM] IP Address: ");
    Serial.println(WiFi.localIP());

    // 3. FreeRTOS Task 생성
    xTaskCreatePinnedToCore(Task_UART_RX, "UART_RX", 4096, NULL, 3, NULL, 1); // High
    xTaskCreatePinnedToCore(Task_HTTP_RX, "HTTP_RX", 4096, NULL, 3, NULL, 1); // High
    xTaskCreatePinnedToCore(Task_JSON,    "JSON_CVT", 4096, NULL, 2, NULL, 1); // Normal
    xTaskCreatePinnedToCore(Task_HTTP_TX, "HTTP_TX", 4096, NULL, 2, NULL, 1); // Normal
    xTaskCreatePinnedToCore(Task_UART_TX, "UART_TX", 4096, NULL, 2, NULL, 1); // Normal

    // 4. 모든 준비가 끝난 후, 맨 마지막에 UART 통신 및 인터럽트 활성화
    Serial2.begin(115200, SERIAL_8N1, 16, 17);
    Serial2.onReceive(uartRxISR); 
    
    Serial.println("[SYSTEM] ESP32 Firmware Started Successfully!\n");
}

void loop() {
    // FreeRTOS를 사용하므로 loop()는 비워둡니다.
    vTaskDelete(NULL);
}

// ---------------------------------------------------------
// FreeRTOS 태스크 구현부
// ---------------------------------------------------------

// [Task 1] UART RX Task: ECU4 데이터 수신
void Task_UART_RX(void *pvParameters) {
    char rxBuffer[64];
    memset(rxBuffer, 0, sizeof(rxBuffer));
    int idx = 0;
    
    for (;;) {
        if (xSemaphoreTake(uartRxSemaphore, portMAX_DELAY) == pdTRUE) {
            while (Serial2.available()) {
                char c = Serial2.read();
                rxBuffer[idx++] = c;
                
                if (c == '\n' || idx >= 63) {
                    rxBuffer[idx] = '\0';
                    
                    // 👉 [디버그 추가] 수신된 Raw 데이터 확인
                    Serial.print("[DEBUG - UART RX] STM32(ECU4)로부터 수신한 Raw Data : ");
                    Serial.println(rxBuffer); 
                    
                    xQueueSend(rawDataQueue, rxBuffer, portMAX_DELAY);
                    memset(rxBuffer, 0, sizeof(rxBuffer));
                    idx = 0;
                }
            }
        }
    }
}

// [Task 2] JSON Task: Raw 문자열 -> JSON 포맷 변환
void Task_JSON(void *pvParameters) {
    char rawString[64];
    char jsonString[256]; 
    
    StaticJsonDocument<256> doc; 

    for (;;) {
        if (xQueueReceive(rawDataQueue, rawString, portMAX_DELAY) == pdTRUE) {
            doc.clear();
            
            int speed, dist, temp, humi, lux, risk, brake, wiper, led, hb1, hb2, hb3;
            
            if (strncmp(rawString, "GW", 2) == 0) {
                int parsed = sscanf(rawString, "GW,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
                                    &speed, &dist, &temp, &humi, &lux, &risk, 
                                    &brake, &wiper, &led, &hb1, &hb2, &hb3);
                
                if (parsed == 12) { 
                    doc["spd"] = speed;
                    doc["dist"] = dist;
                    doc["temp"] = temp;
                    doc["humi"] = humi;
                    doc["lux"] = lux;
                    doc["risk"] = risk;
                    doc["brake"] = brake;
                    doc["wiper"] = wiper;
                    doc["led"] = led;
                    
                    JsonArray hbArray = doc.createNestedArray("hb");
                    hbArray.add(hb1);
                    hbArray.add(hb2);
                    hbArray.add(hb3);

                    serializeJson(doc, jsonString);
                    
                    // 👉 [디버그 추가] 파싱된 JSON 확인
                    Serial.print("[DEBUG - JSON] 파싱 성공 -> 큐에 전달 : ");
                    Serial.println(jsonString);
                    
                    xQueueSend(jsonTxQueue, jsonString, portMAX_DELAY);
                } else {
                    // 👉 [디버그 추가] 파싱 실패 시 원인 파악
                    Serial.println("[DEBUG - JSON ERROR] STM32 데이터 파싱 실패! 형식 불일치");
                }
            }
        }
    }
}

// [Task 3] HTTP TX Task: JSON 데이터를 라즈베리파이로 송신
void Task_HTTP_TX(void *pvParameters) {
    char jsonString[256];
    HTTPClient http;
    
    for (;;) {
        if (xQueueReceive(jsonTxQueue, jsonString, portMAX_DELAY) == pdTRUE) {
            if (WiFi.status() == WL_CONNECTED) {
                http.begin(pi_server_url);
                http.setTimeout(2000); 
                http.addHeader("Content-Type", "application/json");
                
                // 👉 [디버그 추가] 전송 시도 로깅
                Serial.println("[DEBUG - HTTP TX] 라즈베리파이로 전송 시도 중...");
                int httpResponseCode = http.POST(jsonString);
                
                if (httpResponseCode > 0) {
                    // 👉 [디버그 추가] 성공 시 응답 코드 및 보낸 데이터 확인
                    Serial.printf("[DEBUG - HTTP TX] 전송 성공! (HTTP %d)\n", httpResponseCode);
                    Serial.print("  └─ 전송한 데이터 : ");
                    Serial.println(jsonString); 
                } else {
                    // 👉 [디버그 추가] 실패 시 에러 코드 확인
                    Serial.printf("[DEBUG - HTTP TX ERROR] 전송 실패: %s\n", http.errorToString(httpResponseCode).c_str());
                }
                http.end();
            } else {
                Serial.println("[DEBUG - HTTP TX ERROR] Wi-Fi 연결 끊김!");
            }
        }
    }
}

// [Task 4] HTTP RX Task: 라즈베리파이의 제어/OTA 명령 수신
void Task_HTTP_RX(void *pvParameters) {
    server.on("/api/control", HTTP_POST, []() {
        if (server.hasArg("plain")) {
            String body = server.arg("plain");
            
            // 👉 [디버그 추가] 라즈베리파이에서 들어온 명령(JSON) 확인
            Serial.print("[DEBUG - HTTP RX] 라즈베리파이로부터 제어 명령 수신 : ");
            Serial.println(body);
            
            StaticJsonDocument<200> doc;
            doc.clear(); 
            deserializeJson(doc, body);
            
            int cmdId = doc["cmd"];
            int targetEcu = doc["target"]; // 필요시 패킷에 조합
            int param1 = doc["param1"];
            int param2 = doc["param2"];
            
            char txString[32];
            sprintf(txString, "CMD,%d,%d,%d,%d\n", cmdId, targetEcu, param1, param2);
            
            xQueueSend(uartTxQueue, txString, portMAX_DELAY);
            
            server.send(200, "application/json", "{\"status\":\"success\"}");
        } else {
            server.send(400, "text/plain", "Bad Request");
        }
    });

    server.begin();
    
    for (;;) {
        server.handleClient();
        vTaskDelay(10 / portTICK_PERIOD_MS); 
    }
}

// [Task 5] UART TX Task: 파싱된 제어 명령을 ECU4로 하달
void Task_UART_TX(void *pvParameters) {
    char txString[32];
    
    for (;;) {
        if (xQueueReceive(uartTxQueue, txString, portMAX_DELAY) == pdTRUE) {
            Serial2.print(txString); 
            
            // 👉 [디버그 추가] 하달하는 커맨드 확인
            Serial.print("[DEBUG - UART TX] STM32(ECU4)로 명령 하달 : ");
            Serial.print(txString); // 이미 \n이 포함되어 있으므로 print 사용
        }
    }
}