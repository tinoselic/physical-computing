// Rotary dial and telephone pick-up button

// For handling software debouncing of mechanical switch contacts
#include <Bounce2.h>

// Maximum number of digits that can be dialled (Time: hh:mm = 4), must be static
#define MAXDIGITS 4

// Pins
int idlePin = 2;    // Handset
int dialPin = 10;   // Control of rotary-dial
int numberPin = 9;  // Number dial

// States
int state = 0;

// How many pulses have been detected for the dialled digit
int pulseCount = 0;

// The dialled digit
int digit = 0;

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
  Serial.println("Sketch is running...");
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

  // If the handset is placed on the telephone, the telephone becomes idle (no matter when)
  if (idleSwitch.rose()) {
    state = 0;
    pulseCount = 0;
    Serial.println("Back to idle.");
  }

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
        Serial.println("Handset lifted.");
        state = digit;
        pulseCount = 0;
      }
      break;

    // Setting the alarm
    case 1:
      if (numberSwitch.rose()) {
        pulseCount++;
      }
      // Check whether the dial has returned all the way
      if (dialSwitch.rose()) {
      Serial.println("Setting the alarm...");
        // The digit 0 has 10 pulses
        if (pulseCount == 10) {
          pulseCount = 0;
        }
        digit = pulseCount;
        Serial.println(digit);
        pulseCount = 0;
      }
      break;
  }
}