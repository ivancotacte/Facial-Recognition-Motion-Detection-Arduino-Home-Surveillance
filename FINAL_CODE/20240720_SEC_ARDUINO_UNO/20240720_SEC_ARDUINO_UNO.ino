int buzzerPin = 13;
int pirsensorPin = 2;
int pirState = LOW;
int val = 0;

void setup() {
  pinMode(buzzerPin, OUTPUT);
  pinMode(pirsensorPin, INPUT);
  Serial.begin(9600);
}

void loop() {
  val = digitalRead(pirsensorPin);
  if (val == HIGH) {
    digitalWrite(buzzerPin, HIGH);
    if (pirState == LOW) {
      Serial.println("Motion detected!");
      pirState = HIGH;
    }
  } else {
    digitalWrite(buzzerPin, LOW);
    if (pirState == HIGH) {
      Serial.println("Motion ended!");
      pirState = LOW;
    }
  }
}