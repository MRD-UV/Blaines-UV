//Error Codes//
//If motion is sensed more times that startUVCycle will allow
#define ERROR00 Serial.print("ERROR 00 - "); Serial.print(unitName); Serial.print(" has attempted to start a cleaning cycle "); Serial.print(numberOfTrips); Serial.print(" time(s). Start cleaning cycle again and evacuate room within "); Serial.print((((delayTime * numberOfTrips) * 60000) + (7000 * 3)) / 10000); Serial.println(" seconds of triggering a cleaning cycle. Please press and hold the button to confirm.");

//If button is pressed durring the startUVCycle
#define ERROR01 Serial.println("ERROR 01 - Button was pressed during the startup. Most likely the unit was accidentaly triggered and then cancled.");

//If button is pressed durring the sensorCheck
#define ERROR02 Serial.println("ERROR 02 - Button was pressed during the Sensor Check. Most likely the unit was accidentaly triggered and then cancled.");

//Once UV Lamp is struck, Motion is read
#define ERROR03 Serial.println("ERROR 03. Unit has detected motion. Cleaning Cycle has been canceled. Please press and hold the button to confirm.");

//Once UV Lamp is stuck, Button is pressed
#define ERROR04 Serial.println("ERROR 04. Unit has detected a button press. Cleaning Cycle has been canceled.");



//EDITABLE VARIABLES//
//Name of Device
char *unitName = {"Cabin 1422"};

//Buzzer Tone
int toneLow = 200;
int toneHigh = 800;

//Button Press Time
int buttonPressTime = 4;//Default value is 4. Must be >2

//Sensors Checks
int delayTime = 4; //in Seconds. This is the time to leave the room once button has been triggered =,
int numberOfTrips = 3;// default value is 3

//UV Lamp Strikes
int sanitationTime = 1; //in Minutes. default value is 15



//DO NOT EDIT BELOW THIS LINE
//-------------------------------------------------------------------------------------------------------

//Libraries
#include <Wire.h>
#include <DS3231.h>
#include <EEPROMex.h>

//Setting up time clock
DS3231 clock;
RTCDateTime dt;

//defines
#define LEDU analogWrite(rLEDPin, red); analogWrite(gLEDPin, green);  analogWrite(bLEDPin, blue);
#define timeAndDate Serial.print(dt.year); Serial.print("-"); Serial.print(dt.month); Serial.print("-"); Serial.print(dt.day); Serial.print(" "); Serial.print(dt.hour); Serial.print(":"); Serial.print(dt.minute); Serial.print(":"); Serial.println(dt.second);

//NON EDITABLE VARIABLES//
//const ints for Pin Mode Setup
const int gLEDPin = 9;
const int bLEDPin = 10;
const int rLEDPin = 5;
const int relayPin = 3;
const int buttonPin = 2;
const int soundPin = 6;
const int motionPin = 7;

//EEPROM Reading and Writing ints
long cycleCount;//Number of times Unit has run a UV Cleaning
int eepromCycleAddressStorage = 0; //Takes up 4 Bytes. Next avalible Storage location is 4


//starting board int values
int red = 0;
int green = 0;
int blue = 0;
int relay = LOW;
int sound = 0;

//Button Press Arrguments
int lastButton = 0;
long int timeFirst = 0;
long int timeSecond = 0;
long int timeThird = 0;
int hold = 0;
int button = 0;

//Saftey Check Variables
int motion = 0;
int countUpSensorCheck = 1;
int countUpStartUVCycle = 0;
int safteyTriggerMotion = 0;
int safteyTriggerButton = 0;

void setup() {
  //begin Serial Communication
  Serial.begin(9600);

  //Set Date and Time and Report
  clock.begin();
  clock.setDateTime(__DATE__, __TIME__);
  dt = clock.getDateTime();
  Serial.print(unitName);
  Serial.print(" - Booting at "); timeAndDate;

  //Update the Number of cycles ran from Local Memory
  delay(20);
  cycleCount = EEPROM.readLong(eepromCycleAddressStorage);
  delay(20);

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
  delay(250);
  red = 0;
  green = 255;
  LEDU;
  delay(250);
  green = 0;
  blue = 255;
  LEDU;
  delay(250);
  blue = 0;
  LEDU;

  //Test Cycle for Buzzer
  tone(soundPin, toneLow, 1000);
  delay(600);
  noTone(soundPin);

  //Post Startup holding state
  red = 0;
  green = 100;
  blue = 0;
  LEDU;
}

void loop() {
  button = digitalRead(buttonPin);
  delay(10);
  dt = clock.getDateTime();

  //First Button Press
  if (button == 1 && lastButton == 0) {
    timeSecond = dt.unixtime;
    delay(5);
    Serial.print(unitName);
    Serial.print(" - Button pressed at "); timeAndDate;
    lastButton = 1;
  }

  //Held
  if (button == 1 && lastButton == 1) {
    timeFirst = dt.unixtime;
    if ((timeFirst - timeSecond) >= buttonPressTime) {
      hold = 1;
      tone(soundPin, toneLow);
      red = 150;
      green = 0;
    }
    else {
      tone(soundPin, toneHigh);
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
      timeFirst = 0;
      timeSecond = 0;
      red = 0 ;
      green = 100;
      lastButton = 0;
      LEDU;
    }
    else {
      Serial.print(unitName);
      Serial.print(" - Beginning UVC Cycle");
      Serial.print(" at "); timeAndDate;
      timeFirst = 0;
      timeSecond = 0;
      red = 0;
      green = 0;
      blue = 255;
      hold = 0;
      timeThird = dt.unixtime;
      LEDU;
      startUVCycle();
    }
  }
}


void startUVCycle() {
  lastButton = 0;
  safteyTriggerButton = 0;

  //Check to see how many times the unit has tried to start up.
  if (countUpSensorCheck < (numberOfTrips + 1)) {

    //report number of attempts
    Serial.print(unitName); Serial.print(" - TN "); Serial.print(cycleCount);
    Serial.print(" - Checking Room Conditions. Attempt ");
    Serial.print(countUpSensorCheck);
    Serial.print(" of ");
    Serial.println(numberOfTrips);
    delay(100);

    //Warning light to exit the room
    do {

      //LED fades down
      for (blue = 255; blue >= 05; blue--) {//DO NOT CHANGE LED LEVELS
        tone(soundPin, toneHigh);
        LEDU;
        delay(2);//DO NOT CHANGE DELAY TIME

        //Check Button Status
        button = digitalRead(buttonPin);
        if (button > 0) {
          lastButton = 1;
        }

        //Check Breakaway Status from 'LED fades down' based on button
        if (lastButton == 1) {
          safteyTriggerButton = 1;
          break;
        }
      }

      //LED fades up
      for (blue = 05; blue <= 254; blue++) {//DO NOT CHANGE LED LEVELS
        tone(soundPin, toneLow);
        LEDU;
        delay(2);//DO NOT CHANGE DELAY TIME

        //Check Button Status
        button = digitalRead(buttonPin);
        if (button > 0) {
          lastButton = 1;
        }

        //Check Breakaway Status from 'LED fades up' based on button
        if (lastButton == 1) {
          break;
        }
      }

      //Check Breakaway Status from 'Warning Light to exit the room' based on button status
      if (safteyTriggerButton == 1) {
        break;
      }

      //Add one to the counter
      else {
        countUpStartUVCycle++;
      }
    }

    //Checking the counter vs the delay time. If it fails, return to the DO command
    while (countUpStartUVCycle != delayTime);
    //Again, checking the button status
    if (safteyTriggerButton == 1) {
      countUpSensorCheck = 0;
      safteyTriggerButton - 0;
      Serial.print(unitName); Serial.print(" - TN "); Serial.print(cycleCount); Serial.print(" - ");
      ERROR01;
      resetUnitToStart();
    }

    //Move to the next step and check sensors
    else {
      sensorCheck();
    }
  }

  //If unit has tried to start a cleanning cycle too many times it call this attempt a failure and cancled the cleaning cycle
  else {
    countUpSensorCheck = 0;
    noTone(soundPin);
    ERROR00;
    cancledUVCycle();
  }
}

void sensorCheck() {
  //reset Int from previous
  countUpStartUVCycle = 0;
  int upperTen = 0;

  //beging dectection loop
  for (upperTen = 0; upperTen <= 7000; upperTen++) {
    button = digitalRead(buttonPin);
    motion = digitalRead(motionPin);
    if (motion == 1) {
      safteyTriggerMotion = 1;
    }
    if (button == 1) {
      safteyTriggerButton = 1;
    }
    delay(1);
    if (safteyTriggerButton == 1) {
      break;
    }
  }
  if (safteyTriggerButton == 1) {
    upperTen = 0;
    Serial.print(unitName); Serial.print(" - TN "); Serial.print(cycleCount); Serial.print(" - ");
    ERROR02;
    noTone(soundPin);
    resetUnitToStart();
  }
  else {
    //If motion has not been sensed
    if (safteyTriggerMotion != 0) {
      countUpSensorCheck++;
      lastButton = 0;
      safteyTriggerMotion = 0;
      motion = 0;
      startUVCycle();
    }
    //If motion has been sensed
    else {
      countUpSensorCheck = 0;
      lastButton = 0;
      motion = 0;
      safteyTriggerMotion = 0;
      noTone(soundPin);
      Serial.print(unitName); Serial.print(" - TN "); Serial.print(cycleCount); Serial.print(" - ");
      Serial.println("Room conditions are clear.");
      uvLampStrike();
    }
  }
}


void uvLampStrike() {
  //UV Lamp Strikes
  long int sanitationForm = ((sanitationTime * 60) / 4);
  long int countUp = 0;

  //Serial Print Commands
  Serial.print(unitName); Serial.print(" - TN "); Serial.print(cycleCount); Serial.print(" - ");
  Serial.print("UV Lamp Relay ON. Please stand clear for ");
  Serial.print(sanitationTime); Serial.println(" minutes.");

  relay = HIGH;
  red = 255;
  blue = 0;
  green = 0;

  digitalWrite(relayPin, relay);
  LEDU;

  do {
    for (red = 255; red >= 05; red--) {//DONT CHANGE RED LED VALUES
      button = digitalRead(buttonPin);
      motion = digitalRead(motionPin);
      if (motion == 1) {
        safteyTriggerMotion = true;
      }
      if (button == 1) {
        safteyTriggerButton = true;
      }
      LEDU;
      delay(8);//DONT CHANGE DELAY VALUE
      if (safteyTriggerMotion + safteyTriggerButton >= 1) {
        break;
      }
    }
    for (red = 05; red <= 254; red++) {//DONT CHANGE RED LED VALUES
      button = digitalRead(buttonPin);
      motion = digitalRead(motionPin);
      if (motion == 1) {
        safteyTriggerMotion = 1;
      }
      if (button == 1) {
        safteyTriggerButton = 1;
      }
      LEDU;
      delay(8);//DONT CHANGE DELAY VALUE
      if (safteyTriggerMotion + safteyTriggerButton >= 1) {
        break;
      }
    }
    if (safteyTriggerMotion + safteyTriggerButton >= 1) {
      countUp = sanitationForm;
    }
    else {
      countUp++;
    }
  }
  while (countUp < sanitationForm);

  if (safteyTriggerMotion == 1) {
    countUp = 0;
    safteyTriggerMotion = 0;
    motionFault();
  }
  else if (safteyTriggerButton == 1) {
    countUp = 0;
    safteyTriggerButton = 0;
    buttonFault();
  }
  else {
    relay = LOW;
    digitalWrite(relayPin, relay);
    Serial.print(unitName); Serial.print(" - TN "); Serial.print(cycleCount); Serial.print(" - ");
    Serial.print("UV Cleaning Cycle was completed without error at ");
    Serial.print(dt.year);   Serial.print("-");
    Serial.print(dt.month);  Serial.print("-");
    Serial.print(dt.day);    Serial.print(" ");
    Serial.print(dt.hour);   Serial.print(":");
    Serial.print(dt.minute); Serial.print(":");
    Serial.print(dt.second); Serial.println("");
    resetUnitToStart();
  }
}

void motionFault() {
  ERROR03;
  relay = LOW;
  digitalWrite(relayPin, relay);
  tone(soundPin, toneHigh);
  delay(4000);
  noTone(soundPin);
  delay(500);
  cancledUVCycle();
}

void buttonFault() {
  ERROR04;
  relay = LOW;
  digitalWrite(relayPin, relay);
  tone(soundPin, toneHigh);
  delay(2000);
  noTone(soundPin);
  delay(500);
  resetUnitToStart();
}



void cancledUVCycle() {
  button = 0;
  int hourGlass = 0;
  do {
    for (hourGlass = 0; hourGlass <= 500; hourGlass++) {
      red = 50;
      blue = 0;
      green = 0;
      LEDU;
      button = digitalRead(buttonPin);
      if (button == 1) {
        break;
      }
      delay(1);
    }
    for (hourGlass = 500; hourGlass >= 0; hourGlass--) {
      red = 240;
      blue = 0;
      green = 0;
      LEDU;
      button = digitalRead(buttonPin);
      if (button == 1) {
        break;
      }
      delay(1);
    }
    button = digitalRead(buttonPin);
  } while (button != 1);

  //beep beep
  tone(soundPin, toneHigh);
  delay(100);
  noTone(soundPin);
  delay(100);
  tone(soundPin, toneHigh);
  delay(100);
  noTone(soundPin);
  delay(100);
  Serial.print("TN ");
  Serial.print(cycleCount);
  Serial.println(" cancled. Returning to Standby Mode");
  delay(1000);
  resetUnitToStart();
}


void resetUnitToStart() {
  countUpStartUVCycle = 0;
  button = 0;
  relay = LOW;
  digitalWrite(relayPin, relay);
  tone(soundPin, toneLow, 500);
  delay(1000);
  noTone(soundPin);
  motion = 0;
  red = 0;
  green = 100;
  blue = 0;
  LEDU;

  //Update Cycle Count to EEPROM
  delay(20);
  cycleCount++;
  EEPROM.writeLong(eepromCycleAddressStorage, cycleCount++);
  delay(20);
  
  countUpSensorCheck = 0;
  Serial.println(" ");
  Serial.println(" ");
  loop();
}
