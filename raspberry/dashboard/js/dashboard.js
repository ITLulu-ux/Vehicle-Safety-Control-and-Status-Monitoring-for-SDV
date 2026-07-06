// dashboard.js

const table = document.getElementById("dashboardTable");

/**
 * 현재 시간 반환
 * 형식 : YYYY-MM-DD HH:mm:ss
 */
function getCurrentTime() {

    const now = new Date();

    const year = now.getFullYear();
    const month = String(now.getMonth() + 1).padStart(2, "0");
    const day = String(now.getDate()).padStart(2, "0");

    const hour = String(now.getHours()).padStart(2, "0");
    const minute = String(now.getMinutes()).padStart(2, "0");
    const second = String(now.getSeconds()).padStart(2, "0");

    return `${year}-${month}-${day} ${hour}:${minute}:${second}`;

}

/**
 * Dashboard 더미 데이터
 */
const sampleData = [

    {
        date: getCurrentTime(),
        ecu: "ECU1",
        status: "ACTIVE",
        message: "Connected"
    },

    {
        date: getCurrentTime(),
        ecu: "ECU2",
        status: "ACTIVE",
        message: "Sensor OK"
    },

    {
        date: getCurrentTime(),
        ecu: "ECU3",
        status: "UPDATING",
        message: "Downloading..."
    },

    {
        date: getCurrentTime(),
        ecu: "ECU4",
        status: "ERROR",
        message: "CAN Timeout"
    }

];

/**
 * 테이블 출력
 */
function loadTable(data) {

    table.innerHTML = "";

    data.forEach(item => {

        table.innerHTML += `

        <tr>

            <td>${item.date}</td>
            <td>${item.ecu}</td>
            <td>${item.status}</td>
            <td>${item.message}</td>

        </tr>

        `;

    });

}

/**
 * 최초 출력
 */
loadTable(sampleData);

/**
 * 검색 버튼
 */
document
.getElementById("searchBtn")
.addEventListener("click", () => {

    const ecu = document.getElementById("ecuFilter").value;

    const keyword = document
        .getElementById("keyword")
        .value
        .toLowerCase();

    const result = sampleData.filter(item => {

        return (

            (ecu === "" || item.ecu === ecu)

            &&

            (

                item.message.toLowerCase().includes(keyword)

                ||

                item.status.toLowerCase().includes(keyword)

                ||

                item.ecu.toLowerCase().includes(keyword)

            )

        );

    });

    loadTable(result);

});

/**
 * 1초마다 현재 시간 갱신
 */
setInterval(() => {

    sampleData.forEach(item => {

        item.date = getCurrentTime();

    });

    loadTable(sampleData);

}, 1000);