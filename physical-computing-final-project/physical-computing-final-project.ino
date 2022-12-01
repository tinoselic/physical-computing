// Rotary dial and telephone pick-up button

// For handling software debouncing of mechanical switch contacts
#include <Bounce2.h>

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

// Time interval between ringing; ringtone
unsigned long lastRingTime;

// Bounce objects
Bounce idleSwitch = Bounce();
Bounce dialSwitch = Bounce();
Bounce numberSwitch = Bounce();

// Ensure that the dialled time is empty
//memset(clock, 0, MAXDIGITS);

void setup() {
  // Open the serial port
  Serial.begin(9600);
  // Print out test
  Serial.println("Dial: 0-Idle, 1-Set Alarm");
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

  //Serial.println("Hello");
}

void loop() {
  // Read the current state of all switches
  idleSwitch.update();
  dialSwitch.update();
  numberSwitch.update();
  /*
  // If the handset is placed on the telephone, the telephone becomes idle (no matter when)
  if (idleSwitch.rose()) {
    state = 0;
    pulseCount = 0;
    Serial.println("Back to idle.");
  }
  */
  switch (state) {
    // Idle
    case 0:
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
        Serial.println("Handset lifted");
        state = digit;
        pulseCount = 0;
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
        Serial.println("Alarm set");
        state = 10;
      }
      break;

    // Alarm set
    case 10:
      delay(1000);  // Time comparison goes here
      Serial.println("Ringing");
      state = 100;
      break;

      // Alarm ringing
    case 100:
      // Ringtone: 2 x 0.4 seconds ringing, 1 x 0.2 seconds pause
      int now = millis();
      if (now - lastRingTime > 4000) {
        for (int j = 0; j < 2; j++) {
          for (int i = 0; i < 20; i++) {
            // Check if the handset is lifted to end the ringing
            idleSwitch.update();
            if (idleSwitch.fell()) {
              j = 2;
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
        state = 0;
      }
      break;
  }
}