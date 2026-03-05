#include <LiquidCrystal.h>

volatile int flowPulseCount = 0;
float flowRate;
unsigned int totalMilliLitres;
unsigned long oldTime;

const int flowSensorPin = 2;    // Flow sensor signal pin
const int moisturePin = A2;     // Soil moisture analog pin
int wat_level =  0;
int moistureValue = 0;

LiquidCrystal lcd(7, 6, 5, 4, 3, 8); // RS, E, D4, D5, D6, D7

#include <SoftwareSerial.h>
SoftwareSerial gsmSerial(10, 11);

//Buzzer
int buzzer = 13;
int relay = 12;

String STATUS;
String waterLevelStatus;

int count = 0;

void setup() {
  pinMode(flowSensorPin, INPUT_PULLUP);
  pinMode(buzzer, OUTPUT);
  pinMode(relay, OUTPUT);
  
  Serial.begin(9600);
  gsmSerial.begin(9600);
  
  attachInterrupt(digitalPinToInterrupt(flowSensorPin), pulseCounter, RISING);

  lcd.begin(16, 2);
  lcd.setCursor(0,0);
  lcd.print(" WATER FLOW ");
  lcd.setCursor(0,1);
  lcd.print(" MONITOIRNG ");
  delay(1800);
  digitalWrite(buzzer, HIGH);
   
  oldTime = millis();
  delay(200);
  digitalWrite(buzzer, LOW);
  lcd.clear();
}

void loop() {
  if (millis() - oldTime > 1000) {
    detachInterrupt(digitalPinToInterrupt(flowSensorPin));

    // Flow calculation
    flowRate = ((1000.0 / (millis() - oldTime)) * flowPulseCount) / 7.5;
    totalMilliLitres += (flowRate / 60) * 1000;

    oldTime = millis();
    flowPulseCount = 0;

    // Read soil moisture
    moistureValue = analogRead(moisturePin);
    wat_level = ( 100 - ( (moistureValue/1024.00) * 100 ) );
    
    waterLevelStatus = "LOW";
    attachInterrupt(digitalPinToInterrupt(flowSensorPin), pulseCounter, RISING);
  }

   // Display on LCD
    lcd.setCursor(0, 0);
    lcd.print("WF:");
    lcd.print(flowRate, 1);
    lcd.print(" L/m");

    lcd.setCursor(0, 1);
    lcd.print("WL:");
    lcd.print(wat_level);
    lcd.print("  ");

    // Debugging
    Serial.print("Flow rate: ");
    Serial.print(flowRate);
    Serial.print(" L/min\tTotal: ");
    Serial.print(totalMilliLitres);
    Serial.print(" mL\tWater Level: ");
    Serial.println(waterLevelStatus);

  
  if(flowRate > 1){
    digitalWrite(relay, HIGH);
    STATUS  = "WATER FLOW ON";
    count++;
    if(count == 1){
      gsm_msg();
    }
  }
  else  if (wat_level < 20) {
      digitalWrite(relay, LOW);
      waterLevelStatus = "OK ";
      STATUS  = "WATER LEVEL LOW";
      digitalWrite(relay, LOW);
      count++;
      if(count == 1){
        gsm_msg();
      }
    }
   else{
      count = 0;
   }
   delay(500);

}

void pulseCounter() {
  flowPulseCount++;
}

void gsm_msg(){
    lcd.setCursor(8,1);
    lcd.print("Sending.");
    delay(100);
    gsmSerial.println("AT");
    delay(500);
    gsmSerial.println("AT+CMGF=1");    //To send SMS in Text Mode
    delay(500);
    gsmSerial.println("AT+CMGS=\"+917904293984\"\r"); // change to the phone number you using 
    delay(500);
    lcd.setCursor(8,1);
    lcd.print("Sendi...");
    gsmSerial.println("Status: ");//the content of the message
    gsmSerial.print(STATUS);
    //gsmSerial.println();
  
    delay(500);
    lcd.setCursor(8,1);
    lcd.print("Send....");
    delay(500);
    digitalWrite(buzzer, HIGH);
    gsmSerial.println((char)26);//the stopping character
    delay(500);
    digitalWrite(buzzer, LOW);
    lcd.setCursor(8,1);
    lcd.print("SENT...     ");
    delay(500);
    lcd.setCursor(8,1);
    lcd.print("            ");
}
