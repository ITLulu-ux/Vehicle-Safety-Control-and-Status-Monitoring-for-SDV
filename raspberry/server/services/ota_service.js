const fs = require("fs");
const path = require("path");

const { sendCommandToEcu4 } = require("../gateway/uart_receiver");

function getOTAStatus(){

    return{

        version:"1.0.0",

        status:"READY"

    };

}

module.exports={

    getOTAStatus

};

async function startOTA(target) {

    const fileName = `ecu${target}_sensor.bin`;

    const filePath = path.join(
        __dirname,
        "..",
        "..",
        fileName
    );

    if (!fs.existsSync(filePath)) {
        throw new Error(`${fileName} 파일이 없습니다.`);
    }

    const firmware = fs.readFileSync(filePath);
    
    console.log("========== OTA ==========");
    console.log("Target :", target);
    console.log("File   :", fileName);
    console.log("Size   :", firmware.length, "Bytes");
    console.log(firmware.length);
    console.log(firmware.slice(0, 16));
    console.log("=========================");

    sendOTAStart(target, firmware.length);
    
    await sendOTAData(target, firmware);

    console.log("[CALL OTA END]");

    sendOTAEnd(target);
}

function sendOTAStart(target, firmwareSize) {

    const frame = Buffer.alloc(8);

    frame[0] = 0x02;          // CMD_OTA_START
    frame[1] = target;        // ECU1

    // 파일 크기 (Little Endian)
    frame.writeUInt32LE(firmwareSize, 2);

    frame[6] = 0x00;
    frame[7] = 0x00;

    console.log("[OTA START FRAME]", frame);

    sendCommandToEcu4(frame);

}

function delay(ms) {
    return new Promise(resolve => setTimeout(resolve, ms));
}

async function sendOTAData(target, firmware) {

    for (let offset = 0; offset < firmware.length; offset += 6) {

        const frame = Buffer.alloc(8);

        frame[0] = 0x03;
        frame[1] = target;

        const bytesToCopy = Math.min(6, firmware.length - offset);

        firmware.copy(frame, 2, offset, offset + bytesToCopy);

        sendCommandToEcu4(frame);

        await delay(2);   // 처음엔 2~5ms 정도 권장

        if (offset >= 60) {   // 약 11프레임 전송 후 종료
            break;
        }
    }

    console.log("[OTA DATA END]");

}

function sendOTAEnd(target) {

    const frame = Buffer.alloc(8);

    frame[0] = 0x04;      // CMD_OTA_END
    frame[1] = target;    // ECU1

    frame[2] = 0x00;
    frame[3] = 0x00;
    frame[4] = 0x00;
    frame[5] = 0x00;
    frame[6] = 0x00;
    frame[7] = 0x00;

    console.log("[OTA END FRAME]", frame);

    sendCommandToEcu4(frame);

}

module.exports = {

    getOTAStatus,
    startOTA,
    sendCommandToEcu4

};


// // 테스트 코드

// const fs = require("fs");
// const path = require("path");

// const { sendCommandToEcu4 } = require("../gateway/uart_receiver");

// function getOTAStatus() {
//     return {
//         version: "1.0.0",
//         status: "READY"
//     };
// }

// async function startOTA(target) {

//     const fileName = `ecu${target}_sensor.bin`;

//     const filePath = path.join(
//         __dirname,
//         "..",
//         "..",
//         fileName
//     );

//     if (!fs.existsSync(filePath)) {
//         throw new Error(`${fileName} 파일이 없습니다.`);
//     }

//     const firmware = fs.readFileSync(filePath);

//     sendOTAStart(target, firmware.length);

//     // ECU에서 Flash Erase 완료 대기
//     await delay(2000);

//     await sendOTAData(target, firmware);

//     sendOTAEnd(target);
// }

// function sendOTAStart(target, firmwareSize) {

//     const frame = Buffer.alloc(8);

//     frame[0] = 0x02;      // OTA_START
//     frame[1] = target;    // Target ECU

//     frame.writeUInt32LE(firmwareSize, 2);

//     sendCommandToEcu4(frame);
// }

// async function sendOTAData(target, firmware) {

//     for (let offset = 0; offset < firmware.length; offset += 6) {

//         const frame = Buffer.alloc(8);

//         frame[0] = 0x03;      // OTA_DATA
//         frame[1] = target;

//         const bytesToCopy = Math.min(6, firmware.length - offset);

//         firmware.copy(
//             frame,
//             2,
//             offset,
//             offset + bytesToCopy
//         );

//         sendCommandToEcu4(frame);

//         await delay(2);
//     }
// }

// function sendOTAEnd(target) {

//     const frame = Buffer.alloc(8);

//     frame[0] = 0x04;      // OTA_END
//     frame[1] = target;

//     sendCommandToEcu4(frame);
// }

// function delay(ms) {
//     return new Promise(resolve => setTimeout(resolve, ms));
// }

// module.exports = {
//     getOTAStatus,
//     startOTA
// };