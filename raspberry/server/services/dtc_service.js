const db = require('../../config/db');

/**
 * DTC 저장
 */
async function saveDTC(dtc) {

    console.log("[HB]", dtc.ecu_id);
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

    console.log("[HB DONE]");
}

/**
 * ACTIVE DTC 존재 여부
 */
async function existsActiveDTC(ecuId, dtcCode) {

    console.log("[HB]", ecuId);
    const [rows] = await db.execute(

        `SELECT id
         FROM dtc_log
         WHERE ecu_id = ?
         AND dtc_code = ?
         AND status='ACTIVE'
         LIMIT 1`,

        [ecuId, dtcCode]

    );

    console.log("[HB DONE]");
    return rows.length > 0;

}

/**
 * DTC 조회
 */
async function getAllDTC() {

    //console.log("[HB]", ecuId);
    const [rows] = await db.execute(

        `SELECT *
         FROM dtc_log
         ORDER BY timestamp DESC`

    );

    console.log("[HB DONE]");
    return rows;

}

/**
 * Heartbeat 갱신
 */
async function updateHeartbeat(ecuId) {

    console.log("[HB]", ecuId);
    await db.execute(

        `INSERT INTO heartbeat_log
        (ecu_id,last_received,status)

        VALUES(?,NOW(),'CONNECTED')

        ON DUPLICATE KEY UPDATE

        last_received=NOW(),

        status='CONNECTED'`,

        [ecuId]

    );
    console.log("[HB DONE]");

}

/**
 * Heartbeat 조회
 */
async function getHeartbeat() {

    //console.log("[HB]", ecuId);
    const [rows] = await db.execute(

        `SELECT
            ecu_id,
            last_received,
            status
         FROM heartbeat_log
         ORDER BY ecu_id`

    );

    console.log("[HB DONE]");
    return rows;

}

/**
 * Heartbeat Timeout 검사
 */
async function checkHeartbeatTimeout() {

    const rows = await getHeartbeat();

    for (const ecu of rows) {

        const diff =
            Date.now() -
            new Date(ecu.last_received).getTime();

        if (diff > 3000 && ecu.status === 'CONNECTED') {

            console.log("[HB]", ecu.ecu_id);
            await db.execute(

                `UPDATE heartbeat_log
                 SET status='DISCONNECTED'
                 WHERE ecu_id=?`,

                [ecu.ecu_id]

            );

            console.log("[HB DONE]");
            const exists =
                await existsActiveDTC(
                    ecu.ecu_id,
                    'P0702'
                );

            if (!exists) {

                await saveDTC({

                    ecu_id: ecu.ecu_id,

                    dtc_code: 'P0702',

                    description: 'Heartbeat Timeout',

                    status: 'ACTIVE'

                });

            }

        }

    }

}


function startHeartbeatMonitor() {
    setInterval(checkHeartbeatTimeout,1000);
}

async function clearAllDTC() {

    // STM32 Clear 명령
    //sendDtcClearCommand();

    // DB 삭제
    await db.query(
        "DELETE FROM dtc_log"
    );

    return {
        success: true
    };
}

async function getDTCList() {
    const [rows] = await db.query(
        "SELECT * FROM dtc_log ORDER BY id DESC"
    );

    return rows;
}
/* 1초마다 검사 */

setInterval(checkHeartbeatTimeout,1000);

module.exports = {

    saveDTC,

    existsActiveDTC,

    getAllDTC,

    updateHeartbeat,

    getHeartbeat,

    checkHeartbeatTimeout,

    startHeartbeatMonitor,

    clearAllDTC,

    getDTCList

};