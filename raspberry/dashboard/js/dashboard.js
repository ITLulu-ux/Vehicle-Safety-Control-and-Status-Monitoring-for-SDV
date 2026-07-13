/**
 * dashboard.js
 * Vehicle Monitoring Dashboard
 */

const ECU_NAMES = {
    1: "ECU1 (Environment)",
    2: "ECU2 (Driving)",
    3: "ECU3 (Safety Control)",
    4: "ECU4 (Gateway)"
};

const HEARTBEAT_INTERVAL_MS = 1000;
const MAX_CAN_LOG_ROWS = 20;
const MAX_HEARTBEAT_LOG_ROWS = 20;
const ICON_PATH = "icon/";

let lastVehicleState = null;
let lastHeartbeatTimes = { 1: null, 2: null, 3: null, 4: null };
const heartbeatLogBuffer = [];

/* ===============================
    Helpers
=============================== */

function convertRiskLevel(level) {
    switch (level) {
        case 0: return "SAFE";
        case 1: return "CAUTION";
        case 2: return "WARNING";
        case 3: return "DANGER";
        default: return String(level);
    }
}

function riskColorClass(level) {
    switch (level) {
        case 0: return "text-green";
        case 1: return "text-yellow";
        case 2: return "text-orange";
        case 3: return "text-red";
        default: return "text-blue";
    }
}

function convertLedState(state) {
    switch (state) {
        case 0: return { text: "GREEN", className: "text-green" };
        case 1: return { text: "YELLOW", className: "text-yellow" };
        case 2: return { text: "RED", className: "text-red" };
        default: return { text: String(state), className: "text-blue" };
    }
}

function formatTime(date) {
    if (!date) return "-";
    const d = date instanceof Date ? date : new Date(date);
    if (Number.isNaN(d.getTime())) return "-";

    const pad = (n, len = 2) => String(n).padStart(len, "0");
    return `${pad(d.getHours())}:${pad(d.getMinutes())}:${pad(d.getSeconds())}.${pad(d.getMilliseconds(), 3)}`;
}

function formatDbTime(value) {
    if (!value) return "-";
    return formatTime(new Date(value));
}

function toHexByte(value) {
    return Number(value).toString(16).toUpperCase().padStart(2, "0");
}

function setText(id, text, className) {
    const el = document.getElementById(id);
    if (!el) return;
    el.textContent = text;
    if (className) {
        el.className = className;
    }
}

function setIconSrc(id, filename) {
    const el = document.getElementById(id);
    if (!el) return;
    el.src = `${ICON_PATH}${filename}`;
}

function isVehicleNormal(vehicleState) {
    const allAlive =
        vehicleState.ecu1.alive &&
        vehicleState.ecu2.alive &&
        vehicleState.ecu3.alive &&
        vehicleState.ecu4.alive;

    return allAlive && vehicleState.ecu3.riskLevel === 0;
}

function updateStatusIcons(vehicleState) {
    const normal = isVehicleNormal(vehicleState);
    const risk = vehicleState.ecu3.riskLevel;

    setIconSrc(
        "vehicleStatusIcon",
        normal ? "icon-check-circle_O.png" : "icon-check-circle_X.png"
    );

    setIconSrc(
        "riskShieldIcon",
        risk === 0 ? "icon-shield_O.png" : "icon-shield_X.png"
    );
}

/* ===============================
    Vehicle Dashboard
=============================== */

function updateVehicleStatusSummary(vehicleState) {
    const risk = vehicleState.ecu3.riskLevel;
    const riskText = convertRiskLevel(risk);
    const riskClass = riskColorClass(risk);

    const allAlive =
        vehicleState.ecu1.alive &&
        vehicleState.ecu2.alive &&
        vehicleState.ecu3.alive &&
        vehicleState.ecu4.alive;

    updateStatusIcons(vehicleState);

    if (!allAlive) {
        setText("vehicleStatus", "연결 확인 필요", "text-lg text-yellow");
        setText("vehicleStatusDesc", "일부 ECU와 통신이 끊어졌습니다.");
        return;
    }

    if (risk === 0) {
        setText("vehicleStatus", `정상 (${riskText})`, "text-lg text-green");
        setText("vehicleStatusDesc", "모든 시스템이 정상적으로 작동 중입니다.");
    } else if (risk === 1) {
        setText("vehicleStatus", `주의 (${riskText})`, "text-lg text-yellow");
        setText("vehicleStatusDesc", "주의 단계입니다. 전방 거리를 확인하세요.");
    } else if (risk === 2) {
        setText("vehicleStatus", `경고 (${riskText})`, "text-lg text-orange");
        setText("vehicleStatusDesc", "경고 단계입니다. 감속이 필요합니다.");
    } else {
        setText("vehicleStatus", `위험 (${riskText})`, "text-lg text-red");
        setText("vehicleStatusDesc", "위험 단계입니다. 즉시 정지하세요.");
    }

    const riskEl = document.getElementById("risk");
    if (riskEl) {
        riskEl.textContent = riskText;
        riskEl.className = riskClass;
    }
}

function appendCanLog(vehicleState) {
    const table = document.getElementById("canLogTable");
    if (!table) return;

    const now = new Date();
    const time = formatTime(now);
    const ecu1 = vehicleState.ecu1;
    const ecu2 = vehicleState.ecu2;
    const ecu3 = vehicleState.ecu3;

    const rows = [
        {
            canId: "0x100",
            ecu: "ECU1",
            data: [
                toHexByte(ecu1.temperature),
                toHexByte(ecu1.humidity),
                "01",
                toHexByte((ecu1.lux >> 8) & 0xff),
                toHexByte(ecu1.lux & 0xff),
                "00", "00", "00"
            ].join(" ")
        },
        {
            canId: "0x200",
            ecu: "ECU2",
            data: [
                toHexByte(ecu2.speed),
                toHexByte((ecu2.distance >> 8) & 0xff),
                toHexByte(ecu2.distance & 0xff),
                "00", "00", "00", "00", "00"
            ].join(" ")
        },
        {
            canId: "0x300",
            ecu: "ECU3",
            data: [
                toHexByte(ecu3.riskLevel),
                toHexByte(ecu3.brakeLevel),
                toHexByte(ecu3.wiperState),
                toHexByte(ecu3.ledState),
                "00", "00", "00", "00"
            ].join(" ")
        },
        {
            canId: "0x400",
            ecu: "ECU4",
            data: [
                toHexByte(vehicleState.ecu1.alive ? 1 : 0),
                toHexByte(vehicleState.ecu2.alive ? 1 : 0),
                toHexByte(vehicleState.ecu3.alive ? 1 : 0),
                toHexByte(vehicleState.ecu4.alive ? 1 : 0),
                "00", "00", "00", "00"
            ].join(" ")
        }
    ];

    if (table.rows.length === 1 && table.rows[0].cells.length === 1) {
        table.innerHTML = "";
    }

    rows.reverse().forEach((row) => {
        const tr = document.createElement("tr");
        tr.innerHTML = `
            <td>${time}</td>
            <td>${row.canId}</td>
            <td>${row.ecu}</td>
            <td>${row.data}</td>
        `;
        table.insertBefore(tr, table.firstChild);
    });

    while (table.rows.length > MAX_CAN_LOG_ROWS) {
        table.deleteRow(table.rows.length - 1);
    }
}

function updateVehicleDashboard(vehicleState) {
    lastVehicleState = vehicleState;

    updateVehicleStatusSummary(vehicleState);

    setText("brake", `${vehicleState.ecu3.brakeLevel} %`, "text-blue");
    setText("wiper", vehicleState.ecu3.wiperState ? "ON" : "OFF", "text-blue");

    const led = convertLedState(vehicleState.ecu3.ledState);
    setText("led", led.text, led.className);

    setText("temperature", `${vehicleState.ecu1.temperature} °C`, "text-blue");
    setText("humidity", `${vehicleState.ecu1.humidity} %`, "text-blue");
    setText("lux", `${vehicleState.ecu1.lux} lux`, "text-blue");
    setText("speed", `${vehicleState.ecu2.speed} km/h`, "text-yellow");
    setText("distance", `${vehicleState.ecu2.distance} cm`, "text-yellow");

    updateEcuConnectionFromState(vehicleState);
    updateHeartbeatBarsFromState(vehicleState);
    appendHeartbeatLogFromState(vehicleState);
    appendCanLog(vehicleState);
}

/* ===============================
    UDS Dashboard
=============================== */

async function updateUDSDashboard() {
    try {
        const response = await fetch("/api/dtc");
        const rows = await response.json();
        const table = document.getElementById("dtcTable");
        const emptyEl = document.getElementById("dtcEmpty");

        if (!table) return;

        table.innerHTML = "";

        if (!rows.length) {
            setText("dtcSummary", "현재 DTC가 없습니다.");
            setText("dtcSummaryEn", "No DTC");
            setIconSrc("dtcStatusIcon", "icon-check-circle.png");
            if (emptyEl) emptyEl.style.display = "flex";
            return;
        }

        if (emptyEl) emptyEl.style.display = "none";

        setText("dtcSummary", `활성 DTC ${rows.length}건`);
        setText("dtcSummaryEn", `${rows.length} Active DTC`);
        setIconSrc("dtcStatusIcon", "icon-check-circle_X.png");

        rows.forEach((item) => {
            const tr = document.createElement("tr");
            tr.innerHTML = `
                <td>${item.dtc_code}</td>
                <td>${item.description}</td>
                <td>${formatDbTime(item.timestamp)}</td>
                <td>${item.status}</td>
            `;
            table.appendChild(tr);
        });
    } catch (err) {
        console.error("[DTC ERROR]", err);
    }
}

function bindControlButtons() {
    const clearButtons = [
        document.getElementById("btnClear"),
        document.getElementById("btnClearAll")
    ];

    clearButtons.forEach((btn) => {
        if (!btn) return;
        // btn.addEventListener("click", () => {
        //     alert("DTC 삭제 API는 아직 연결되지 않았습니다.");
        // });

        btn.addEventListener("click", async () => {
    try {
        const res = await fetch("/api/dtc/clear", {
            method: "POST"
        });

        const result = await res.json();

        alert(result.message);

        // 목록 새로고침
        loadDashboard();

    } catch (err) {
        alert("DTC 삭제 실패");
        console.error(err);
    }
        });
    });

    const otaBtn = document.getElementById("btnOta");

    if (otaBtn) {

    otaBtn.addEventListener("click", async () => {

        try {

            const res = await fetch("/api/ota/start", {
                method: "POST",
                headers: {
                    "Content-Type": "application/json"
                },
                body: JSON.stringify({
                    target: 1
                })
            });

            const result = await res.json();

            alert(result.message);

        } catch (err) {

            console.error(err);

            alert("OTA 실행 실패");

        }

    });

    }
}

/* ===============================
    Heartbeat Dashboard
=============================== */

function updateEcuConnectionFromState(vehicleState) {
    const table = document.getElementById("ecuConnectionTable");
    if (!table) return;

    const ecus = [
        { id: 1, name: ECU_NAMES[1], alive: vehicleState.ecu1.alive },
        { id: 2, name: ECU_NAMES[2], alive: vehicleState.ecu2.alive },
        { id: 3, name: ECU_NAMES[3], alive: vehicleState.ecu3.alive },
        { id: 4, name: ECU_NAMES[4], alive: vehicleState.ecu4.alive }
    ];

    table.innerHTML = ecus.map((ecu) => {
        const lastTime = lastHeartbeatTimes[ecu.id];
        const statusText = ecu.alive ? "Connected" : "Disconnected";
        const statusClass = ecu.alive ? "text-green" : "text-red";
        const dotClass = ecu.alive ? "status-dot" : "status-dot disconnected";
        const lossRate = ecu.alive ? "0 %" : "100 %";

        return `
            <tr>
                <td>${ecu.name}</td>
                <td><span class="${dotClass}"></span><span class="${statusClass}">${statusText}</span></td>
                <td>${formatTime(lastTime)}</td>
                <td>${lossRate}</td>
            </tr>
        `;
    }).join("");
}

function updateHeartbeatBarsFromState(vehicleState) {
    const aliveMap = {
        1: vehicleState.ecu1.alive,
        2: vehicleState.ecu2.alive,
        3: vehicleState.ecu3.alive,
        4: vehicleState.ecu4.alive
    };

    const now = Date.now();

    Object.keys(aliveMap).forEach((key) => {
        const ecuId = Number(key);
        const bar = document.getElementById(`hbBar${ecuId}`);
        const msEl = document.getElementById(`hbMs${ecuId}`);
        if (!bar || !msEl) return;

        if (aliveMap[ecuId]) {
            lastHeartbeatTimes[ecuId] = new Date();
        }

        const last = lastHeartbeatTimes[ecuId];
        if (!last) {
            bar.style.width = "0%";
            bar.className = "progress-bar danger";
            msEl.textContent = "-";
            return;
        }

        const elapsed = now - last.getTime();
        const width = Math.max(0, Math.min(100, (elapsed / HEARTBEAT_INTERVAL_MS) * 100));

        bar.style.width = `${width}%`;
        msEl.textContent = `${elapsed} ms`;

        if (!aliveMap[ecuId] || elapsed > HEARTBEAT_INTERVAL_MS * 1.25) {
            bar.className = "progress-bar danger";
        } else if (elapsed > HEARTBEAT_INTERVAL_MS) {
            bar.className = "progress-bar warning";
        } else {
            bar.className = "progress-bar";
        }
    });
}

function appendHeartbeatLogFromState(vehicleState) {
    const now = new Date();
    const ecus = [
        { id: 1, alive: vehicleState.ecu1.alive },
        { id: 2, alive: vehicleState.ecu2.alive },
        { id: 3, alive: vehicleState.ecu3.alive },
        { id: 4, alive: vehicleState.ecu4.alive }
    ];

    ecus.forEach((ecu) => {
        heartbeatLogBuffer.unshift({
            time: now,
            ecu: ECU_NAMES[ecu.id],
            status: ecu.alive ? "OK" : "LOST"
        });
    });

    while (heartbeatLogBuffer.length > MAX_HEARTBEAT_LOG_ROWS) {
        heartbeatLogBuffer.pop();
    }

    renderHeartbeatLogTable();
}

function renderHeartbeatLogTable() {
    const table = document.getElementById("heartbeatTable");
    if (!table) return;

    if (!heartbeatLogBuffer.length) {
        table.innerHTML = `
            <tr>
                <td colspan="3" class="text-sm">Heartbeat 로그가 없습니다.</td>
            </tr>
        `;
        return;
    }

    table.innerHTML = heartbeatLogBuffer.map((item) => `
        <tr>
            <td>${formatTime(item.time)}</td>
            <td>${item.ecu}</td>
            <td class="${item.status === "OK" ? "text-green" : "text-red"}">${item.status}</td>
        </tr>
    `).join("");
}

async function updateHeartbeatDashboard() {
    try {
        const response = await fetch("/api/heartbeat");
        const rows = await response.json();

        rows.forEach((item) => {
            const ecuId = Number(item.ecu_id);
            if (!ecuId) return;
            lastHeartbeatTimes[ecuId] = new Date(item.last_received);
        });

        if (lastVehicleState) {
            updateEcuConnectionFromState(lastVehicleState);
            updateHeartbeatBarsFromState(lastVehicleState);
        }
    } catch (err) {
        console.error("[Heartbeat ERROR]", err);
    }
}

/* ===============================
    WebSocket Callback
=============================== */

function onVehicleStateReceived(vehicleState) {
    updateVehicleDashboard(vehicleState);
}

/* ===============================
    Initial Load
=============================== */

bindControlButtons();
updateUDSDashboard();
updateHeartbeatDashboard();

setInterval(() => {
    updateUDSDashboard();
    updateHeartbeatDashboard();
    if (lastVehicleState) {
        updateHeartbeatBarsFromState(lastVehicleState);
    }
}, 3000);

setInterval(() => {
    if (lastVehicleState) {
        updateHeartbeatBarsFromState(lastVehicleState);
    }
}, 500);
