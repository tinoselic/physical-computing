// Physical Computing Final Project by Tino Selic

// For handling software debouncing of mechanical switch contacts
#include <Bounce2.h>

// Libraries for RTC
#include <Wire.h>
#include <RTClib.h>

// Libraries for the OLED display
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Define an object of RTC_DS1307 class
RTC_DS1307 RTC;

// Storing the current time in a variable
//DateTime now = RTC.now();

// OLED display width and height in pixels
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

// Number of snowflakes in the animation
#define NUMFLAKES 10

// Bitmap image
#define LOGO_HEIGHT 21
#define LOGO_WIDTH 21
static const unsigned char PROGMEM logo_bmp[] = {
  0x00, 0x88, 0x00, 0x00, 0x50, 0x00, 0x00, 0x20, 0x00, 0x08, 0x20, 0x80, 0x18, 0x70, 0xc0, 0x04,
  0x21, 0x00, 0x03, 0x26, 0x00, 0x03, 0x26, 0x00, 0x80, 0xa8, 0x08, 0x48, 0x70, 0x90, 0x3f, 0xdf,
  0xe0, 0x48, 0x70, 0x90, 0x80, 0xa8, 0x08, 0x03, 0x26, 0x00, 0x03, 0x26, 0x00, 0x04, 0x21, 0x00,
  0x18, 0x70, 0xc0, 0x08, 0x20, 0x80, 0x00, 0x20, 0x00, 0x00, 0x50, 0x00, 0x00, 0x88, 0x00
};

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library.
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET -1        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Maximum number of digits that can be dialled (Time: hh:mm = 4), must be static
#define MAXDIGITS 4

// Pins
int idlePin = 11;    // Handset
int dialPin = 12;    // Control of rotary-dial
int numberPin = 13;  // Number dial
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
  Serial.println(__FILE__);
  Serial.println("Compiled: "__DATE__
                 ", "__TIME__);
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

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;  // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000);  // Pause for 2 seconds
  // Clear the buffer
  display.clearDisplay();
}

void loop() {
  // Read the current state of all switches
  idleSwitch.update();
  dialSwitch.update();
  numberSwitch.update();

  // Print out the state of the handset
  if (idleSwitch.rose()) {
    Serial.println("Handset lifted");
  }
  if (idleSwitch.fell()) {
    Serial.println("Handset placed");
  }

  switch (mode) {
    // Idle
    case 0:
      // Display
      if (dialSwitch.read() == HIGH) {
        drawHome();
      }
      // Time-alarm comparison
      if (alarmActive && RTC.now().hour() == alarmHours && RTC.now().minute() == alarmMinutes && RTC.now().second() == 0) {
        Serial.println("Alarm ringing");
        mode = 10;
      }
      dialling();
      // Switch to alarm mode by lifting the handset
      if (idleSwitch.rose()) {
        Serial.println("Alarm Mode");
        mode = 1;
      }
      break;

    // Alarm Mode
    case 1:
      number = 0;
      // Display
      if (dialSwitch.read() == HIGH) {
        drawAlarm();
      }
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

        alarmTime[alarmDigit] = number;
        Serial.println("Set alarm in 24h-format: ");
        Serial.println(alarmTime[alarmDigit]);
        alarmDigit++;
        if (alarmTime[0] > 2 || alarmTime[0] == 2 && alarmTime[1] > 3 || alarmTime[2] > 5) {
          Serial.println("Invalid time. Please use the 24h-format: hh:mm");
          display.clearDisplay();
          display.setTextSize(1);
          display.setTextColor(SSD1306_WHITE);
          display.setCursor(0, 0);
          display.println("ERROR: INVALID TIME");
          display.println("Please use the");
          display.println("24-hour notation:");
          display.println("hh:mm");
          display.display();
          alarmDigit = 0;
          alarmTime[0] = 0;
          alarmTime[1] = 0;
          alarmTime[2] = 0;
          alarmTime[3] = 0;
          alarmValid = false;
          delay(5000);
          display.clearDisplay();
        } else {
          alarmValid = true;
        }
        if (alarmDigit > 3) {
          alarmDigit = 0;
        }
      }
      // Confirm the alarm by placing the handset back on the telephone
      if (idleSwitch.fell()) {
        if (alarmValid) {
          alarmHours = alarmTime[0] * 10 + alarmTime[1];
          alarmMinutes = alarmTime[2] * 10 + alarmTime[3];
          alarmActive = true;
          alarmDigit = 0;
          Serial.print("Alarm set at: ");
          Serial.print(alarmHours);
          Serial.print(":");
          Serial.println(alarmMinutes);
          Serial.println("Idle");
          mode = 0;
        } else {
          Serial.println("Idle");
          alarmActive = false;
          alarmDigit = 0;
          alarmTime[0] = 0;
          alarmTime[1] = 0;
          alarmTime[2] = 0;
          alarmTime[3] = 0;
          mode = 0;
        }
      }
      break;

    // Alarm ringing
    case 10:
      // Display
      if (dialSwitch.read() == HIGH) {
        drawHome();
      }
      number = 0;
      // Ringtone: ringing, 1 x 0.4 seconds pause
      while (alarmActive) {
        for (int i = 0; i < 20; i++) {
          // Check if the handset is lifted to end the ringing
          idleSwitch.update();
          if (idleSwitch.rose()) {
            alarmActive = false;
            Serial.print("Ringing ended at: ");
            Serial.print(RTC.now().hour(), DEC);
            Serial.print(':');
            Serial.print(RTC.now().minute(), DEC);
            Serial.print(":");
            Serial.println(RTC.now().second(), DEC);
            break;
          }
          digitalWrite(ringerPins[0], i % 2);
          digitalWrite(ringerPins[1], 1 - i % 2);
          delay(40);  // time between hits
        }
        // seconds between ringing
        delay(1000);
      }
      // Stop ringing
      digitalWrite(ringerPins[0], LOW);
      digitalWrite(ringerPins[1], LOW);

      if (idleSwitch.fell()) {
        Serial.println("Alarm off");
        Serial.println("Idle");
        alarmActive = false;
        mode = 0;
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

void drawHome(void) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  // Display date and time
  display.print("Date:  ");
  if (RTC.now().day() < 10) {
    display.print("0");
  }
  display.print(RTC.now().day(), DEC);
  display.print('/');
  if (RTC.now().month() < 10) {
    display.print("0");
  }
  display.print(RTC.now().month(), DEC);
  display.print('/');
  display.print(RTC.now().year(), DEC);
  display.println();
  display.print("Time:  ");
  if (RTC.now().hour() < 10) {
    display.print("0");
  }
  display.print(RTC.now().hour(), DEC);
  display.print(':');
  if (RTC.now().minute() < 10) {
    display.print("0");
  }
  display.print(RTC.now().minute(), DEC);
  display.print(':');
  if (RTC.now().second() < 10) {
    display.print("0");
  }
  display.print(RTC.now().second(), DEC);
  display.println();

  // Alarm on/off
  display.print("Alarm: ");
  if (alarmActive) {
    if (alarmHours < 10) {
      display.print("0");
    }
    display.print(alarmHours);
    display.print(":");
    if (alarmMinutes < 10) {
      display.print("0");
    }
    display.print(alarmMinutes);
    display.println();
  } else {
    display.print("off");
    display.println();
  }

  // Mode
  display.print("Dial:  ");
  display.print(number);
  display.println();

  display.display();
}

void drawAlarm(void) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(37, 8);

  display.print(alarmTime[0]);
  display.print(alarmTime[1]);
  display.print(":");
  display.print(alarmTime[2]);
  display.print(alarmTime[3]);

  display.display();
}