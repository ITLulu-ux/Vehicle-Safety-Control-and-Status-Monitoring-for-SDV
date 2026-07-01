#include <WiFi.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <ArduinoJson.h>

// ---------------------------------------------------------
// 설정 변수
// ---------------------------------------------------------
const char* ssid = "AndriodHotspot1538";
const char* password = "11223344";
const char* pi_server_url = "http://192.168.0.100:3000/api/telemetry"; // 라즈베리파이 수신 주소

// 라즈베리파이로부터 제어 명령을 받기 위한 웹 서버 (포트 80)
WebServer server(80); 

// FreeRTOS 핸들러 선언
SemaphoreHandle_t uartRxSemaphore;
QueueHandle_t rawDataQueue; // ECU4 -> ESP32 데이터 큐
QueueHandle_t jsonTxQueue;  // JSON 변환 완료 큐
QueueHandle_t uartTxQueue;  // ESP32 -> ECU4 제어 명령 큐

// ---------------------------------------------------------
// 인터럽트 및 셋업 함수
// ---------------------------------------------------------
void ARDUINO_ISR_ATTR uartRxISR() {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(uartRxSemaphore, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR();
    }
}

void setup() {
    Serial.begin(115200); // 디버깅용 PC 시리얼
    
    // ECU4와의 UART 통신 (Serial2 사용, 핀 16: RX, 17: TX)
    Serial2.begin(115200, SERIAL_8N1, 16, 17);
    Serial2.onReceive(uartRxISR); // 수신 인터럽트 활성화

    // Wi-Fi 연결
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWi-Fi Connected!");

    // FreeRTOS 객체 생성
    uartRxSemaphore = xSemaphoreCreateBinary();
    rawDataQueue = xQueueCreate(10, 64); // 64바이트 길이의 Raw 문자열 10개
    jsonTxQueue = xQueueCreate(5, 256);  // 256바이트 길이의 JSON 문자열 5개
    uartTxQueue = xQueueCreate(5, 32);   // 32바이트 길이의 하향 제어 문자열 5개

    // FreeRTOS Task 생성
    xTaskCreatePinnedToCore(Task_UART_RX, "UART_RX", 4096, NULL, 3, NULL, 1); // High
    xTaskCreatePinnedToCore(Task_HTTP_RX, "HTTP_RX", 4096, NULL, 3, NULL, 1); // High
    xTaskCreatePinnedToCore(Task_JSON,    "JSON_CVT", 4096, NULL, 2, NULL, 1); // Normal
    xTaskCreatePinnedToCore(Task_HTTP_TX, "HTTP_TX", 4096, NULL, 2, NULL, 1); // Normal
    xTaskCreatePinnedToCore(Task_UART_TX, "UART_TX", 4096, NULL, 2, NULL, 1); // Normal
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
    char rxBuffer[64]; // [수정] 큐 사이즈에 맞게 버퍼 크기 증가
    int idx = 0;
    
    for (;;) {
        if (xSemaphoreTake(uartRxSemaphore, portMAX_DELAY) == pdTRUE) {
            while (Serial2.available()) {
                char c = Serial2.read();
                rxBuffer[idx++] = c;
                
                // 개행문자('\n')를 만나거나 버퍼 한계 도달 시 큐로 전달
                if (c == '\n' || idx >= 63) {
                    rxBuffer[idx] = '\0'; // 문자열 종단 처리
                    xQueueSend(rawDataQueue, &rxBuffer, portMAX_DELAY);
                    idx = 0;
                }
            }
        }
    }
}

// [Task 2] JSON Task: Raw 문자열 -> JSON 포맷 변환 (ECU4 GW 포맷 호환)
void Task_JSON(void *pvParameters) {
    char rawString[64]; // [수정] 큐에서 받을 버퍼
    char jsonString[256]; // [수정] 직렬화된 JSON을 담을 문자열 버퍼
    
    StaticJsonDocument<256> doc; 

    for (;;) {
        if (xQueueReceive(rawDataQueue, &rawString, portMAX_DELAY) == pdTRUE) {
            doc.clear(); // 메모리 초기화
            
            int speed, dist, temp, humi, lux, risk, brake, wiper, led, hb1, hb2, hb3;
            
            // [수정] ECU4의 "GW,..." 프로토콜에 맞게 파싱
            if (strncmp(rawString, "GW", 2) == 0) {
                int parsed = sscanf(rawString, "GW,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
                                    &speed, &dist, &temp, &humi, &lux, &risk, 
                                    &brake, &wiper, &led, &hb1, &hb2, &hb3);
                
                if (parsed == 12) { // 12개의 데이터가 정상적으로 파싱되었는지 확인
                    doc["spd"] = speed;
                    doc["dist"] = dist;
                    doc["temp"] = temp;
                    doc["humi"] = humi;
                    doc["lux"] = lux;
                    doc["risk"] = risk;
                    doc["brake"] = brake;
                    doc["wiper"] = wiper;
                    doc["led"] = led;
                    
                    // 하트비트 상태를 배열로 전송
                    JsonArray hbArray = doc.createNestedArray("hb");
                    hbArray.add(hb1);
                    hbArray.add(hb2);
                    hbArray.add(hb3);

                    // 파싱된 JSON을 문자열로 직렬화하여 송신 큐로 전달
                    serializeJson(doc, jsonString);
                    xQueueSend(jsonTxQueue, &jsonString, portMAX_DELAY);
                }
            }
        }
    }
}

// [Task 3] HTTP TX Task: JSON 데이터를 라즈베리파이로 송신
void Task_HTTP_TX(void *pvParameters) {
    char jsonString[256]; // [수정] 큐에서 받을 버퍼
    HTTPClient http;
    
    for (;;) {
        if (xQueueReceive(jsonTxQueue, &jsonString, portMAX_DELAY) == pdTRUE) {
            if (WiFi.status() == WL_CONNECTED) {
                http.begin(pi_server_url);
                http.addHeader("Content-Type", "application/json");
                
                int httpResponseCode = http.POST(jsonString);
                
                if (httpResponseCode > 0) {
                    Serial.printf("HTTP POST Success: %d\n", httpResponseCode);
                } else {
                    Serial.printf("HTTP POST Failed: %s\n", http.errorToString(httpResponseCode).c_str());
                }
                http.end();
            }
        }
    }
}

// [Task 4] HTTP RX Task: 라즈베리파이의 제어/OTA 명령 수신
void Task_HTTP_RX(void *pvParameters) {
    server.on("/api/control", HTTP_POST, []() {
        if (server.hasArg("plain")) {
            String body = server.arg("plain");
            StaticJsonDocument<200> doc;
            deserializeJson(doc, body);
            
            int cmdId = doc["cmd"];
            int targetEcu = doc["target"];
            int param1 = doc["param1"];
            int param2 = doc["param2"];
            
            char txString[32]; // [수정] 넉넉한 버퍼 크기
            // ECU4와 약속한 커맨드 포맷 "CMD,ID,param1,param2"
            sprintf(txString, "CMD,%d,%d,%d\n", cmdId, param1, param2);
            
            xQueueSend(uartTxQueue, &txString, portMAX_DELAY);
            
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
    char txString[32]; // [수정] 큐에서 받을 버퍼
    
    for (;;) {
        if (xQueueReceive(uartTxQueue, &txString, portMAX_DELAY) == pdTRUE) {
            Serial2.print(txString); 
            Serial.print("Downlink to ECU4: ");
            Serial.print(txString);
        }
    }
}