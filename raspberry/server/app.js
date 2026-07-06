const express = require('express');
const http = require('http');
const net = require('net');
const WebSocket = require('ws');
const path = require('path');

const apiRouter = require('./routes/api');
const { parseIncomingWirelessPacket } = require('./gateway/json_parser');

const app = express();

/* ---------------- HTTP ---------------- */

app.use(express.json());

app.use('/api', apiRouter);

/* dashboard 폴더가 있다면 */
app.use(express.static(path.join(__dirname, '../dashboard')));

/* --- uds_dashboard 폴더가 있다면 --- */
app.use("/uds", express.static(path.join(__dirname,"../uds_dashboard")));

/* ---------------- HTTP Server ---------------- */

const server = http.createServer(app);

/* ---------------- WebSocket ---------------- */

const wss = new WebSocket.Server({ server });

wss.on('connection', (ws) => {

    console.log("[WS] Dashboard Connected");

    ws.on('close', () => {

        console.log("[WS] Dashboard Disconnected");

    });

});

/* ---------------- TCP ---------------- */

const tcpServer = net.createServer((socket)=>{

    console.log("[TCP] ECU4 Gateway Connected");

    socket.on('data',(buffer)=>{

        console.log("[TCP] Packet Received");
        console.log(buffer);

        parseIncomingWirelessPacket(buffer);

    });

});

/* ---------------- Start ---------------- */

server.listen(3000,()=>{

    console.log("HTTP Server : 3000");

});

tcpServer.listen(5000,()=>{

    console.log("TCP Server : 5000");

});