const moment = require("moment-timezone");
const { sendEmailImage } = require("./emailSender.js");
const fs = require("fs");

const uploadImage = (req, res) => {
    console.log("Received POST request to /upload");

    if (!req.files || !req.files.image) {
        console.log("No files or image uploaded");
        return res.status(400).send("No files or image uploaded");
    }

    const image = req.files.image;

    if (!/^image/.test(image.mimetype)) {
        console.log("Invalid image mimetype:", image.mimetype);
        return res.status(400).send("Invalid image mimetype");
    }

    image.name = moment().tz("Asia/Manila").format('YYYY-MM-DD_hh-mm-ssA') + "_" + image.name;

    const imagesDir = "./images";
    if (!fs.existsSync(imagesDir)) {
        fs.mkdirSync(imagesDir);
    }

    const imagePath = `${imagesDir}/${image.name}`;
    image.mv(imagePath, (err) => {
        if (err) {
            console.log("Error uploading image:", err);
            return res.status(500).send("Error uploading image");
        }

        if (image.name !== "IntruderAlert.jpg") {
            sendEmailImage("intruderFace", imagePath);
        }

        console.log("Image uploaded successfully:", image.name);
        res.status(200).send("Image uploaded successfully");
    });
};

module.exports = {
    uploadImage,
};