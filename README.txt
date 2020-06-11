{\rtf1\ansi\ansicpg1252\cocoartf1671\cocoasubrtf100
{\fonttbl\f0\fswiss\fcharset0 Helvetica;}
{\colortbl;\red255\green255\blue255;}
{\*\expandedcolortbl;;}
\margl1440\margr1440\vieww10800\viewh8400\viewkind0
\pard\tx720\tx1440\tx2160\tx2880\tx3600\tx4320\tx5040\tx5760\tx6480\tx7200\tx7920\tx8640\pardirnatural\partightenfactor0

\f0\fs24 \cf0 //Error Codes//\
\
//If motion is sensed more times that startUVCycle will allow\
#define ERROR00 Serial.print("ERROR 00 - "); Serial.print(unitName); Serial.print(" has attempted to start a cleaning cycle "); Serial.print(numberOfTrips); Serial.print(" time(s). Start cleaning cycle again and evacuate room within "); Serial.print((((delayTime * numberOfTrips) * 60000) + (7000 * 3)) / 10000); Serial.println(" seconds of triggering a cleaning cycle. Please press and hold the button to confirm.");\
\
//If button is pressed durring the startUVCycle\
#define ERROR01 Serial.println("ERROR 01 - Button was pressed during the startup. Most likely the unit was accidentaly triggered and then cancled.");\
\
//If button is pressed durring the sensorCheck\
#define ERROR02 Serial.println("ERROR 02 - Button was pressed during the Sensor Check. Most likely the unit was accidentaly triggered and then cancled.");\
\
//Once UV Lamp is struck, Motion is read\
#define ERROR03 Serial.println("ERROR 03. Unit has detected motion. Cleaning Cycle has been canceled. Please press and hold the button to confirm.");\
\
//Once UV Lamp is stuck, Button is pressed\
#define ERROR04 Serial.println("ERROR 04. Unit has detected a button press. Cleaning Cycle has been canceled. Please press and hold the button to confirm.");\
}