const express = require('express');
const router = express.Router();

const vehicleState = require('../gateway/vehicle_state');
const dtcService = require('../services/dtc_service');

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
 * GET /api/heartbeat
 * ECU Heartbeat 조회
 */
router.get('/heartbeat', async (req, res) => {

    try {

        const rows = await dtcService.getHeartbeat();

        res.json(rows);

    }
    catch (err) {

        console.error("[API HEARTBEAT ERROR]", err);

        res.status(500).json({

            status: "fail",

            message: "Failed to load heartbeat"

        });

    }

});

/**
 * GET /api/dtc
 * DTC 목록 조회
 */
router.get('/dtc', async (req, res) => {

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

module.exports = router;