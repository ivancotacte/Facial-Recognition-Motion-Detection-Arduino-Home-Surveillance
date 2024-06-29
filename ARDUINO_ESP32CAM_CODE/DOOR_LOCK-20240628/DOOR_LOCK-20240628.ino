#include <Keypad.h>
#include <EEPROM.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>

#define RXD2 12
#define TXD2 13
#define BUZZER 10
#define DOOR_RELAY 11

SoftwareSerial mySerial(RXD2, TXD2);

const int ROW_NUM = 4;
const int COLUMN_NUM = 4;

char keys[ROW_NUM][COLUMN_NUM] = {
  {'1','2','3', 'A'},
  {'4','5','6', 'B'},
  {'7','8','9', 'C'},
  {'*','0','#', 'D'}
};

byte pin_rows[ROW_NUM] = {9, 8, 7, 6};
byte pin_column[COLUMN_NUM] = {5, 4, 3, 2};

Keypad keypad = Keypad(makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM);

String passcode;
String input_passcode;
String new_passcode;
String current_passcode;

enum State {
  NORMAL,
  RESET_PASSCODE_ENTER_CURRENT,
  RESET_PASSCODE_ENTER_NEW,
  RESET_PASSCODE_CONFIRM_NEW,
  SET_DOOR_DURATION
};

State currentState = NORMAL;

LiquidCrystal_I2C lcd(0x27, 16, 2);

bool doorRelayActive = false;
unsigned long prevMillis = 0;
const int interval = 5000;

unsigned long lastInputTime = 0;
const unsigned long inputTimeout = 15000; // 15 seconds

unsigned long doorOpenDuration = 5000; // Default 5 seconds

void setup() {
  Serial.begin(115200);
  mySerial.begin(115200);

  input_passcode.reserve(32);
  new_passcode.reserve(32);
  current_passcode.reserve(32);

  pinMode(BUZZER, OUTPUT);
  pinMode(DOOR_RELAY, OUTPUT);

  digitalWrite(DOOR_RELAY, LOW);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Enter Passcode:");

  loadPasscodeFromEEPROM();
  lastInputTime = millis();
}

void loop() {
  char key = keypad.getKey();

  if (millis() - lastInputTime > inputTimeout) {
    lcd.noBacklight(); // Dim the light if there's no input for 15 seconds
  }

  if (mySerial.available() > 0) {
    bool doorRelay = mySerial.read() == '1';

    if (doorRelay && !doorRelayActive) {
      doorRelayActive = true;
      digitalWrite(DOOR_RELAY, HIGH);
      lcd.setCursor(0, 1);
      lcd.print("Door Unlock      ");
      prevMillis = millis();
    }
  }

  if (doorRelayActive && millis() - prevMillis > interval) {
    doorRelayActive = false;
    digitalWrite(DOOR_RELAY, LOW);
    lcd.setCursor(0, 1);
    lcd.print("Door Lock     ");
    delay(2000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Enter Passcode:");
  }

  if (key) {
    lastInputTime = millis();
    lcd.backlight(); // Turn on the light when there is input

    Serial.println(key);
    tone(BUZZER, 1000, 100);
    delay(100);

    switch (currentState) {
      case NORMAL:
        if (key == '*') {
          input_passcode = "";
          lcd.setCursor(0, 1);
          lcd.print("                ");
        } else if (key == '#') {
          if (passcode == input_passcode) {
            Serial.println("passcode is correct");
            lcd.setCursor(0, 1);
            lcd.print("Access Granted");
            digitalWrite(DOOR_RELAY, HIGH);
            tone(BUZZER, 2000, 500);
            delay(doorOpenDuration);
            digitalWrite(DOOR_RELAY, LOW);
            input_passcode = "";
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Enter Passcode:");
          } else {
            Serial.println("passcode is incorrect, try again");
            lcd.setCursor(0, 1);
            lcd.print("Access Denied");
            for (int i = 0; i < 3; i++) {
              tone(BUZZER, 500, 100);
              delay(200);
            }
            input_passcode = "";
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Enter Passcode:");
          }
        } else if (key == 'A') {
          Serial.println("Enter current passcode:");
          lcd.setCursor(0, 0);
          lcd.print("Current Passcode:");
          current_passcode = "";
          currentState = RESET_PASSCODE_ENTER_CURRENT;
        } else if (key == 'B') {
          Serial.println("Enter duration in seconds:");
          lcd.setCursor(0, 0);
          lcd.print("Duration (s):");
          input_passcode = "";
          currentState = SET_DOOR_DURATION;
        } else {
          input_passcode += key;
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Enter Passcode:");
          lcd.setCursor(0, 1);
          lcd.print(input_passcode);
        }
        break;

      case RESET_PASSCODE_ENTER_CURRENT:
        if (key == '*') {
          current_passcode = "";
          lcd.setCursor(0, 1);
          lcd.print("                ");
        } else if (key == '#') {
          if (passcode == current_passcode) {
            Serial.println("Enter new passcode:");
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("New Passcode:   ");
            new_passcode = "";
            currentState = RESET_PASSCODE_ENTER_NEW;
          } else {
            Serial.println("Incorrect current passcode, try again");
            lcd.setCursor(0, 1);
            lcd.print("Incorrect");
            for (int i = 0; i < 3; i++) {
              tone(BUZZER, 500, 100);
              delay(200);
            }
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Enter Passcode: ");
            lcd.setCursor(0, 1);
            lcd.print("                ");
            currentState = NORMAL;
          }
        } else {
          current_passcode += key;
          lcd.setCursor(0, 1);
          lcd.print(current_passcode);
        }
        break;

      case RESET_PASSCODE_ENTER_NEW:
        if (key == '*') {
          new_passcode = "";
          lcd.setCursor(0, 1);
          lcd.print("                ");
        } else if (key == '#') {
          Serial.println("Confirm new passcode:");
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Confirm Passcode:");
          input_passcode = "";
          currentState = RESET_PASSCODE_CONFIRM_NEW;
        } else {
          new_passcode += key;
          lcd.setCursor(0, 1);
          lcd.print(new_passcode);
        }
        break;

      case RESET_PASSCODE_CONFIRM_NEW:
        if (key == '*') {
          input_passcode = "";
          lcd.setCursor(0, 1);
          lcd.print("                ");
        } else if (key == '#') {
          if (new_passcode == input_passcode) {
            passcode = new_passcode;
            savePasscodeToEEPROM();
            Serial.println("Passcode reset successful");
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Reset Successful");
            tone(BUZZER, 2000, 500);
            delay(1000);
            input_passcode = "";
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Enter Passcode:");
          } else {
            Serial.println("Passcodes do not match, try again");
            lcd.setCursor(0, 1);
            lcd.print("Not Matched     ");
            for (int i = 0; i < 3; i++) {
              tone(BUZZER, 500, 100);
              delay(200);
            }
            input_passcode = "";
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Enter Passcode:");
          }
          lcd.setCursor(0, 0);
          lcd.print("Enter Passcode: ");
          currentState = NORMAL;
        } else {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Confirm Passcode:");
          lcd.setCursor(0, 1);
          input_passcode += key;
          lcd.print(input_passcode);
        }
        break;

      case SET_DOOR_DURATION:
        if (key == '#') {
          doorOpenDuration = input_passcode.toInt() * 1000;
          lcd.setCursor(0, 1);
          lcd.print("Duration Set");
          delay(2000);
          input_passcode = "";
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Enter Passcode:");
          currentState = NORMAL;
        } else {
          input_passcode += key;
          lcd.setCursor(0, 1);
          lcd.print(input_passcode);
        }
        break;
    }
  }
}

void savePasscodeToEEPROM() {
  for (int i = 0; i < passcode.length(); i++) {
    EEPROM.write(i, passcode[i]);
  }
  EEPROM.write(passcode.length(), '\0');
}

void loadPasscodeFromEEPROM() {
  char stored_passcode[32];
  int i;
  for (i = 0; i < sizeof(stored_passcode); i++) {
    stored_passcode[i] = EEPROM.read(i);
    if (stored_passcode[i] == '\0') break;
  }
  stored_passcode[i] = '\0';
  passcode = String(stored_passcode);
}