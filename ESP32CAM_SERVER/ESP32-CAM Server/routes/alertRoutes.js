const express = require("express");
const router = express.Router();
const alertController = require("../controllers/alertController");

router.post("/sendalerts", alertController.sendAlert);

module.exports = router;
