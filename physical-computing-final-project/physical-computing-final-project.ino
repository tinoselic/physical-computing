// Rotary dial and telephone pick-up button

// For handling software debouncing of mechanical switch contacts
#include <Bounce2.h>

// Maximum number of digits that can be dialled (Time: hh:mm = 4), must be static
#define MAXDIGITS 4

// Pins
int idlePin = 2;    // Handset
int numberPin = 9;  // Rotary-dial
int dialPin = 10;   // Control of rotary-dial

// States
int state = 0;

// How many pulses have been detected for the dialled digit
int pulseCount = 0;

// Bounce objects
Bounce idleSwitch = Bounce();
Bounce numberSwitch = Bounce();
Bounce dialSwitch = Bounce();

// Ensure that the dialled time is empty
//memset(clock, 0, MAXDIGITS);

void setup() {
  // Open the serial port
  Serial.begin(9600);
  // Declare pin inputs and attach debounce ojects
  pinMode(idlePin, INPUT);
  idleSwitch.attach(idlePin);
  idleSwitch.interval(5);
  pinMode(numberPin, INPUT);
  numberSwitch.attach(numberPin);
  numberSwitch.interval(5);
  pinMode(dialPin, INPUT);
  dialSwitch.attach(dialPin);
  dialSwitch.interval(5);

  //Serial.println("Hello");
}

void loop() {
  // Read the current state of all switches
  //digitalRead(idlePin);
  //digitalRead(Pin);
  //digitalRead(dialPin);
  idleSwitch.update();
  numberSwitch.update();
  dialSwitch.update();

  switch (state) {
    // Idle
    case 0:
      if (idleSwitch.fell()) {
        //Serial.println("Handset lifted.");
        state = 1;
      }
      break;

    // Dial
    case 1:
      if (dialSwitch.rose()) {
        if (numberSwitch.rose()) {
          pulseCount++;
          // The digit 0 has 10 pulses
          if (pulseCount == 10) {
            pulseCount = 0;
          }

          //Serial.print("Pulse Count: ");
        }
        break;
      }
  }

  // If the handset is placed on the telephone, the telephone becomes idle (no matter when)
  if (idleSwitch.rose()) {
    Serial.println(pulseCount);
    state = 0;
    pulseCount = 0;
    //memset(clock, 0, MAXDIGITS);
    //Serial.println("Back to idle.");
  }
}