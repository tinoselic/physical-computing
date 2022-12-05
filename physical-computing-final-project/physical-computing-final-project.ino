// Rotary dial and telephone pick-up button

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

// States
int state = 0;

// How many pulses have been detected for the dialled digit
int pulseCount = 0;

// The dialled digit
int digit = 0;

// Alarm variables
bool alarmActive = false; // State of the alarm (on/off)
int alarmHours;           // hh
int alarmMinutes;         // mm

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
  Serial.println("Dial: 0-Idle, 1-Set Alarm, 2-Light, 3-Mode 3,..., 9-Mode 9");
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
  if (idleSwitch.rose()) {
    Serial.println("Handset placed");
  }
  if (idleSwitch.fell()) {
    Serial.println("Handset lifted");
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

  switch (state) {
    // Idle
    case 0:
      // Time-alarm comparison
      if (now.hour() == alarmHours && now.minute() == alarmMinutes && now.second() == 0) {
        Serial.println("Alarm ringing");
        state = 10;
      }
      if (numberSwitch.rose()) {
        pulseCount++;
      }
      // Check whether the dial has returned all the way
      if (dialSwitch.rose()) {
        // The digit 0 has 10 pulses
        if (pulseCount == 10) {
          pulseCount = 0;
        }
        digit = pulseCount;
        Serial.println(digit);
        pulseCount = 0;
      }
      // Lifting the handset
      if (idleSwitch.fell()) {
        Serial.print("Mode ");
        Serial.println(digit);
        state = digit;
      }
      break;

    // Alarm Mode
    case 1:
      if (numberSwitch.rose()) {
        pulseCount++;
      }
      // Check whether the dial has returned all the way
      if (dialSwitch.rose()) {
        Serial.println("Set your alarm");
        // The digit 0 has 10 pulses
        if (pulseCount == 10) {
          pulseCount = 0;
        }
        digit = pulseCount;
        Serial.println(digit);
        pulseCount = 0;
      }
      // Confirm the alarm by placing the handset back on the telephone
      if (idleSwitch.rose()) {
        alarmActive = true;
        Serial.println("Alarm set");
        state = 0;
      }
      break;

    // Alarm ringing
    case 10:
      // Ringtone: 2 x 0.4 seconds ringing, 1 x 0.2 seconds pause
      int now = millis();
      if (now - lastRingTime > 4000) {
        for (int j = 0; j < 2; j++) {
          for (int i = 0; i < 20; i++) {
            // Check if the handset is lifted to end the ringing
            idleSwitch.update();
            if (idleSwitch.fell()) {
              alarmActive = false;
              j = 2;
              Serial.println("Ringing ended");
              break;
            }
            digitalWrite(ringerPins[0], i % 2);
            digitalWrite(ringerPins[1], 1 - i % 2);
            delay(40);
          }
          // 0.2 seconds off
          delay(200);
        }
        // Stop ringing
        digitalWrite(ringerPins[0], LOW);
        digitalWrite(ringerPins[1], LOW);
        lastRingTime = now;
      }
      if (idleSwitch.rose()) {
        Serial.println("Back to idle");
        alarmActive = false;
        state = 0;
      }
      break;

    // Lights
    case 2:
      if (numberSwitch.rose()) {
        pulseCount++;
      }
      // Check whether the dial has returned all the way
      if (dialSwitch.rose()) {
        Serial.println("Set the brightness of the light");
        // The digit 0 has 10 pulses
        if (pulseCount == 10) {
          pulseCount = 0;
        }
        digit = pulseCount;
        Serial.println(digit);
        pulseCount = 0;
      }
      // Confirm by placing the handset back on the telephone
      if (idleSwitch.rose()) {
        Serial.println("Brightness set");
        Serial.println("Back to idle");
        state = 0;  // Change this
      }
      break;

    // Mode 3
    case 3:
      if (numberSwitch.rose()) {
        pulseCount++;
      }
      // Check whether the dial has returned all the way
      if (dialSwitch.rose()) {
        // The digit 0 has 10 pulses
        if (pulseCount == 10) {
          pulseCount = 0;
        }
        digit = pulseCount;
        Serial.println(digit);
        pulseCount = 0;
      }
      // Confirm by placing the handset back on the telephone
      if (idleSwitch.rose()) {
        Serial.println("Mode 3 set");
        Serial.println("Back to idle");
        state = 0;  // Change this
      }
      break;

    // Mode 4
    case 4:
      if (numberSwitch.rose()) {
        pulseCount++;
      }
      // Check whether the dial has returned all the way
      if (dialSwitch.rose()) {
        // The digit 0 has 10 pulses
        if (pulseCount == 10) {
          pulseCount = 0;
        }
        digit = pulseCount;
        Serial.println(digit);
        pulseCount = 0;
      }
      // Confirm by placing the handset back on the telephone
      if (idleSwitch.rose()) {
        Serial.println("Mode 4 set");
        Serial.println("Back to idle");
        state = 0;  // Change this
      }
      break;

    // Mode 5
    case 5:
      if (numberSwitch.rose()) {
        pulseCount++;
      }
      // Check whether the dial has returned all the way
      if (dialSwitch.rose()) {
        // The digit 0 has 10 pulses
        if (pulseCount == 10) {
          pulseCount = 0;
        }
        digit = pulseCount;
        Serial.println(digit);
        pulseCount = 0;
      }
      // Confirm by placing the handset back on the telephone
      if (idleSwitch.rose()) {
        Serial.println("Mode 5 set");
        Serial.println("Back to idle");
        state = 0;  // Change this
      }
      break;

    // Mode 6
    case 6:
      if (numberSwitch.rose()) {
        pulseCount++;
      }
      // Check whether the dial has returned all the way
      if (dialSwitch.rose()) {
        // The digit 0 has 10 pulses
        if (pulseCount == 10) {
          pulseCount = 0;
        }
        digit = pulseCount;
        Serial.println(digit);
        pulseCount = 0;
      }
      // Confirm by placing the handset back on the telephone
      if (idleSwitch.rose()) {
        Serial.println("Mode 6 set");
        Serial.println("Back to idle");
        state = 0;  // Change this
      }
      break;

    // Mode 7
    case 7:
      if (numberSwitch.rose()) {
        pulseCount++;
      }
      // Check whether the dial has returned all the way
      if (dialSwitch.rose()) {
        // The digit 0 has 10 pulses
        if (pulseCount == 10) {
          pulseCount = 0;
        }
        digit = pulseCount;
        Serial.println(digit);
        pulseCount = 0;
      }
      // Confirm by placing the handset back on the telephone
      if (idleSwitch.rose()) {
        Serial.println("Mode 7 set");
        Serial.println("Back to idle");
        state = 0;  // Change this
      }
      break;

    // Mode 8
    case 8:
      if (numberSwitch.rose()) {
        pulseCount++;
      }
      // Check whether the dial has returned all the way
      if (dialSwitch.rose()) {
        // The digit 0 has 10 pulses
        if (pulseCount == 10) {
          pulseCount = 0;
        }
        digit = pulseCount;
        Serial.println(digit);
        pulseCount = 0;
      }
      // Confirm by placing the handset back on the telephone
      if (idleSwitch.rose()) {
        Serial.println("Mode 8 set");
        Serial.println("Back to idle");
        state = 0;  // Change this
      }
      break;

    // Mode 9
    case 9:
      if (numberSwitch.rose()) {
        pulseCount++;
      }
      // Check whether the dial has returned all the way
      if (dialSwitch.rose()) {
        // The digit 0 has 10 pulses
        if (pulseCount == 10) {
          pulseCount = 0;
        }
        digit = pulseCount;
        Serial.println(digit);
        pulseCount = 0;
      }
      // Confirm by placing the handset back on the telephone
      if (idleSwitch.rose()) {
        Serial.println("Mode 9 set");
        Serial.println("Back to idle");
        state = 0;  // Change this
      }
      break;
  }
}