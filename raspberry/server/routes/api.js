const otaService = require('../services/ota_service');
const express = require('express');
const router = express.Router();

const vehicleState = require('../gateway/vehicle_state');

const isAws = process.env.MODE === "AWS";

let dtcService = null;

if (!isAws) {
    dtcService = require('../services/dtc_service');
}

/**
 * GET /api/vehicle
 * 차량 전체 상태 조회
 */
router.get('/vehicle', (req, res) => {

    res.json({

        status: "success",

        timestamp: vehicleState.timestamp,

        data: vehicleState

    });

});

/**
 * GET /api/dtc
 * DTC 목록 조회
 */
router.get('/dtc', async (req, res) => {

    if (isAws) {
        return res.json([]);
    }

    try {

        const rows = await dtcService.getAllDTC();

        res.json(rows);

    }
    catch (err) {

        console.error("[API DTC ERROR]", err);

        res.status(500).json({

            status: "fail",

            message: "Failed to load DTC"

        });

    }

});

/* DTC_CLEAR */
router.post("/dtc/clear", async (req, res) => {

    try {

        await dtcService.clearAllDTC();

        res.json({
            success: true,
            message: "DTC 삭제 완료"
        });

    } catch(err){

       console.error("[DTC CLEAR ERROR]", err);

        res.status(500).json({
            success: false,
            message: err.message
    });

    }

});

router.get('/heartbeat', async (req, res) => {

    if (isAws) {
        return res.json([]);
    }

    try {

        const rows = await dtcService.getHeartbeat();

        res.json(rows);

    } catch (err) {

        console.error("[API HEARTBEAT ERROR]", err);

        res.status(500).json({
            success: false,
            message: "Heartbeat 조회 실패"
        });

    }

});

router.post('/ota/start', async (req, res) => {

    try {

        const target = 1;      // 현재 ECU1만 테스트

        await otaService.startOTA(target);

        res.json({
            success: true,
            message: "OTA 시작"
        });

    } catch (err) {

        console.error("[OTA ERROR]", err);

        res.status(500).json({
            success: false,
            message: err.message
        });

    }

});

module.exports = router;