#include <SoftwareSerial.h>
const int PIN_OXYGEN_SENSOR = A0;
const int PIN_LIQUID_LEVEL_SENSOR = 2;
const int PIN_LIGHTING = 3;
const float OXYGEN_THRESHOLD = 15.0;
const int READ_INTERVAL = 1000;
SoftwareSerial gsmSerial(10, 11);
bool smsSent = false;

float calculateOxygenPercentage(int sensorValue) {
  float voltage = sensorValue * (5.0 / 1023.0);
  return (voltage / 5.0) * 21.0;
}

void sendSMS(String message) {
  gsmSerial.println("AT");
  delay(100);
  gsmSerial.println("AT+CMGF=1");
  delay(100);
  gsmSerial.println("AT+CMGS=\"+1234567890\"");
  delay(100);
  gsmSerial.print(message);
  delay(100);
  gsmSerial.write(26);
  delay(1000);
}

void setup() {
  Serial.begin(9600);
  gsmSerial.begin(9600);
  pinMode(PIN_OXYGEN_SENSOR, INPUT);
  pinMode(PIN_LIQUID_LEVEL_SENSOR, INPUT_PULLUP);
  pinMode(PIN_LIGHTING, OUTPUT);
  digitalWrite(PIN_LIGHTING, LOW);
  Serial.println("Start system 'Chlorella'");
}

void updateLighting(bool liquidLevelOk, float oxygenPercentage) {
  if (liquidLevelOk) {
    if (oxygenPercentage < OXYGEN_THRESHOLD) {
      digitalWrite(PIN_LIGHTING, HIGH);
      delay(200);
      digitalWrite(PIN_LIGHTING, LOW);
      delay(200);
    } else {
      digitalWrite(PIN_LIGHTING, HIGH);
    }
  } else {
    digitalWrite(PIN_LIGHTING, LOW);
  }
}

void loop() {
  int oxygenRaw = analogRead(PIN_OXYGEN_SENSOR);
  float oxygenPercentage = calculateOxygenPercentage(oxygenRaw);
  int liquidLevelState = digitalRead(PIN_LIQUID_LEVEL_SENSOR);
  bool isLiquidLevelOk = (liquidLevelState == HIGH);
  Serial.print("Oxygen: ");
  Serial.print(oxygenPercentage, 2);
  Serial.print("%, Raw: ");
  Serial.print(oxygenRaw);
  Serial.print(" | Liquid level: ");
  Serial.println(isLiquidLevelOk ? "OK" : "Low");
  
  if (oxygenPercentage < OXYGEN_THRESHOLD && !smsSent) {
    String smsMessage = "Warning: Oxygen level low: " + String(oxygenPercentage, 2) + "%";
    sendSMS(smsMessage);
    smsSent = true;
  }
  if (oxygenPercentage >= OXYGEN_THRESHOLD) {
    smsSent = false;
  }
  
  updateLighting(isLiquidLevelOk, oxygenPercentage);
  delay(READ_INTERVAL);
}
