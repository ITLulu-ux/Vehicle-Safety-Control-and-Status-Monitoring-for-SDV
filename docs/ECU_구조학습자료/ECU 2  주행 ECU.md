ECU 2 : 주행 ECU





역할 : 

\-	가변저항(속도측정)

\-	HC-SR04 초음파센서로 전방 장애물과 거리측정(라이브러리 없어서 직접 드라이브 만들어야함)

\-	LCD를 통해 현재 차량상태 운전자에게 제공

\-	주행 데이터 CAN 송신

\-	GateWay 명령 수신

\-	hearbeat 송신





======================================================================================================================



사용 Peripheral



| Peripheral              | 사용 대상           | 프로젝트에서의 역할                                                                  |

| ----------------------- | ---------------     | --------------------------------------------------------------------                 |

| \*\*GPIO\*\*                | HC-SR04 Trigger     | HC-SR04에 10μs Trigger 신호를 출력하여 초음파 측정을 시작한다.                       |

| \*\*TIM (Input Capture)\*\* | HC-SR04 Echo        | Echo 핀의 HIGH 유지 시간을 측정하여 전방 장애물과의 거리를 계산한다.                 |

| \*\*ADC1\*\*                | 가변저항            | 가변저항의 아날로그 전압을 읽어 차량 속도를 시뮬레이션한다.                          |

| \*\*I2C\*\*                 | I2C LCD             | 현재 속도, 전방 장애물 거리, 시스템 상태를 LCD에 실시간으로 표시한다.                |

| \*\*CAN1\*\*                | CAN Bus             | 측정한 속도 및 거리 데이터를 CAN 메시지로 ECU3와 ECU4에 전송한다.                    |

| \*\*FreeRTOS\*\*            | STM32 내부          | Speed Task, Distance Task, LCD Task, CAN TX Task를 스케줄링하여 실시간으로 실행한다. |



======================================================================================================================



Task 우선순위



| Task           | Priority | 역할             												 |

| -------------- | -------- | --------------- 												 |

| CAN RX Task    | High     | CAN제어 명령을 즉시 수신하고 명력을 해석하여 필요한 Flag를 설정						 |

|----------------------------------------------------------------------------------------------------------------------------------------|

| Distance Task  | 운선순위 | HC-SR04를 이용하여 전방 장애물 거리 측정 후 DrivingData 구조체 갱신					 |

| Speed Task 	 | 같음	    | ADC를 이용하여 가변저항 값을 읽고 차량속도를 계산하여 DrivingData 구조체 갱신				 |

|----------------------------------------------------------------------------------------------------------------------------------------|

| CAN TX Task    | Normal   | 공유 구조체를 읽어 Speed,Distance 데이터를 CAN메시지로 송신				      	 |

| Heartbeat Task | Low      | ECU의 정상 동작 여부를 주기적으로 알림 									 |

| LCD Task	 | Low      | 화면 표시가 약간 늦어져도 시스템 제어에는 영향이 적음							 |



\# 왜 우선순위가 같을까?



Speed Task와 Distance Task는 각각 차량 속도와 전방 장애물 거리를 측정하는 Task로, 두 데이터 모두 ECU3의 위험도 판단에 핵심적으로 사용된다. 

특정 Task를 우선하기보다 동일한 우선순위를 부여하여 FreeRTOS의 Round Robin 스케줄링을 통해 두 센서의 데이터를 균형 있게 갱신하도록 설계하였다.



\-----------------------------------------

예를 들어 ECU2에서



Speed Task      Priority = 5

Distance Task   Priority = 5



이렇게만 설정하면 자동으로 번갈아 실행됨. 단 Task가 CPU를 계속 붙잡고 있으면 안됨

========

void SpeedTask(void \*arg)

{

&#x20;   for(;;)

&#x20;   {

&#x20;       Read\_ADC();

&#x20;       osDelay(20);

&#x20;   }

}

처럼 CPU를 양보해야 함.

========



======================================================================================================================



인터럽트



| Interrupt                            | 발생 조건                                  | 역할                                                                    			       |

| ------------------------------------ | ------------------------------             | ---------------------------------------------------------------------- 		               |

| \*\*TIM Input Capture (HC-SR04 Echo)\*\* | HC-SR04 Echo 신호의 상승/하강 에지 발생 시 | Echo 펄스의 HIGH 유지 시간을 측정하여 전방 장애물과의 거리를 계산한다.                           |

| \*\*CAN RX Interrupt\*\*                 | CAN 메시지 수신 시                  	    | CAN 수신 이벤트를 감지하고 CAN RX Task를 실행할 수 있도록 Semaphore를 Give하여 제어 명령을 처리. |

| \*\*SysTick Interrupt\*\*                | 1ms 주기                                   | FreeRTOS Tick을 생성하여 Task 스케줄링, `osDelay()`, 소프트웨어 타이머 등을 관리한다.            |



\# 자세한건 LLM 으로 학습할 것!



======================================================================================================================



\*\*\* ECU1 에서는 Queue를 사용했지만 ECU2 에서는 Mutex 사용 \*\*\*

Mutex = 공유자원(DrivingData 구조체) 에대한 동시 접근을 방지하여 데이터의 일관성을 보장하는 동기화 기법



ECU2는 속도와 거리 데이터를 각각 독립적인 Task에서 생성하지만, 두 데이터를 동시에 사용하는 LCD 출력과 CAN 송신 기능이 필요하다. 

Queue는 Task 간 데이터 전달에는 적합하지만, 여러 Task가 동일한 최신 데이터를 공유하는 구조에는 적합하지 않다. 따라서 하나의 

공유 구조체(Shared DrivingData)를 사용하고, 여러 Task가 안전하게 접근할 수 있도록 Mutex를 적용하였다.



=================구조체=================

typedef struct

{

&#x20;   uint8\_t speed;

&#x20;   uint16\_t distance;

} DrivingData\_t;



DrivingData\_t drivingData;

========================================



Speed Task

&#x20;     │

&#x20;     ▼

drivingData.speed 갱신

&#x20;     │

Distance Task

&#x20;     │

&#x20;     ▼

drivingData.distance 갱신

&#x20;     │

&#x20;     ▼

&#x20;  Shared DrivingData

&#x20;     │

&#x20;   (Mutex)

&#x20;     │

&#x20;┌────┴────┐

&#x20;▼         ▼

LCD Task  CAN TX Task

\----------------------------------------

\# 왜 ECU1 처럼 sensor Task 하나만 두지않고 speed Task,distance Task 로 분리하였나??



Speed Task와 Distance Task는 사용하는 센서의 측정 방식과 처리 시간이 서로 다르므로 독립적인 Task로 분리하였다. 

이를 통해 각 센서를 독립적으로 주기 제어할 수 있으며, FreeRTOS의 Round Robin 스케줄링을 이용하여 실시간으로 

최신 속도와 거리 데이터를 획득하도록 설계하였다. 또한 하나의 Task가 다른 센서 측정을 기다리지 않아 응답성과 

유지보수성을 향상시켰다.

\----------------------------------------



Task 별 예시 코드 





=================Speed Task=================

* 잘못된 코드일 수 있음 !!

for(;;)

{

&#x20;   speed = Read\_ADC();



&#x20;   xSemaphoreTake(drivingMutex, portMAX\_DELAY);



&#x20;   drivingData.speed = speed;



&#x20;   xSemaphoreGive(drivingMutex);



&#x20;   osDelay(20);

}



=================Distance Task=================

* 잘못된 코드일 수 있음 !!

for(;;)

{

&#x20;   distance = HC\_SR04\_Read();



&#x20;   xSemaphoreTake(drivingMutex, portMAX\_DELAY);



&#x20;   drivingData.distance = distance;



&#x20;   xSemaphoreGive(drivingMutex);



&#x20;   osDelay(20);

}



=================LCD Task=================

* 잘못된 코드일 수 있음 !!

xSemaphoreTake(drivingMutex, portMAX\_DELAY);



LCD\_Show(drivingData.speed,

&#x20;        drivingData.distance);



xSemaphoreGive(drivingMutex);



=================CAN TX Task=================

* 잘못된 코드일 수 있음 !!

xSemaphoreTake(drivingMutex, portMAX\_DELAY);



txData = drivingData;	// 공유 구조체를 복사해서 송신 !!

CAN\_Send(txData);



xSemaphoreGive(drivingMutex);

======================================================================================================================



Semaphore



1\. Binary Semaphore (CAN RX)



\# CAN 메시지 수신 이벤트를 CAN RX Task에 전달하기 위한 동기화 객체이다. 

&#x20; 인터럽트에서는 Semaphore를 Give하고, CAN RX Task는 Semaphore를 Take하여 제어 명령을 처리한다.



| Semaphore                   | Give                 | Take            | 역할                                                                                 |

| --------------------------- | -------------------- | --------------- | ----------------------------------------------------------------------------------   |

| \*\*CAN RX Binary Semaphore\*\* | \*\*CAN RX Interrupt\*\* | \*\*CAN RX Task\*\* | CAN 메시지가 수신되면 인터럽트에서 Semaphore를 Give하여 CAN RX Task를 깨우고,        |

&#x09;								 CAN RX Task가 제어 명령을 처리한다. 						      |

**----------------------------------**



2\. Mutex (DrivingData)



\# 여러 Task가 공유하는 DrivingData 구조체에 동시에 접근하지 못하도록 제어하는 동기화 객체이다. 

&#x20; 이를 통해 Speed Task, Distance Task, LCD Task, CAN TX Task 간 데이터의 일관성을 보장한다.



| Mutex                 | 사용 Task                                          | 역할                                                                |

| --------------------- | ------------------------------------------------ | -----------------------------------------------------------------     |

| \*\*DrivingData Mutex\*\* | Speed Task, Distance Task, LCD Task, CAN TX Task | `DrivingData` 공유 구조체에 여러 Task가 동시에 접근하지 못하도록      |

&#x09;								      보호하여 데이터의 일관성을 유지한다.				   |



======================================================================================================================



ECU 2의 CAN



TX : Speed, Distance, Heartbeat

RX : OTA, Reset, 설정 변경

======================================================================================================================

헤더파일

(더 추가해야할수도있음)

| 헤더 파일            | 역할                               |

| ----------------     | ----------------------------       |

| `main.h`             | GPIO, 핀 정의 및 프로젝트 공통 헤더|

| `adc\_speed.h`        | 가변저항(ADC) 속도 측정 함수       |

| `hcsr04.h`           | HC-SR04 거리 측정 함수             |

| `lcd\_i2c.h`          | I2C LCD 제어 함수                  |

| `can\_comm.h`         | CAN 송수신 함수 및 CAN ID 정의     |

| `tasks.h`            | FreeRTOS Task 생성 및 함수 선언    |

| `heartbeat.h`        | Heartbeat CAN 메시지 송신          |

| `driving\_data.h`     | `DrivingData` 구조체 및 Mutex 선언 |



































