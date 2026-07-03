/*
 * risk.c
 *
 *  Created on: 2026. 6. 30.
 *      Author: rlaek
 */

#include "risk.h"
#include "control_data.h"

/**
  * @brief 수신된 초음파 거리를 기반으로 위험도 단계를 자체 판단 연산
  */
void Process_Risk_Analysis(void)
{
    if (osMutexAcquire(Mutex_RiskDataHandle, osWaitForever) == osOK)
    {
        uint16_t dist = drivingData.distance;

        if (dist > 0 && dist < 20)       controlData.riskLevel = RISK_DANGER;
        else if (dist >= 20 && dist < 50) controlData.riskLevel = RISK_WARNING;
        else if (dist >= 50 && dist < 80) controlData.riskLevel = RISK_CAUTION;
        else                              controlData.riskLevel = RISK_SAFE;

        osMutexRelease(Mutex_RiskDataHandle);
    }
}
