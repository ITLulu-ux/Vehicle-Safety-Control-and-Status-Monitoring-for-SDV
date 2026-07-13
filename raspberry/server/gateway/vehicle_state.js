/**
 * 차량 전체 상태 저장소 (Memory Store)
 * 모든 Service에서 공유
 */

const vehicleState = {

    timestamp: null,

    ecu1: {
    status: "DISCONNECTED",
    alive: false,

    temperature: 0,
    humidity: 0,
    lux: 0
},

ecu2: {
    status: "DISCONNECTED",
    alive: false,

    speed: 0,
    distance: 0
},

ecu3: {
    status: "DISCONNECTED",
    alive: false,

    riskLevel: 0,
    brakeLevel: 0,
    wiperState: 0,
    ledState: 0,

    lastHeartbeat: null
},

ecu4: {
    status: "DISCONNECTED",
    alive: false
}

};

module.exports = vehicleState;