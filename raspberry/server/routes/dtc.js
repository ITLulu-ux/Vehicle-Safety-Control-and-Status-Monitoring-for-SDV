const express = require('express');

const router = express.Router();

const dtcService =
require('../services/dtc_service');

router.get('/',async(req,res)=>{

    const result =
    await dtcService.getAllDTC();

    res.json(result);

});

module.exports=router;