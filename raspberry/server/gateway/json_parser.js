const riskService = require('../services/risk_service');
const sensorService = require('../services/sensor_service');
const dtcService = require('../services/dtc_service');

async function parseEcu4JsonLine(line) {
    const normalized = line.replace(/\r/g, '').trim();

    if (!normalized) {
        return;
    }

    /*
     * UART 스트림에서 깨진 조각/잡음/복수 JSON 결합이 들어올 수 있어
     * 한 줄에서 JSON 객체 패턴만 추출해서 순차 처리한다.
     */
    const jsonCandidates = normalized.match(/\{[^{}]*\}/g);

    if (!jsonCandidates || jsonCandidates.length === 0) {
        return;
    }

    console.log("[RAW LINE]", JSON.stringify(normalized));
    console.log("[JSON CANDIDATES]", jsonCandidates);
    for (const jsonText of jsonCandidates) {
        try {
            const packet = JSON.parse(jsonText);

            if (packet.type !== "sensor") {
                continue;
            }

            /* 메모리 상태 갱신 */
            riskService.updateVehicleState(packet);

            /* Sensor DB 저장 */
            await sensorService.saveSensorLog(packet);

            /* Heartbeat 갱신 */
            if (packet.hb1)
                await dtcService.updateHeartbeat(1);

            if (packet.hb2)
                await dtcService.updateHeartbeat(2);

            if (packet.hb3)
                await dtcService.updateHeartbeat(3);

            if (packet.hb4)
                await dtcService.updateHeartbeat(4);
        }
        catch (err) {
            console.error("[JSON ERROR]", err.message);
            console.error(err.stack);
            console.error("[JSON TEXT]", jsonText);

        }
    }

}

module.exports = {

    parseEcu4JsonLine

};