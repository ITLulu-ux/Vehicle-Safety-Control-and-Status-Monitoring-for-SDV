/**
 * websocket.js
 * Raspberry Pi WebSocket Client
 */

const socket = new WebSocket(
    `ws://${location.hostname}:3000`
);

/**
 * 최초 차량 상태 조회
 */
async function loadInitialVehicleState() {

    try {

        const response = await fetch("/api/vehicle");

        const result = await response.json();

        if (result.status === "success") {

            onVehicleStateReceived(result.data);

        }

    }
    catch (err) {

        console.error("[Vehicle API Error]", err);

    }

}

/**
 * 연결 성공
 */
socket.onopen = () => {

    console.log("[WebSocket] Connected");

};

/**
 * 서버 데이터 수신
 */
socket.onmessage = (event) => {

    try {

        const vehicleState = JSON.parse(event.data);

        onVehicleStateReceived(vehicleState);

    }
    catch (err) {

        console.error("[WebSocket Parse Error]", err);

    }

};

/**
 * 연결 종료
 */
socket.onclose = () => {

    console.log("[WebSocket] Disconnected");

};

/**
 * 오류 발생
 */
socket.onerror = (err) => {

    console.error("[WebSocket Error]", err);

};

/* 최초 데이터 로드 */
loadInitialVehicleState();