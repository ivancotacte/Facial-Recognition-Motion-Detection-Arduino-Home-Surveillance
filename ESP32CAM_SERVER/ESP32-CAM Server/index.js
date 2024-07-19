const express = require("express");
const fileUpload = require("express-fileupload");
const uploadRoutes = require("./routes/uploadRoutes");
const alertRoutes = require("./routes/alertRoutes");
const bodyParser = require("body-parser");
const path = require("path");
const fs = require("fs");

const app = express();
const PORT = process.env.PORT || 3000;

app.use(fileUpload({
    limits: {
        fileSize: 10000000,
    },
    abortOnLimit: true,
}));
app.use("/images", express.static(path.join(__dirname, "images")));
app.use(express.static(path.join(__dirname, "public")));

app.use(bodyParser.urlencoded({ extended: false }));
app.use(bodyParser.json());

app.set("view engine", "ejs");
app.set("views", path.join(__dirname, "views"));

app.get("/", (req, res) => {
    console.log("Received GET request to /");
    res.send("Hello World!");
});

app.get("/gallery", (req, res) => {
    const imagesDir = path.join(__dirname, "images");
    fs.readdir(imagesDir, (err, files) => {
        if (err) {
            console.log("Error reading images directory:", err);
            return res.status(500).send("Error reading images directory");
        }

        const fileStats = files.map(file => {
            const filePath = path.join(imagesDir, file);
            const stats = fs.statSync(filePath);
            return {
                file,
                createdAt: stats.birthtime
            };
        });

        fileStats.sort((a, b) => b.createdAt - a.createdAt);
        const sortedFiles = fileStats.map(stat => stat.file);

        res.render("gallery", { images: sortedFiles });
    });
});

app.use("/", uploadRoutes);
app.use("/", alertRoutes);

app.listen(PORT, () => {
    console.log(`Server listening on port ${PORT}`);
});