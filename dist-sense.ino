#include <Tiny4kOLED.h>
#include <U8g2lib.h>
#include "pitches.h"

#define PIN_ECHO A2
#define PIN_TRIG A3

#define PIN_SDA A4
#define PIN_SCL A5

#define PIN_R 5
#define PIN_G 6
#define PIN_B 9

#define PIN_bz1 8

U8X8_SSD1306_128X64_NONAME_HW_I2C display(U8X8_PIN_NONE);

unsigned long previousMillis;
const unsigned long interval = 40;

float newStatus = 50.0;        
float filteredStatus = 50.0;

float x = -0.5;  


void setup() {

  // Set up all pin locations
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  pinMode(PIN_R, OUTPUT);
  pinMode(PIN_G, OUTPUT);
  pinMode(PIN_B, OUTPUT);
  pinMode(PIN_bz1, OUTPUT);


  // Setting the screen up
  display.begin();
  display.setPowerSave(0);
  display.setFont(u8x8_font_pxplusibmcgathin_f);
}

void checkSensor(const int d)
{
  long unsigned currentMillis = millis();

  if(d < 50) {
    newStatus = 100; // Red (close)
  } else if(d >= 50 && d < 200) {
    newStatus = 50; // Green (near) 
  } else {
    newStatus = 0; // Blue (far away)
  }

  if(currentMillis - previousMillis >= interval)
  {

    previousMillis = currentMillis;

    const float pct = 1.8;  
    filteredStatus = (( 1.0 - (pct / 100.0)) * filteredStatus) + (pct / 100.0 * newStatus);  

    float r, g, b;     // Values 0 to 1
    float f;           // Frequency of pulses

    if(filteredStatus < 50.0) {
      // RGB LED changes
      r = 0.0;
      g = filteredStatus / 50.0;
      b = 1.0 - g;
      f = 0.5 + (g / 2);

    } else {
      // RGB LED changes
      r = (filteredStatus - 50.0) / 50.0;
      g = 1.0 - r;
      b = 0.0;
      f = 1.0 + (2 * r);  
      
    }

    float y = expf( -50.0 * squaref(x)); // Amplitude of the RGB values

    x += f * float( interval) / 1000.0;  // divide interval by 1000 because it is in milliseconds
    if( x >= 0.5)
      x -= 1.0;

    int pwmR = (int) round( 1.0 + (r * y * 254.0));
    int pwmG = (int) round( 1.0 + (g * y * 254.0));
    int pwmB = (int) round( 1.0 + (b * y * 254.0));

    bool shouldBeep = false;
    int beepFreq = 0;
    int beepDuration = 0;

    if (newStatus == 50) {
      shouldBeep = true;
      beepFreq = NOTE_C5;
      beepDuration = int(abs(x) * 100000); // Use abs(x) if you want positive time
    } else if (newStatus == 100) {
      shouldBeep = true;
      beepFreq = NOTE_DS5;
      beepDuration = int(abs(x) * 1000000000);
    }

    analogWrite(PIN_R, pwmR);
    analogWrite(PIN_G, pwmG);
    analogWrite(PIN_B, pwmB);

    if (shouldBeep && beepDuration > 0) {
      tone(PIN_bz1, beepFreq, beepDuration); // Play beep for a set duration
    } else {
      noTone(PIN_bz1); // Explicitly turn off buzzer when not needed
    }
  }
}

void loop() {
  // Start a new measurement:
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);

  // Read the result:
  const int dur = pulseIn(PIN_ECHO, HIGH) / 58;

  checkSensor(dur);

  display.setCursor(1, 3);
  display.print("Distance: ");
  display.print(dur);
  display.print("cm");

}
