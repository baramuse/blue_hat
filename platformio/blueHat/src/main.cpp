#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define PIN         A1
#define NUMPIXELS   7

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRBW + NEO_KHZ800);
#define DELAYVAL 500

const int SAMPLE_WINDOW_MS = 50; // Sample window width in mS (50 mS = 20Hz)
const int MAX_LEVEL = 700;
unsigned int sample;
unsigned int signalMax;
unsigned int signalMin;
unsigned long startMillis;
unsigned short brightness;
unsigned short min_brightness = 0;
 
void initSoundLevelSample() {
   startMillis = millis();
   signalMax = 0;
   signalMin = MAX_LEVEL;
}

// #define VBATPIN A9
// void checkBatteryVoltage () {
   
   
// float measuredvbat = analogRead(VBATPIN);
// measuredvbat *= 2;    // we divided by 2, so multiply back
// measuredvbat *= 3.3;  // Multiply by 3.3V, our reference voltage
// measuredvbat /= 1024; // convert to voltage
// Serial.print("VBat: " ); Serial.println(measuredvbat);
// }

void checkSoundLevel() {
 if (millis() - startMillis >= SAMPLE_WINDOW_MS) {
      unsigned int peakToPeak = signalMax - signalMin;  // max - min = peak-peak amplitude
      float percent = (peakToPeak) / 700.0;  // convert to volts
      //Serial.println(percent);
      brightness = min_brightness + (256 - min_brightness) * percent;
      Serial.printf("new brightness = %d\n", brightness);
      initSoundLevelSample();
   } else {
      sample = analogRead(A4);
      if (sample < 700)  // toss out spurious readings
      {
         if (sample > signalMax)
         {
            signalMax = sample;  // save just the max levels
         }
         else if (sample < signalMin)
         {
            signalMin = sample;  // save just the min levels
         }
      }
   }
}


void setColor(uint32_t color) {
    pixels.fill(color, 0, NUMPIXELS);
    pixels.show();
}
void setup() 
{
   Serial.begin(9600);
   initSoundLevelSample();
   pixels.begin();
   pixels.setBrightness(min_brightness);
}
 
void loop()
{
  checkSoundLevel();
  setColor(pixels.Color(255,0,0,120));
  pixels.setBrightness(brightness);
}