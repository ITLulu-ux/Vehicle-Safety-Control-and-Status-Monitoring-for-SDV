#include <WiFi.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <ArduinoJson.h>

// ---------------------------------------------------------
// 1. 설정 변수 (사용자 환경에 맞게 수정)
// ---------------------------------------------------------
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* pi_server_url = "http://192.168.0.100:3000/api/telemetry"; // 라즈베리파이 수신 주소

// 라즈베리파이로부터 제어 명령을 받기 위한 웹 서버 (포트 80)
WebServer server(80); 

// ---------------------------------------------------------
// 2. FreeRTOS 핸들러 선언
// ---------------------------------------------------------
SemaphoreHandle_t uartRxSemaphore;
QueueHandle_t rawDataQueue; // ECU4 -> ESP32 데이터 큐
QueueHandle_t jsonTxQueue;  // JSON 변환 완료 큐
QueueHandle_t uartTxQueue;  // ESP32 -> ECU4 제어 명령 큐

// ---------------------------------------------------------
// 3. 인터럽트 및 셋업 함수
// ---------------------------------------------------------
// UART 수신 인터럽트 (ECU4에서 데이터가 들어오면 세마포어 발생)
void ARDUINO_ISR_ATTR uartRxISR() {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(uartRxSemaphore, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR();
    }
}

void setup() {
    Serial.begin(115200); // 디버깅용 PC 시리얼
    
    // ECU4와의 UART 통신 (Serial2 사용, 핀 16: RX, 17: TX로 가정)
    Serial2.begin(115200, SERIAL_8N1, 16, 17);
    Serial2.onReceive(uartRxISR); // 수신 인터럽트 활성화 [6]

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

    // FreeRTOS Task 생성 (명세서 우선순위 반영) [4]
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
// 4. FreeRTOS 태스크 구현부
// ---------------------------------------------------------

// [Task 1] UART RX Task: ECU4 데이터 수신 [4, 6]
void Task_UART_RX(void *pvParameters) {
    char rxBuffer[7];
    int idx = 0;
    
    for (;;) {
        if (xSemaphoreTake(uartRxSemaphore, portMAX_DELAY) == pdTRUE) {
            while (Serial2.available()) {
                char c = Serial2.read();
                rxBuffer[idx++] = c;
                
                // 개행문자('\n')를 만나면 큐로 전달
                if (c == '\n' || idx >= 63) {
                    rxBuffer[idx] = '\0';
                    xQueueSend(rawDataQueue, &rxBuffer, portMAX_DELAY);
                    idx = 0;
                }
            }
        }
    }
}

// [Task 2] JSON Task: Raw 문자열 -> JSON 포맷 변환 (S2S) [5, 8, 9]
void Task_JSON(void *pvParameters) {
    char rawString[7];
    char jsonString;
    
    // 메모리 풀 최적화를 위해 루프 밖에서 StaticJsonDocument 생성
    StaticJsonDocument<256> doc; 

    for (;;) {
        if (xQueueReceive(rawDataQueue, &rawString, portMAX_DELAY) == pdTRUE) {
            doc.clear(); // 메모리 누수 방지 (필수) [5]
            
            int id = 0;
            // 예시: "100,85,350\n" 포맷 파싱
            if (sscanf(rawString, "%d,", &id) == 1) {
                if (id == 100) { // 주행 데이터
                    int speed, distance;
                    sscanf(rawString, "100,%d,%d", &speed, &distance);
                    doc["id"] = 100;
                    doc["spd"] = speed;
                    doc["dist"] = distance;
                } 
                else if (id == 200) { // 환경 데이터
                    int temp, humi, lux;
                    sscanf(rawString, "200,%d,%d,%d", &temp, &humi, &lux);
                    doc["id"] = 200;
                    doc["temp"] = temp;
                    doc["humi"] = humi;
                    doc["lux"] = lux;
                }
                
                // 파싱된 JSON을 문자열로 직렬화하여 송신 큐로 전달
                serializeJson(doc, jsonString);
                xQueueSend(jsonTxQueue, &jsonString, portMAX_DELAY);
            }
        }
    }
}

// [Task 3] HTTP TX Task: JSON 데이터를 라즈베리파이로 송신 [4, 10]
void Task_HTTP_TX(void *pvParameters) {
    char jsonString;
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

// [Task 4] HTTP RX Task: 라즈베리파이의 제어/OTA 명령 수신 [4, 10]
void Task_HTTP_RX(void *pvParameters) {
    // 라즈베리파이가 이 주소(/api/control)로 POST 요청을 보냄
    server.on("/api/control", HTTP_POST, []() {
        if (server.hasArg("plain")) {
            String body = server.arg("plain");
            StaticJsonDocument<200> doc;
            deserializeJson(doc, body);
            
            // 수신된 JSON 형태: {"cmd": 2, "target": 1, "param1": 0, "param2": 0} (예: OTA Start)
            int cmdId = doc["cmd"];
            int targetEcu = doc["target"];
            int param1 = doc["param1"];
            int param2 = doc["param2"];
            
            // ECU4(Gateway)로 보낼 Raw 문자열 포맷팅 (0x400 처리용)
            char txString[11];
            sprintf(txString, "%d,%d,%d,%d\n", cmdId, targetEcu, param1, param2);
            
            // UART 송신 큐로 전달
            xQueueSend(uartTxQueue, &txString, portMAX_DELAY);
            
            server.send(200, "application/json", "{\"status\":\"success\"}");
        } else {
            server.send(400, "text/plain", "Bad Request");
        }
    });

    server.begin();
    
    for (;;) {
        // 웹 서버 클라이언트 대기 및 처리
        server.handleClient();
        vTaskDelay(10 / portTICK_PERIOD_MS); // CPU 양보를 위한 Delay
    }
}

// [Task 5] UART TX Task: 파싱된 제어 명령을 ECU4로 하달 [4, 12, 13]
void Task_UART_TX(void *pvParameters) {
    char txString[11];
    
    for (;;) {
        // 큐에 명령이 들어오면 ECU4로 UART 송신
        if (xQueueReceive(uartTxQueue, &txString, portMAX_DELAY) == pdTRUE) {
            Serial2.print(txString); 
            Serial.print("Downlink to ECU4: ");
            Serial.print(txString);
        }
    }
}