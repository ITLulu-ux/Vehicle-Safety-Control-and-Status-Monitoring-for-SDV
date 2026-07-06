const express = require('express');
const router = express.Router();

const vehicleState = require('../gateway/vehicle_state');

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

module.exports = router;