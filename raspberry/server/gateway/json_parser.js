/*const riskService = require('../services/risk_service'); // 바뀌어야할수도

/**
 * ECU4 Gateway로부터 유입된 바이너리 버퍼 스트림 해석
 * 규격: StdId (2바이트) + DLC (1바이트) + Data (8바이트) = 총 11바이트 패킷
 
function parseIncomingWirelessPacket(buffer) {
    if (!buffer || buffer.length < 11) return;

    try {
        const stdId = buffer.readUInt16BE(0);
        const dlc = buffer.readUInt8(2);
        const dataBytes = buffer.slice(3, 3 + dlc);

        // ID 권역별로 해당하는 서비스 레이어로 토스
        if (stdId === 0x300 || stdId === 0x703) {
            riskService.updateECU3State(stdId, dataBytes);
        } else if (stdId === 0x100 || stdId === 0x200) {
            riskService.updateExternalSensor(stdId, dataBytes);
        }
    } catch (error) {
        console.error('[Parser Error] TCP Stream Corruption:', error.message);
    }
}

module.exports = {
    parseIncomingWirelessPacket
};*/



const riskService = require('../services/risk_service');

/**
 * ECU4 Gateway로부터 JSON 데이터 수신
 * ECU4 → Raspberry Pi
 */
function parseIncomingWirelessPacket(buffer) {

    if (!buffer) return;

    try {

        // Buffer → String → JSON
        const packet = JSON.parse(buffer.toString());

        console.log("[JSON RX]", packet);

        /*
            예시 JSON

            {
                "stdId": "0x300",
                "data": [2,1,0,1]
            }
        */

        const stdId =
            typeof packet.stdId === "string"
                ? parseInt(packet.stdId, 16)
                : packet.stdId;

        const dataBytes = Buffer.from(packet.data);

        switch (stdId) {

            // ECU3 제어 데이터
            case 0x300:

                riskService.updateECU3State(
                    stdId,
                    dataBytes
                );

                break;

            // ECU3 Heartbeat
            case 0x703:

                riskService.updateECU3State(
                    stdId,
                    dataBytes
                );

                break;

            // ECU1 환경센서
            case 0x100:

                riskService.updateExternalSensor(
                    stdId,
                    dataBytes
                );

                break;

            // ECU2 주행데이터
            case 0x200:

                riskService.updateExternalSensor(
                    stdId,
                    dataBytes
                );

                break;

            default:

                console.warn(
                    `[Unknown CAN ID] ${packet.stdId}`
                );

                break;
        }

    }
    catch (error) {

        console.error(
            "[JSON Parser Error]",
            error.message
        );

    }

}

module.exports = {
    parseIncomingWirelessPacket
};