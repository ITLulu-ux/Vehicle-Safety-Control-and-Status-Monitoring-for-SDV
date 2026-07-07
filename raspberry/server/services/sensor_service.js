const db = require('../../config/db');

async function saveSensorLog(packet) {

    await db.execute(

        `INSERT INTO sensor_log
        (can_id, temperature, humidity, lux, speed, distance)
        VALUES (?, ?, ?, ?, ?, ?)`,

        [
            packet.can_id ?? 0,
            packet.temperature,
            packet.humidity,
            packet.lux,
            packet.speed,
            packet.distance
        ]

    );

}

module.exports = {

    saveSensorLog

};