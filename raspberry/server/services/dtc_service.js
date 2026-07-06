const db = require('../../config/db');

async function saveDTC(dtc) {

    await db.execute(

        `INSERT INTO dtc_log
        (dtc_code, description, ecu, status)
        VALUES (?, ?, ?, ?)`,

        [
            dtc.dtc_code,
            dtc.description,
            dtc.ecu,
            dtc.status
        ]

    );

}

async function getAllDTC() {

    const [rows] = await db.execute(

        `SELECT *
         FROM dtc_log
         ORDER BY timestamp DESC`

    );

    return rows;
}

module.exports = {
    saveDTC,
    getAllDTC,
    updateHeartbeat
};

async function updateHeartbeat(ecuName){

    await db.execute(

        `INSERT INTO heartbeat_log
        (ecu_name,last_received,status)

        VALUES(?,NOW(),'CONNECTED')

        ON DUPLICATE KEY UPDATE

        last_received=NOW(),
        status='CONNECTED'`,

        [ecuName]

    );

}