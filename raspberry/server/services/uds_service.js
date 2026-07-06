const db = require('../../config/db');
const dtcService = require('./dtc_service');

async function getUDSStatus() {

    const [heartbeat] = await db.execute(

        `SELECT
            ecu_name,
            status,
            last_received
         FROM heartbeat_log`

    );

    const dtc = await dtcService.getAllDTC();

    console.log("========== UDS ==========");

    heartbeat.forEach(item=>{

        console.log(

            `${item.ecu_name} : ${item.status}`

        );

    });

    console.log("-------------------------");

    console.log("DTC");

    dtc.forEach(item=>{

        console.log(

            item.dtc_code,

            item.description

        );

    });

    console.log("=========================");

    return{

        heartbeat,

        dtc

    };

}

module.exports={

    getUDSStatus

};