// Physical Computing Final Project by Tino Selic

// For handling software debouncing of mechanical switch contacts
#include <Bounce2.h>

// Libraries for RTC
#include <Wire.h>
#include <RTClib.h>

// Define an object of RTC_DS1307 class
RTC_DS1307 RTC;

// Maximum number of digits that can be dialled (Time: hh:mm = 4), must be static
#define MAXDIGITS 4

// Pins
int idlePin = 2;    // Handset
int dialPin = 10;   // Control of rotary-dial
int numberPin = 9;  // Number dial
int ringerPins[] = { 3, 4 };

// Modes
int mode = 0;

// How many pulses have been detected for the dialled number
int pulseCount = 0;

// The dialled number
int number = 0;

// Alarm variables
bool alarmValid = false;   // Check for correctly dialled time format
bool alarmActive = false;  // State of the alarm (on/off)
int alarmDigit = 0;
int alarmTime[4];
int alarmHours;
int alarmMinutes;

// Time interval between ringing; ringtone
unsigned long lastRingTime;

// Bounce objects
Bounce idleSwitch = Bounce();
Bounce dialSwitch = Bounce();
Bounce numberSwitch = Bounce();

void setup() {
  // Open the serial port
  Serial.begin(9600);
  // Print out test
  Serial.println("Dial: 0-Idle, 1-Alarm, 2-Light, 3-Mode 3,..., 9-Mode 9");
  // Declare pin inputs and attach debounce ojects
  pinMode(idlePin, INPUT_PULLUP);
  idleSwitch.attach(idlePin);
  idleSwitch.interval(5);

  pinMode(dialPin, INPUT_PULLUP);
  dialSwitch.attach(dialPin);
  dialSwitch.interval(5);

  pinMode(numberPin, INPUT_PULLUP);
  numberSwitch.attach(numberPin);
  numberSwitch.interval(5);

  // Set the current time and write it to the RTC chip
  RTC.begin();
  RTC.adjust(DateTime(__DATE__, __TIME__));
  if (!RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }
}

void loop() {
  // Read the current state of all switches
  idleSwitch.update();
  dialSwitch.update();
  numberSwitch.update();

  // Print out the state of the handset
  if (idleSwitch.fell()) {
    Serial.println("Handset lifted");
  }
  if (idleSwitch.rose()) {
    Serial.println("Handset placed");
  }

  DateTime now = RTC.now();
  // Serial.print("Date: ");
  // Serial.print(now.day(), DEC);
  // Serial.print('/');
  // Serial.print(now.month(), DEC);
  // Serial.print('/');
  // Serial.print(now.year(), DEC);
  // Serial.print("  ");
  // Serial.print("Time: ");
  // Serial.print(now.hour(), DEC);
  // Serial.print(':');
  // Serial.print(now.minute(), DEC);
  // Serial.print(':');
  // Serial.print(now.second(), DEC);
  // Serial.println();
  // Serial.print("-------------------------------------");
  // Serial.println();
  // delay(1000);

  switch (mode) {
    // Idle
    case 0:
      // Time-alarm comparison
      if (alarmActive && now.hour() == alarmHours && now.minute() == alarmMinutes && now.second() == 0) {
        Serial.println("Alarm ringing");
        mode = 10;
      }
      dialling();
      // Switch the mode by lifting the handset
      if (idleSwitch.fell()) {
        Serial.print("Mode ");
        Serial.println(number);
        mode = number;
      }
      break;

    // Alarm Mode
    case 1:
      number = 0;
      // Dialling
      if (numberSwitch.rose()) {
        pulseCount++;
      }
      // Check whether the dial has returned all the way
      if (dialSwitch.rose()) {
        // The number 0 has 10 pulses
        if (pulseCount == 10) {
          pulseCount = 0;
        }
        number = pulseCount;
        pulseCount = 0;
        // Check, if the dialled numbers are valid/invalid for the specific time digit (00:00 ... 23:59)
        // switch (alarmDigit) {
        //   case 0:
        //     if (number > 2) {
        //       Serial.println("0: Invalid number. Please dial the time in 24h-format (hh:mm).");
        //       alarmDigit = 0;
        //     } else {
        //       alarmTime[0] = number;
        //       Serial.println("Set alarm in 24h-format: ");
        //       Serial.print(alarmTime[0]);
        //     }
        //     break;
        //   case 1:
        //     if (alarmTime[0] == 2 && number > 3) {
        //       Serial.println("1: Invalid number. Please dial the time in 24h-format (hh:mm).");
        //       alarmDigit = 0;
        //     } else {
        //       alarmTime[1] = number;
        //       Serial.print(alarmTime[1]);
        //       Serial.print(":");
        //     }
        //     break;
        //   case 2:
        //     if (number > 5) {
        //       Serial.println("2: Invalid number. Please dial the time in 24h-format (hh:mm).");
        //       alarmDigit = 0;
        //     } else {
        //       alarmTime[2] = number;
        //       Serial.print(alarmTime[2]);
        //     }
        //     break;
        //   case 3:
        //     alarmTime[3] = number;
        //     Serial.println(alarmTime[3]);
        //     break;
        //   default:
        //     mode = 0;
        // }
        alarmTime[alarmDigit] = number;
        Serial.println("Set alarm in 24h-format: ");
        Serial.print(alarmTime[alarmDigit]);
        alarmDigit++;
        if (alarmDigit == 4) {
          if (alarmTime[0] > 2 || alarmTime[0] == 2 && alarmTime[1] > 3 || alarmTime[2] > 5) {
            Serial.println("Invalid time. Please dial use the 24h-format: hh:mm");
            alarmValid = false;
          } else {
            alarmValid = true;
          }
        }
        if (alarmDigit > 3) {
          alarmDigit = 0;
        }
      }
      // Confirm the alarm by placing the handset back on the telephone
      if (idleSwitch.rose()) {
        if (alarmValid) {
          alarmHours = alarmTime[0] * 10 + alarmTime[1];
          alarmMinutes = alarmTime[2] * 10 + alarmTime[3];
          alarmActive = true;
          alarmDigit = 0;
          Serial.print("Alarm set at: ");
          Serial.print(alarmHours);
          Serial.print(":");
          Serial.println(alarmMinutes);
          Serial.println("Mode 0");
          mode = 0;
        } else {
          Serial.println("Mode 0");
          alarmActive = false;
          mode = 0;
        }
      }
      break;

    // Alarm ringing
    case 10:
      number = 0;
      // Ringtone: ringing, 1 x 0.4 seconds pause
      while (alarmActive) {
        for (int i = 0; i < 20; i++) {
          // Check if the handset is lifted to end the ringing
          idleSwitch.update();
          if (idleSwitch.fell()) {
            alarmActive = false;
            Serial.print("Ringing ended at: ");
            Serial.print(now.hour(), DEC);
            Serial.print(':');
            Serial.print(now.minute(), DEC);
            Serial.print(":");
            Serial.println(now.second(), DEC);
            break;
          }
          digitalWrite(ringerPins[0], i % 2);
          digitalWrite(ringerPins[1], 1 - i % 2);
          delay(40);  // time between hits
        }
        // seconds between ringing
        delay(400);
      }
      // Stop ringing
      digitalWrite(ringerPins[0], LOW);
      digitalWrite(ringerPins[1], LOW);

      if (idleSwitch.rose()) {
        Serial.println("Alarm off");
        Serial.println("Mode 0");
        alarmActive = false;
        mode = 0;
      }
      break;

    // Lights
    case 2:
      Serial.println("Light");
      number = 0;
      Serial.println("Set the brightness of the light");
      dialling();
      // Confirm by placing the handset back on the telephone
      if (idleSwitch.rose()) {
        Serial.println("Brightness set");
        Serial.println("Mode 0");
        mode = 0;  // Change this
      }
      break;

    // Mode 3
    case 3:
      number = 0;
      dialling();
      // Confirm by placing the handset back on the telephone
      if (idleSwitch.rose()) {
        Serial.println("Mode 3 set");
        Serial.println("Mode 0");
        mode = 0;  // Change this
      }
      break;

    // Mode 4
    case 4:
      number = 0;
      dialling();
      // Confirm by placing the handset back on the telephone
      if (idleSwitch.rose()) {
        Serial.println("Mode 4 set");
        Serial.println("Mode 0");
        mode = 0;  // Change this
      }
      break;

    // Mode 5
    case 5:
      number = 0;
      dialling();
      // Confirm by placing the handset back on the telephone
      if (idleSwitch.rose()) {
        Serial.println("Mode 5 set");
        Serial.println("Mode 0");
        mode = 0;  // Change this
      }
      break;

    // Mode 6
    case 6:
      number = 0;
      dialling();
      // Confirm by placing the handset back on the telephone
      if (idleSwitch.rose()) {
        Serial.println("Mode 6 set");
        Serial.println("Mode 0");
        mode = 0;  // Change this
      }
      break;

    // Mode 7
    case 7:
      number = 0;
      dialling();
      // Confirm by placing the handset back on the telephone
      if (idleSwitch.rose()) {
        Serial.println("Mode 7 set");
        Serial.println("Mode 0");
        mode = 0;  // Change this
      }
      break;

    // Mode 8
    case 8:
      number = 0;
      dialling();
      // Confirm by placing the handset back on the telephone
      if (idleSwitch.rose()) {
        Serial.println("Mode 8 set");
        Serial.println("Mode 0");
        mode = 0;  // Change this
      }
      break;

    // Mode 9
    case 9:
      number = 0;
      dialling();
      // Confirm by placing the handset back on the telephone
      if (idleSwitch.rose()) {
        Serial.println("Mode 9 set");
        Serial.println("Mode 0");
        mode = 0;  // Change this
      }
      break;

    default:
      mode = 0;
  }
}

void dialling() {
  if (numberSwitch.rose()) {
    pulseCount++;
  }
  // Check whether the dial has returned all the way
  if (dialSwitch.rose()) {
    // The number 0 has 10 pulses
    if (pulseCount == 10) {
      pulseCount = 0;
    }
    number = pulseCount;
    Serial.println(number);
    pulseCount = 0;
  }
}