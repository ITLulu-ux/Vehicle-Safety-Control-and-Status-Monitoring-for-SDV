const express = require('express');
const http = require('http');
const WebSocket = require('ws');
const path = require('path');

const apiRouter = require('./routes/api');
const { startUartReceiver } = require('./gateway/uart_receiver');
const vehicleState = require('./gateway/vehicle_state');

const app = express();

/* ---------------- HTTP ---------------- */

app.use(express.json());

app.use('/api', apiRouter);

/* Dashboard */
app.use(express.static(path.join(__dirname, '../dashboard')));

/* UDS Dashboard */
app.use('/uds', express.static(path.join(__dirname, '../uds_dashboard')));

/* ---------------- HTTP Server ---------------- */

const server = http.createServer(app);

/* ---------------- WebSocket ---------------- */

const wss = new WebSocket.Server({ server });

wss.on('connection', (ws) => {

    console.log("[WS] Dashboard Connected");

    /* 접속 즉시 현재 상태 전송 */
    ws.send(JSON.stringify(vehicleState));

    ws.on('close', () => {

        console.log("[WS] Dashboard Disconnected");

    });

});

/* ---------------- Dashboard Broadcast ---------------- */

/*
    vehicleState를 1초마다
    Dashboard로 전송
*/
setInterval(() => {

    const json = JSON.stringify(vehicleState);

    wss.clients.forEach(client => {

        if (client.readyState === WebSocket.OPEN) {

            client.send(json);

        }

    });

}, 1000);

/* ---------------- Start ---------------- */

server.listen(3000, () => {

    console.log("=================================");
    console.log(" Raspberry Pi Server Started");
    console.log(" HTTP      : 3000");
    console.log(" WebSocket : 3000");
    console.log(" UART       Ready");
    console.log("=================================");

    /* ECU4 UART 시작 */
    startUartReceiver();

});