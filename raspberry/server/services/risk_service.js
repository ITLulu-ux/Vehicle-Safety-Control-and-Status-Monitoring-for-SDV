const vehicleState = require('../gateway/vehicle_state');

/**
 * ECU3 상태 갱신
 */
function updateECU3State(stdId, dataBytes) {

    const now = new Date();

    vehicleState.timestamp = now;

    vehicleState.ecu3.lastHeartbeat = now;
    vehicleState.ecu3.alive = true;
    vehicleState.ecu3.status = "ACTIVE";

    switch (stdId) {

        case 0x300:

            vehicleState.ecu3.riskLevel  = dataBytes[0];
            vehicleState.ecu3.brakeLevel = dataBytes[1];
            vehicleState.ecu3.wiperState = dataBytes[2];
            vehicleState.ecu3.ledState   = dataBytes[3];

            console.log(
                `[ECU3] Risk=${vehicleState.ecu3.riskLevel}`
            );

            break;

        case 0x703:

            // Heartbeat만 갱신
            break;

    }

}

/**
 * ECU1 / ECU2 센서 데이터 갱신
 */
function updateExternalSensor(stdId, dataBytes) {

    vehicleState.timestamp = new Date();

    switch (stdId) {

        // ECU1
        case 0x100:

            vehicleState.ecu1.alive = true;
            vehicleState.ecu1.status = "ACTIVE";

            vehicleState.ecu1.temperature = dataBytes[0];
            vehicleState.ecu1.humidity    = dataBytes[1];
            vehicleState.ecu1.lux =
                (dataBytes[2] << 8) | dataBytes[3];

            break;

        // ECU2
        case 0x200:

            vehicleState.ecu2.alive = true;
            vehicleState.ecu2.status = "ACTIVE";

            vehicleState.ecu2.speed    = dataBytes[0];
            vehicleState.ecu2.distance = dataBytes[1];

            break;

    }

}

/**
 * ECU3 Heartbeat 감시
 */
setInterval(() => {

    if (!vehicleState.ecu3.lastHeartbeat)
        return;

    const diff =
        Date.now() -
        vehicleState.ecu3.lastHeartbeat.getTime();

    if (diff > 3000) {

        vehicleState.ecu3.alive = false;
        vehicleState.ecu3.status = "DISCONNECTED";

        console.warn("[ALERT] ECU3 Communication Timeout");

    }

}, 2000);

module.exports = {

    getVehicleState: () => vehicleState,

    updateECU3State,

    updateExternalSensor

};