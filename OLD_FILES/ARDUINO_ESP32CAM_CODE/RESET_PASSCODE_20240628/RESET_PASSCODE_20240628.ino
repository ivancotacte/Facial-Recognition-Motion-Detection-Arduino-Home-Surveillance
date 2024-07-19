#include <EEPROM.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

#define BUZZER 10

LiquidCrystal_I2C lcd(0x27, 16, 2);
String new_password;

void setup() {
  Serial.begin(115200);

  pinMode(BUZZER, OUTPUT);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Reset Passcode:");

  clearEEPROM();

  new_password = "1234";
  savePasswordToEEPROM(new_password);

  lcd.setCursor(0, 1);
  lcd.print("New Pass: 1234");
  digitalWrite(BUZZER, HIGH);
  delay(100);
  digitalWrite(BUZZER, LOW);
}

void loop() {
}

void clearEEPROM() {
  for (int i = 0; i < EEPROM.length(); i++) {
    EEPROM.write(i, 0);
  }
}

void savePasswordToEEPROM(String password) {
  for (int i = 0; i < password.length(); i++) {
    EEPROM.write(i, password[i]);
  }
  EEPROM.write(password.length(), '\0');
}
