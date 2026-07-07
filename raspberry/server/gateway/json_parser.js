const riskService = require('../services/risk_service');
const sensorService = require('../services/sensor_service');
const dtcService = require('../services/dtc_service');

async function parseEcu4JsonLine(line) {

    try {

        const packet = JSON.parse(line);

        if (packet.type !== "sensor") {
            return;
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

    }

}

module.exports = {

    parseEcu4JsonLine

};