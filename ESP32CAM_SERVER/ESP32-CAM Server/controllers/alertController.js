const { sendTextAlert } = require("./emailSender.js");

const sendAlert = (req, res) => {
    console.log("Received POST request to /sendalerts");

    const { subject, message } = req.body;

    if (!subject) {
        return res.status(400).send("Missing required fields");
    }

    sendTextAlert(subject, message);
    res.sendStatus(200);
};

module.exports = {
    sendAlert,
};
