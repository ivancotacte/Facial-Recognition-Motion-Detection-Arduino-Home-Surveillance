const nodemailer = require("nodemailer");

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

async function sendEmail(alertType, doorStatus = null) {
    const emailAddress = [
        "cotactearmenion@gmail.com", 
        "jonhdarryld@gmail.com",
        "dominic.loreno0@gmail.com",
        "anjeloasuncion@gmail.com",
        "leernadsadasop@gmail.com"
    ];

    try {
        let alertMessage;
        if (alertType === "door") {
            alertMessage = `Alert: The door has been ${doorStatus}`;
        } else if (alertType === "intruder") {
            alertMessage = "Alert: Possible intruder detected due to multiple failed attempts to unlock the door.";
        } else if (alertType === "someoneChangepasscode") {
            alertMessage = "Alert: Someone has changing the passcode.";
        } else if (alertType === "successChangepasscode") {
            alertMessage = "Alert: Passcode has been successfully changed.";
        }

        const currentDate = new Date();
        const formattedDate = currentDate.toLocaleDateString();
        const formattedTime = currentDate.toLocaleTimeString();

        const mailOptions = {
            from: '"GROUP 10 - LFSA322N002 👻" <cotactearmenion@gmail.com>',
            to: emailAddress,
            subject: alertMessage,
            html: `
                <html>
                    <head>
                        <title>Alert</title>
                    </head>
                    <body>
                        <p>${alertMessage}</p>
                        <p>Date: ${formattedDate}</p>
                        <p>Time: ${formattedTime}</p>
                    </body>
                </html>
            `,
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

module.exports = { sendEmail };