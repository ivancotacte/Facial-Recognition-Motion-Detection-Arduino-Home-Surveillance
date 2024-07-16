const express = require("express");
const fileUpload = require("express-fileupload");
const moment = require("moment-timezone");
const fs = require("fs");

const app = express();
const PORT = process.env.PORT || 3000;

app.use(fileUpload({
    limits: {
        fileSize: 10000000,
    },
    abortOnLimit: true,
}));

app.get("/", (req, res) => {
    console.log("Received GET request to /");
    res.send("Hello World!");
});

app.post("/upload", (req, res) => {
    console.log("Received POST request to /upload");

    if (!req.files) {
        console.log("No files uploaded");
        return res.sendStatus(400);
    }

    let image = req.files.image;

    if (!image) {
        console.log("No image submitted");
        return res.sendStatus(400);
    }

    if (!/^image/.test(image.mimetype)) {
        console.log("Invalid image mimetype:", image.mimetype);
        return res.sendStatus(400);
    }

    image.name = moment().tz("Asia/Manila").format('YYYY-MM-DD_hh-mm-ssA') + "_" + image.name;

    if (!fs.existsSync("./images")) {
        fs.mkdirSync("./images");
    }

    const imagePath = "./images/" + image.name;
    image.mv(imagePath, (err) => {
        if (err) {
            console.log("Error uploading image:", err);
            return res.sendStatus(500);
        }

        console.log("Image uploaded successfully:", image.name);
        res.sendStatus(200);
    });
});

app.listen(PORT, () => {
    console.log(`Server listening on port ${PORT}`);
});