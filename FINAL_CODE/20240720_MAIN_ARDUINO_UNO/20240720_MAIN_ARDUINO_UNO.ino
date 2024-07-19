#include <Keypad.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

const int solenoidDoor = 11;
const int buzzerPin = 10;
const int ROW_NUM = 4;
const int COLUMN_NUM = 4;

SoftwareSerial mySerial(12, 13);
LiquidCrystal_I2C lcd(0x27, 16, 2);

char keys[ROW_NUM][COLUMN_NUM] = {
  {'1','2','3', 'A'},
  {'4','5','6', 'B'},
  {'7','8','9', 'C'},
  {'*','0','#', 'D'}
};

byte pin_rows[ROW_NUM] = {9, 8, 7, 6};
byte pin_column[COLUMN_NUM] = {5, 4, 3, 2};

Keypad keypad = Keypad(makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM);

String password;
String input_password;
String masked_password;
String new_password;
String confirm_password;

int failedAttempts = 0;
int lockoutCounter = 0;
unsigned long lockoutEndTime = 0;

enum State {
  NORMAL,
  ENTER_CURRENT_PASSWORD,
  ENTER_NEW_PASSWORD,
  CONFIRM_NEW_PASSWORD,
  LOCKED_OUT,
  SET_INITIAL_PASSWORD
};

State state = NORMAL;

const int PASSWORD_ADDRESS = 0;

void savePasswordToEEPROM(String pass) {
  for (int i = 0; i < pass.length(); i++) {
    EEPROM.write(PASSWORD_ADDRESS + i, pass[i]);
  }
  EEPROM.write(PASSWORD_ADDRESS + pass.length(), '\0');
}

String readPasswordFromEEPROM() {
  char pass[32];
  int i = 0;
  while (true) {
    pass[i] = EEPROM.read(PASSWORD_ADDRESS + i);
    if (pass[i] == '\0') break;
    i++;
  }
  return String(pass);
}

void setup() {
  Serial.begin(115200);
  mySerial.begin(115200);

  lcd.init();
  lcd.backlight();

  input_password.reserve(32);
  masked_password.reserve(32);
  new_password.reserve(32);
  confirm_password.reserve(32);
  pinMode(solenoidDoor, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  password = readPasswordFromEEPROM();
  if (password.length() == 0) {
    lcd.setCursor(0, 0);
    lcd.print("Set New Password:");
    state = SET_INITIAL_PASSWORD;
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Enter Password:");
    state = NORMAL;
  }
}

void loop() {
  char key = keypad.getKey();
  if (mySerial.available() > 0) {
    String serialDoor = mySerial.readStringUntil('\n');
    serialDoor.trim();

    if (serialDoor == "MATCHFACE") {
      digitalWrite(solenoidDoor, HIGH);
      delay(2000);
      digitalWrite(solenoidDoor, LOW);
    }
  }

  if (state == LOCKED_OUT) {
    unsigned long remainingTime = (lockoutEndTime - millis()) / 1000;
    lcd.setCursor(0, 1);
    lcd.print("Lockout: " + String(remainingTime) + " sec ");

    if (millis() > lockoutEndTime) {
      state = NORMAL;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Enter Password:");
    }
    return;
  }

  if (key) {
    tone(buzzerPin, 2000, 100);
    Serial.println(key);

    if (key == '*') {
      if (input_password.length() > 0) {
        input_password.remove(input_password.length() - 1);
        masked_password.remove(masked_password.length() - 1);
        lcd.setCursor(0, 1);
        lcd.print(masked_password + " ");
      }
    } else if (key == '#') {
      if (state == NORMAL) {
        if (password == input_password) {
          Serial.println("Password is correct");
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Access Granted");
          tone(buzzerPin, 2000, 500);
          digitalWrite(solenoidDoor, HIGH);
          delay(2000);
          digitalWrite(solenoidDoor, LOW);
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Enter Password:");
          failedAttempts = 0;
          lockoutCounter = 0;
        } else {
          Serial.println("Password is incorrect, try again");
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Wrong Password");
          tone(buzzerPin, 1000, 500);
          failedAttempts++;
          if (failedAttempts >= 3) {
            lockoutCounter++;
            state = LOCKED_OUT;
            lockoutEndTime = millis() + (5000 + 10000 * (lockoutCounter - 1));
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Locked Out");
            tone(buzzerPin, 1000);
            delay(1000);
            noTone(buzzerPin);
          }
          delay(2000);
          if (state != LOCKED_OUT) {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Enter Password:");
          }
        }
        input_password = "";
        masked_password = "";
      } else if (state == ENTER_CURRENT_PASSWORD) {
        if (password == input_password) {
          Serial.println("Enter new password");
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("New Password:");
          state = ENTER_NEW_PASSWORD;
        } else {
          Serial.println("Current password is incorrect");
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Wrong Password");
          tone(buzzerPin, 1000, 500);
          delay(2000);
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Enter Password:");
          state = NORMAL;
        }
        input_password = "";
        masked_password = "";
      } else if (state == ENTER_NEW_PASSWORD) {
        new_password = input_password;
        Serial.println("Confirm new password");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Confirm Password:");
        state = CONFIRM_NEW_PASSWORD;
        input_password = "";
        masked_password = "";
      } else if (state == CONFIRM_NEW_PASSWORD) {
        confirm_password = input_password;
        if (new_password == confirm_password) {
          password = new_password;
          savePasswordToEEPROM(password);
          Serial.println("Password changed successfully");
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Password Changed");
          tone(buzzerPin, 2000, 500);
          delay(2000);
        } else {
          Serial.println("Passwords do not match, try again");
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Mismatch, Try Again");
          tone(buzzerPin, 1000, 500);
          delay(2000);
        }
        state = NORMAL;
        new_password = "";
        confirm_password = "";
        input_password = "";
        masked_password = "";
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Enter Password:");
      } else if (state == SET_INITIAL_PASSWORD) {
        new_password = input_password;
        Serial.println("Confirm new password");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Confirm Password:");
        state = CONFIRM_NEW_PASSWORD;
        input_password = "";
        masked_password = "";
      }
    } else if (key == 'D') {
      input_password = "";
      masked_password = "";
      new_password = "";
      confirm_password = "";
      if (state == SET_INITIAL_PASSWORD) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Set New Password:");
      } else {
        state = NORMAL;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Enter Password:");
      }
    } else {
      input_password += key;
      masked_password += '*';
      lcd.setCursor(0, 1);
      lcd.print(masked_password + " ");
    }

    if (key == 'A' && state == NORMAL) {
      Serial.println("Enter current password to change password");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Current Password:");
      state = ENTER_CURRENT_PASSWORD;
      input_password = "";
      masked_password = "";
    }
  }
}