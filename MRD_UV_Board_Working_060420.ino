//Libraries
#include <Wire.h>
#include <DS3231.h>

//Setting up time clock
DS3231 clock;
RTCDateTime realClock;

//defines
#define LEDU analogWrite(rLEDPin, red); analogWrite(gLEDPin, green);  analogWrite(bLEDPin, blue);
#define holdTime buttonPressTime
#define dt realClock
#define buttonU button=digitalRead(buttonPin); 
#define buttonBreakaway buttonU; if(button > 0) {lastButton = 1;}

//const ints for Pin Mode Setup
const int gLEDPin = 9;
const int bLEDPin = 10;
const int rLEDPin = 5;
const int relayPin = 3;
const int buttonPin = 2;
const int soundPin = 6;
const int motionPin = 7;

//starting board int values
int red = 0;
int green = 0;
int blue = 0;
int relay = LOW;
int sound = 0;
int tonerL = 200;
int tonerH = 800;
int bFlash = 0;

//Blue flashing light low and high values
int bHigh = 240;
int bLow = 50;
int rHigh = 240;
int rLow = 50;

//Button Press Ints
int lastButton = 0;
long int onTime = 0;
long int pressTime = 0;
long int releaseTime = 0;
int buttonPressTime = 4;
int doubleTime = 500;
int hold = 0;
int button = 0;

//Saftey Check Variables
int motion = 0;
int delayTime = 5;
int numberOfTrips = (30/delayTime);
int countUpSensorCheck = 0;
int countUpStartUVCycle = 0;
int sanitationTime = 15; //in Minutes

void setup() {
  //begin Serial Communication
  Serial.begin(9600);

  //Set Date and Time and Report
  clock.begin();
  clock.setDateTime(__DATE__, __TIME__);
  dt = clock.getDateTime();
  Serial.print("Boot Date: ");
  Serial.print(dt.year);   Serial.print("-");
  Serial.print(dt.month);  Serial.print("-");
  Serial.print(dt.day);    Serial.print(" ");
  Serial.print(dt.hour);   Serial.print(":");
  Serial.print(dt.minute); Serial.print(":");
  Serial.print(dt.second); Serial.println("");

  //Set Pin Modes
  pinMode(gLEDPin, OUTPUT);
  pinMode(rLEDPin, OUTPUT);
  pinMode(bLEDPin, OUTPUT);
  pinMode(relayPin, OUTPUT);
  pinMode(soundPin, OUTPUT);
  pinMode(buttonPin, INPUT);
  pinMode(motionPin, INPUT);

  //Test Cycle for Colors
  red = 255;
  LEDU;
  delay(500);
  red = 0;
  green = 255;
  LEDU;
  delay(500);
  green = 0;
  blue = 255;
  LEDU;
  delay(500);
  blue = 0;
  LEDU;

  //Test Cycle for Buzzer
  tone(soundPin, tonerL, 1000);
  delay(1100);


  //Post Startup holding state
  red = 0;
  green = 100;
  blue = 0;
  LEDU;
}

void loop() {

  //rereading the clock
  dt = clock.getDateTime(); 

  //waiting for button press
  buttonU;
  onTime = dt.unixtime;
  delay(5);

  //First Button Press  
  if (button == 1 && lastButton == 0) {
    pressTime = onTime;
    lastButton = 1;
  }
  
  //Held
  if (button == 1 && lastButton == 1) {
    if ((onTime - pressTime) > holdTime) {
      hold = 1;
      tone(soundPin, tonerL);
      red = 150;
      green = 0;
    }
      else {
        tone(soundPin, tonerH);
        red = 150;
        green = 50;
        LEDU;
      }
    }
    LEDU;

  //Release
  if (button == 0 && lastButton == 1) {
    if (hold != 1) {
      noTone(soundPin);
      red = 0 ;
      green = 100;
      lastButton = 0;
      LEDU;
    }
    else {
      Serial.println("Beginning UVC Clean Startup");
      red = 0;
      green = 0;
      blue = 255;
      hold = 0;
      lastButton = 0;
      releaseTime = dt.unixtime;
      LEDU;
      startUVCycle();
    }
  }
}


void startUVCycle(){
  if (countUpSensorCheck != numberOfTrips){
    delay(100);
      do {
        for (blue = 255; blue >= 05; blue--) {
          tone(soundPin, tonerH);
          LEDU;
          buttonU;
          buttonBreakaway;
          delay(2);
        }
        for (blue = 05; blue <= 254; blue++) {
          tone(soundPin, tonerL);
          LEDU;
          buttonU;
          buttonBreakaway;
          delay(2);
          }
        if(lastButton == 1){
          break;
        }
        else{
        countUpStartUVCycle++;
        }
      }
      while (countUpStartUVCycle != delayTime);
      if(lastButton == 1){
        cancledUVCycle();
      }
      else {
        noTone(soundPin);
        sensorCheck();
      }
  }
  else {
    cancledUVCycle();
  }
}

void sensorCheck() {
  //reset Int from previous
  countUpStartUVCycle = 0;

int safteyTrigger = 0;

  //beging dectection loop
  for (long int upperTen = 0; upperTen <= 7000; upperTen++) {
    buttonU;
    motion = digitalRead(motionPin);
    if(motion == 1){
      safteyTrigger = 1;
    }
    if(button == 1){
      safteyTrigger = 1;
    }
    delay(1);
  }
  if(safteyTrigger != 1){
    lastButton = 0;
    motion = 0;
    uvLampStrike();
    }
  else {
    countUpSensorCheck++;
    lastButton = 0;
    motion = 0;
    startUVCycle();
  }
}

void cancledUVCycle() {
  button = 0;
  button = digitalRead(buttonPin);
  while(button == 1){
    red = rLow;
    blue = 0;
    green = 0;
    LEDU;
    button = digitalRead(buttonPin);
    delay(500);
    red = rHigh;
    blue = 0;
    green = 0;
    LEDU;
    button = digitalRead(buttonPin);
    delay(500);
  }
  resetUnitToStart();
}
  
  //Rest Int
void resetUnitToStart(){
  button = 0;
  relay = LOW;
  digitalWrite(relayPin, relay);
  tone(soundPin, tonerL, 500);
  delay(1000);
  noTone(soundPin);
  motion = 0;
  red = 0;
  green = 100;
  blue = 0;
  LEDU;
  loop();
}


void uvLampStrike() {
  
  long int currentSanitationTime = 0;
  long int startSanitationTime = 0;
  long int sanitaionForm = (sanitationTime);
  int safteyTrigger = 0;
  long int countUp = 0;
  
  relay = HIGH;
  red = 255;
  blue = 0;
  green = 0;
  digitalWrite(relayPin, relay);
  LEDU;
  do {
    for (red = 255; red >= 05; red--) {
      buttonU;
      motion = digitalRead(motionPin);
      if(motion == 1){
        safteyTrigger = 1;
      }
      if(button == 1){
        safteyTrigger = 1;
      }
      LEDU;
      buttonU;
      buttonBreakaway;
      delay(8);
      }
    for (red = 05; red <= 254; red++) {
      buttonU;
      motion = digitalRead(motionPin);
      if(motion == 1){
        safteyTrigger = 1;
      }
      if(button == 1){
        safteyTrigger = 1;
      }
      LEDU;
      buttonU;
      buttonBreakaway;
      delay(8);
      if(safteyTrigger == 1){
          break;
      
      }
    }
    countUp++;
  }
  while (countUp != sanitaionForm);
  if(safteyTrigger == 1){
    cancledUVCycle();
  }
  else {
    resetUnitToStart();
  }
}
