const db = require('../../config/db');
const dtcService = require('./dtc_service');

async function getUDSStatus() {

    /* Heartbeat 조회 */

    const [heartbeat] = await db.execute(

        `SELECT
            ecu_id,
            status,
            last_received
         FROM heartbeat_log
         ORDER BY ecu_id`

    );

    /* DTC 조회 */

    const dtc = await dtcService.getAllDTC();

    /* Raspberry Pi Console */

    console.log("========== UDS ==========");

    console.log("Heartbeat");

    heartbeat.forEach(item => {

        console.log(

            `ECU${item.ecu_id} : ${item.status}`

        );

    });

    console.log("-------------------------");

    console.log("DTC");

    if (dtc.length === 0) {

        console.log("No DTC");

    }
    else {

        dtc.forEach(item => {

            console.log(

                `[ECU${item.ecu_id}] ${item.dtc_code} - ${item.description} (${item.status})`

            );

        });

    }

    console.log("=========================");

    return {

        heartbeat,

        dtc

    };

}

module.exports = {

    getUDSStatus

};