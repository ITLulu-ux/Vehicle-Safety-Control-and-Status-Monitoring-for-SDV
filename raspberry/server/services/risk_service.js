const vehicleState = require('../gateway/vehicle_state');

function updateVehicleState(packet) {

    vehicleState.timestamp = new Date();

    /* ECU1 */

    vehicleState.ecu1.alive = packet.hb1 === 1;
    vehicleState.ecu1.status =
        packet.hb1 ? "ACTIVE" : "DISCONNECTED";

    vehicleState.ecu1.temperature = packet.temperature;
    vehicleState.ecu1.humidity = packet.humidity;
    vehicleState.ecu1.lux = packet.lux;

    /* ECU2 */

    vehicleState.ecu2.alive = packet.hb2 === 1;
    vehicleState.ecu2.status =
        packet.hb2 ? "ACTIVE" : "DISCONNECTED";

    vehicleState.ecu2.speed = packet.speed;
    vehicleState.ecu2.distance = packet.distance;

    /* ECU3 */

    vehicleState.ecu3.alive = packet.hb3 === 1;
    vehicleState.ecu3.status =
        packet.hb3 ? "ACTIVE" : "DISCONNECTED";

    if (packet.riskLevel !== undefined)
        vehicleState.ecu3.riskLevel = packet.riskLevel;

    if (packet.brakeLevel !== undefined)
        vehicleState.ecu3.brakeLevel = packet.brakeLevel;

    if (packet.wiperState !== undefined)
        vehicleState.ecu3.wiperState = packet.wiperState;

    if (packet.ledState !== undefined)
        vehicleState.ecu3.ledState = packet.ledState;

}

function getVehicleState() {

    return vehicleState;

}

module.exports = {

    updateVehicleState,

    getVehicleState

};