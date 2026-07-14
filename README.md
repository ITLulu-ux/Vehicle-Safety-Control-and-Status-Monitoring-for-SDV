# Vehicle-Safety-Control-and-Status-Monitoring-for-SDV
Software Defined Vehicle(SDV)를 위한 차량 상태 모니터링 및 제어 시스템
STM32 ECU와 Raspberry Pi Gateway를 기반으로 CAN 통신, UDS 진단 서비스, OTA 업데이트, 실시간 Dashboard를 구현한 팀 프로젝트입니다.


## 프로젝트 개요
* CAN 기반 ECU 간 통신
* Raspberry Pi Gateway
* UDS 진단 서비스
* OTA Firmware Update
* 실시간 Dashboard
* 차량 상태 모니터링
* MariaDB 기반 로그 저장

  
# 시스템 아키텍처
<img width="1027" height="782" alt="image" src="https://github.com/user-attachments/assets/ee717540-e5ef-46f4-aac2-804aa4e6a0f6" />




# 주요 기능
## Dashboard
* 차량 상태 실시간 표시
* 센서 데이터 모니터링
* Heartbeat 상태 표시

## UDS
* ReadDataByIdentifier
* ReadDTCInformation
* ClearDiagnosticInformation

## OTA
* Raspberry Pi 기반 OTA 파일 전송
* ECU1 대상 Binary File Transfer 검증
* Bootloader 연동을 고려한 OTA 구조 설계

## Database
* Heartbeat Log
* DTC Log
* Sensor Log

## 기술 스택
분야	           기술
MCU	          STM32F446RE
RTOS	         FreeRTOS
Protocol	     CAN, UDS
Gateway	     Raspberry Pi 5
Backend	        Node.js
Database	       MariaDB
Frontend	  HTML, CSS, JavaScript

# 담당 역할
* ECU3 기능 개발
* Raspberry Pi Node.js 서버 개발
* Dashboard 연동
* MariaDB 연동
* 실시간 데이터 처리
* Heartbeat 통신 오류 디버깅 및 ECU4 상태 정상화
* OTA 파일 전송 기능 구현(ECU1 대상, Bootloader 제외)

