/****************************************
Example Sound Level Sketch for the 
Adafruit Microphone Amplifier
****************************************/
 
const int SAMPLE_WINDOW_MS = 10; // Sample window width in mS (50 mS = 20Hz)
const int MAX_LEVEL = 700;
unsigned int sample;
unsigned int signalMax;
unsigned int signalMin;
unsigned long startMillis;
 
void setup() 
{
   Serial.begin(9600);
   initSoundLevelSample();
  
}

void initSoundLevelSample() {
   startMillis = millis();
   signalMax = 0;
   signalMin = MAX_LEVEL;
}

void checkSoundLevel() {
 if (millis() - startMillis >= SAMPLE_WINDOW_MS) {
      unsigned int peakToPeak = signalMax - signalMin;  // max - min = peak-peak amplitude
      float percent = (peakToPeak) / 700.0;  // convert to volts
      //Serial.println(percent);
      initSoundLevelSample();
   } else {
      sample = analogRead(A4);
      Serial.println(sample);
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
 
void loop()
{
  checkSoundLevel();
}
