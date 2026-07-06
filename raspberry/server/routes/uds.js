const express = require('express');
const router = express.Router();

const udsService = require('../services/uds_service');

router.get('/', async (req, res) => {

    try {

        const result = await udsService.getUDSStatus();

        res.json(result);

    } catch (err) {

        res.status(500).json({

            error: err.message

        });

    }

});

module.exports = router;