<?php
use PHPMailer\PHPMailer\PHPMailer;
use PHPMailer\PHPMailer\Exception;

require 'vendor/autoload.php';

if ($_SERVER["REQUEST_METHOD"] == "POST") {
    $subject = trim($_POST['subject']);
    $message = trim($_POST['message']);
    date_default_timezone_set('Asia/Manila');
    $currentDateTime = date("Y-m-d H:i:s");

    $mail = new PHPMailer(true);

    try {
        // Server settings
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

        // Content
        $mail->isHTML(true);
        $mail->Subject = $subject;
        $mail->Body    = "<p>Date and Time: {$currentDateTime}</p><p>{$message}</p>";

        $mail->send();
        echo 'Message has been sent successfully';
    } catch (Exception $e) {
        echo "Message could not be sent. Mailer Error: {$mail->ErrorInfo}";
    }
}
?>
