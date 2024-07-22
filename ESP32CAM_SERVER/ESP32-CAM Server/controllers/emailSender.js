const nodemailer = require("nodemailer");
require("dotenv").config();
const moment = require("moment-timezone");

const emailAddress = [
    "cotactearmenion@gmail.com",
    "laurenceaguio08@gmail.com",
    "merjenethmelendres@gmail.com"
];

const transporter = nodemailer.createTransport({
    service: "gmail",
    port: 465,
    secure: true,
    auth: {
        user: process.env.EMAIL_ADDRESS,
        pass: process.env.EMAIL_PASSWORD,
    },
    tls: {
        rejectUnauthorized: false,
    },
});

async function sendTextAlert(alertType, alertMessage) {
    try {
        let currentTime = moment().tz("Asia/Manila").format('YYYY-MM-DD HH:mm:ss');
        alertMessage += `\n\nTime: ${currentTime}`;
        
        const mailOptions = {
            from: '"GROUP 10 - LFSA322N002 👻" <cotactearmenion@gmail.com>',
            to: emailAddress,
            subject: alertType,
            text: alertMessage,
        };
        transporter.sendMail(mailOptions, (error, info) => {
            if (error) {
                console.error("Error: ", error);
            } else {
                console.log("Success: ", info.response);
            }
        });
    } catch (error) {
        console.error("Error: ", error);
    }
}

async function sendEmailImage(alertType, imagePath) {
    try {
        let alertMessage = "";
        let customMessage = "";
        let currentTime = moment().tz("Asia/Manila").format('YYYY-MM-DD HH:mm:ss');

        switch (alertType) {
            case "intruderFace":
                alertMessage = "ESP32CAM: Intruder Detected";
                customMessage = `An intruder was detected by the ESP32CAM system at ${currentTime}. Please find the attached image for more details.`;
                break;
        }

        const mailOptions = {
            from: '"GROUP 1 - LFSA322N002 👻" <merjenethmelendres@gmail.com>',
            to: emailAddress,
            subject: alertMessage,
            text: customMessage,
            attachments: [
                {
                    filename: imagePath.split("/").pop(),
                    path: imagePath
                }
            ]
        };

        transporter.sendMail(mailOptions, (error, info) => {
            if (error) {
                console.error("Error: ", error);
            } else {
                console.log("Success: ", info.response);
            }
        });
    } catch (error) {
        console.error("Error: ", error);
    }
}

module.exports = { sendEmailImage, sendTextAlert };
