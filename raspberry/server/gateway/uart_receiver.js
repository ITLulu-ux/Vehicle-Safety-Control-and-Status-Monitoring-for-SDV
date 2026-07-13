const { SerialPort } = require('serialport');
const { ReadlineParser } = require('@serialport/parser-readline');

const config = require('../../config/config');
const { parseEcu4JsonLine } = require('./json_parser');

let port = null;

function startUartReceiver() {

    port = new SerialPort({
        path: config.SERIAL_PORT,
        baudRate: config.SERIAL_BAUD,
        dataBits: 8,
        stopBits: 1,
        parity: 'none',
        autoOpen: false
    });

    const parser = port.pipe(
        new ReadlineParser({
            delimiter: '\n'
        })
    );

    port.open((err) => {

        if (err) {
            console.error("[UART OPEN FAIL]", err.message);
            return;
        }

        console.log("[UART CONNECTED]", config.SERIAL_PORT);

    });

    parser.on('data', async (line) => {

        //console.log(line);
        const trimmed = line.trim();

        if (!trimmed) return;

        await parseEcu4JsonLine(trimmed);

    });

    port.on('error', (err) => {

        console.error("[UART ERROR]", err.message);

    });

}

function sendCommandToEcu4(command) {

    if (!port || !port.isOpen) {

        console.log("[UART] PORT CLOSED");
        return;

    }

    console.log("[UART TX]", command);

    port.write(command, (err) => {

        if (err) {
            console.error("[UART WRITE FAIL]", err.message);
        } else {
            console.log("[UART WRITE OK]");
        }

    });

}

module.exports = {
    startUartReceiver,
    sendCommandToEcu4
};