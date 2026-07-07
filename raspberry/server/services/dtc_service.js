const db = require('../../config/db');

/**
 * DTC 저장
 */
async function saveDTC(dtc) {

    await db.execute(

        `INSERT INTO dtc_log
        (ecu_id, dtc_code, description, status)
        VALUES (?, ?, ?, ?)`,

        [
            dtc.ecu_id,
            dtc.dtc_code,
            dtc.description,
            dtc.status
        ]

    );

}

/**
 * DTC 전체 조회
 */
async function getAllDTC() {

    const [rows] = await db.execute(

        `SELECT *
         FROM dtc_log
         ORDER BY timestamp DESC`

    );

    return rows;

}

/**
 * Heartbeat 갱신
 */
async function updateHeartbeat(ecuId) {

    await db.execute(

        `INSERT INTO heartbeat_log
        (ecu_id, last_received, status)

        VALUES (?, NOW(), 'CONNECTED')

        ON DUPLICATE KEY UPDATE

        last_received = NOW(),
        status = 'CONNECTED'`,

        [ecuId]

    );

}

/**
 * Heartbeat 조회
 */
async function getHeartbeat() {

    const [rows] = await db.execute(

        `SELECT
            ecu_id,
            last_received,
            status
         FROM heartbeat_log
         ORDER BY ecu_id`

    );

    return rows;

}

module.exports = {

    saveDTC,

    getAllDTC,

    updateHeartbeat,

    getHeartbeat

};