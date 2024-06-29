<?php
use PHPMailer\PHPMailer\PHPMailer;
use PHPMailer\PHPMailer\Exception;

require 'vendor/autoload.php';

date_default_timezone_set('Asia/Manila');
$target_dir = "captured_images/";
$date = new DateTime();
$date_string = $date->format('Y-m-d_His ');
$target_file = $target_dir . $date_string . basename($_FILES["imageFile"]["name"]);
$uploadOk = 1;
$imageFileType = strtolower(pathinfo($target_file, PATHINFO_EXTENSION));
$file_name = pathinfo($target_file, PATHINFO_BASENAME);

if (isset($_POST["imageFile"])) {
    $check = getimagesize($_FILES["imageFile"]["tmp_name"]);
    if ($check !== false) {
        echo "File is an image - " . $check["mime"] . ".";
        $uploadOk = 1;
    } else {
        echo "File is not an image.";
        $uploadOk = 0;
    }
}

if (file_exists($target_file)) {
    echo "Sorry, file already exists.";
    $uploadOk = 0;
}

if ($_FILES["imageFile"]["size"] > 500000) {
    echo "Sorry, your file is too large.";
    $uploadOk = 0;
}

if ($imageFileType != "jpg" && $imageFileType != "png" && $imageFileType != "jpeg" && $imageFileType != "gif") {
    echo "Sorry, only JPG, JPEG, PNG & GIF files are allowed.";
    $uploadOk = 0;
}

if ($uploadOk == 0) {
    echo "Sorry, your file was not uploaded.";
} else {
    if (move_uploaded_file($_FILES["imageFile"]["tmp_name"], $target_file)) {
        echo "Photos successfully uploaded to the server with the name : " . $file_name;

        // Determine email subject and body based on file name
        $subject = '';
        $body = '';

        if (strpos($file_name, 'MotionDetected.jpg') !== false) {
            $subject = 'Motion Detected - Alert';
            $body = 'Motion has been detected in the captured image.';
        } else if (strpos($file_name, 'IntruderAlert.jpg') !== false) {
            $subject = 'Intruder Alert';
            $body = 'An intruder has been detected in the captured image.';
        } else {
            $subject = 'New Photo Uploaded';
            $body = 'A new photo has been uploaded.';
        }

        // Send email with PHPMailer
        $mail = new PHPMailer(true);
        
        try {
            //Server settings
            $mail->isSMTP();
            $mail->Host       = 'smtp.gmail.com';
            $mail->SMTPAuth   = true;
            $mail->Username   = 'cotactearmenion@gmail.com';        
            $mail->Password   = 'eptg zrey kmju fnri'; 
            $mail->SMTPSecure = PHPMailer::ENCRYPTION_STARTTLS;
            $mail->Port       = 587;

            // Recipients
            $mail->setFrom('cotactearmenion@gmail.com', 'GROUP 1 - LFSA322N002');
            $mail->addAddress('cotactearmenion@gmail.com');
            $mail->addAddress('laurenceaguio08@gmail.com');
            $mail->addAddress('rawrrsusan@gmail.com');

            // Attach uploaded file
            $mail->addAttachment($target_file, $file_name); // Add attachments

            // Content
            $mail->isHTML(true); // Set email format to HTML
            $mail->Subject = $subject;
            $mail->Body    = $body;
            
            $mail->send();
            echo 'Email sent successfully.';
        } catch (Exception $e) {
            echo "Mailer Error: {$mail->ErrorInfo}";
        }
        
    } else {
        echo "Sorry, there was an error in the photo upload process.";
    }
}
?>
