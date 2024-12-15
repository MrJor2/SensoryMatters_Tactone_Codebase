//Code by Jesse Strijker and Joep van de Berg
//Done for the Semester project of Sensory Matters at the university of Eindhoven
//Includes logic for music manipulation at the hand of a teensy 4.1, and reading people's hand position above a sensor pad

//----------------------------------------------------CONFIGURABLE-VARIABLES--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//

//These variables can be changed to finetune response of the ldr's to coverage
//lowerbound is a global value for all ldr's
//If 1 ldr is acting up you can alter the triggermaps, to make individual ldr's trigger differently
int lowerBound = 400;

int LDRtriggermap1[] = { lowerBound, lowerBound, lowerBound, lowerBound, lowerBound, lowerBound, lowerBound, lowerBound, lowerBound, lowerBound, lowerBound, lowerBound, lowerBound, lowerBound, lowerBound, lowerBound };
int LDRtriggermap2[] = { lowerBound, lowerBound, lowerBound, lowerBound, lowerBound, lowerBound, lowerBound, lowerBound + 100, lowerBound, lowerBound, lowerBound, lowerBound, lowerBound, lowerBound, lowerBound, lowerBound };
int LDRtriggermap3[] = { lowerBound, lowerBound, lowerBound, lowerBound, lowerBound, lowerBound, lowerBound, lowerBound, lowerBound, lowerBound, lowerBound, lowerBound, lowerBound, lowerBound, lowerBound, lowerBound };
int LDRtriggermap4[] = { lowerBound, lowerBound, lowerBound + 100, lowerBound, lowerBound, lowerBound, lowerBound, lowerBound, lowerBound, lowerBound, lowerBound, lowerBound, lowerBound, lowerBound, lowerBound, lowerBound };

//If you hook up a more external powersupply (higher amperage) you can increase this value
//Otherwise it is to remain at this value to not impact the air pumps
int brightness = 120;

// File list of the music files on the SD card that are loaded in. Can be altered if there are different music files.
const char* filelist[8] = {
  "HAPPYRITHEM.WAV", "HAPPYLEAD.WAV", "CALMRITHEM.WAV", "CALMLEAD.WAV", "SADRITHEM.WAV", "SADLEAD.WAV", "STRESSRITHEM.WAV", "STRESSLEAD.WAV"
};


//-------------------------------------------DO-NOT-CHANGE-THESE-VARIABLES----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
// setup audio
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <FastLED.h>

unsigned long caseStartTime = 0;  // Tracks when the current case became active
int lastActiveMultiplexer = -1;   // Tracks the previous active multiplexer
unsigned long caseDuration = 0;   // Duration of the last active case

int ESNumber = 0;
int lastESNumber;
unsigned long currentCaseTime;

#define NUM_LEDS 87
#define LED_PIN 5

// Declare the audio control object for the SGTL5000 audio shield
AudioControlSGTL5000 sgtl5000_1;

// GUItool: begin automatically generated code
AudioPlaySdWav playSdWav2;        
AudioPlaySdWav playSdWav1;         
AudioFilterStateVariable filter1; 
AudioMixer4 mixer1;                
AudioOutputI2S i2s1;               
AudioConnection patchCord1(playSdWav2, 1, filter1, 0);
AudioConnection patchCord2(playSdWav1, 0, mixer1, 0);
AudioConnection patchCord3(filter1, 0, mixer1, 1);
AudioConnection patchCord4(mixer1, 0, i2s1, 0);
AudioConnection patchCord5(mixer1, 0, i2s1, 1);

//init chorus - can be removed
#define chorusDelayLength (16 * AUDIO_BLOCK_SAMPLES)  // Number of samples in each delay line
short delay1line[chorusDelayLength]; // Allocate the delay lines for left and right channels
CRGB leds[NUM_LEDS];                 

// Use these with the Teensy Audio Shield
#define SDCARD_CS_PIN 10
#define SDCARD_MOSI_PIN 7  // Teensy 4 ignores this, uses pin 11
#define SDCARD_SCK_PIN 14  // Teensy 4 ignores this, uses pin 13

// Pump 1, 2, 3, 4
#define enA1 25
#define in2_1 27
#define enA2 24
#define in1_2 30
#define enA3 28
#define in1_3 31
#define enA4 29
#define in1_4 32

// Defining selection pins
const int S0 = 37;
const int S1 = 36;
const int S2 = 35;
const int S3 = 34;

// Analog pin reading the LDR values
const int mux1Pin = 38;
const int mux2Pin = 39;
const int mux3Pin = 40;
const int mux4Pin = 41;

//Variable that keeps track of how much a pad of ldr's is being covered, automatically switches to the most covered pad
float coverage = 0;

const int muxPins[] = { mux1Pin, mux2Pin, mux3Pin, mux4Pin };

// Amount of channels on the multiplexer
const int numChannels = 16;

// Storage for the sensor values
int sensorValues[numChannels];
int sensorValues2[numChannels];
int sensorValues3[numChannels];
int sensorValues4[numChannels];

#define arrSize(x) sizeof(x) / sizeof(x[0])

struct PumpState {
  unsigned long lastSwitchTime;  // Tracks the last time the pump's state switched
  bool pumpState;                // Tracks whether the pump is currently on or off
};

PumpState pumpStates[4];  // One state per pump

const boolean muxChannel[16][4] = {
  { 0, 0, 0, 0 },  //channel 0
  { 1, 0, 0, 0 },  //channel 1
  { 0, 1, 0, 0 },  //channel 2
  { 1, 1, 0, 0 },  //channel 3
  { 0, 0, 1, 0 },  //channel 4
  { 1, 0, 1, 0 },  //channel 5
  { 0, 1, 1, 0 },  //channel 6
  { 1, 1, 1, 0 },  //channel 7
  { 0, 0, 0, 1 },  //channel 8
  { 1, 0, 0, 1 },  //channel 9
  { 0, 1, 0, 1 },  //channel 10
  { 1, 1, 0, 1 },  //channel 11
  { 0, 0, 1, 1 },  //channel 12
  { 1, 0, 1, 1 },  //channel 13
  { 0, 1, 1, 1 },  //channel 14
  { 1, 1, 1, 1 }   //channel 15
};

void setup() {
  Serial.begin(115200);
  AudioMemory(8);
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.5);
  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);

  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);

  if (!(SD.begin(SDCARD_CS_PIN))) {
    while (1) {
      Serial.println("Unable to access the SD card");
      delay(500);
    }
  }

  // Put selection pins as OUTPUT
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);

  digitalWrite(S0, LOW);
  digitalWrite(S1, LOW);
  digitalWrite(S2, LOW);
  digitalWrite(S3, LOW);

  // Set pump control pins as outputs
  pinMode(enA1, OUTPUT);
  pinMode(in2_1, OUTPUT);
  pinMode(enA2, OUTPUT);
  pinMode(in1_2, OUTPUT);
  pinMode(enA3, OUTPUT);
  pinMode(in1_3, OUTPUT);
  pinMode(enA4, OUTPUT);
  pinMode(in1_4, OUTPUT);

  //set up the filter resonance and octave control
  filter1.resonance(5);
  filter1.octaveControl(7);

  delay(2000);
  Serial.println("code has started");
}

void loop() {  //loop
  // Read all 16 channels of the multiplexer
  for (int multiplexer = 0; multiplexer < 4; multiplexer++) {
    for (int channel = 0; channel < numChannels; channel++) {
      int val = writeMux(channel, multiplexer);

      // Read the value of the LDR
      if (multiplexer == 0) {
        sensorValues[channel] = val;
      } else if (multiplexer == 1) {
        sensorValues2[channel] = val;
      } else if (multiplexer == 2) {
        sensorValues3[channel] = val;
      } else if (multiplexer == 3) {
        sensorValues4[channel] = val;
      }
    }
  }

  int counts[4];
  counts[0] = arrayCount(sensorValues, lowerBound, LDRtriggermap1);
  counts[1] = arrayCount(sensorValues2, lowerBound, LDRtriggermap2);
  counts[2] = arrayCount(sensorValues3, lowerBound, LDRtriggermap3);
  counts[3] = arrayCount(sensorValues4, lowerBound, LDRtriggermap4);

  int activeMultiplexer = getMax(counts, arrSize(counts));

  if (activeMultiplexer != -1) {
    coverage = percentage(counts[activeMultiplexer], 16);
  }

  //prints all the sensor values in the serial monitor
  printMultiplexerValues("[multiplexer0]: ", sensorValues);
  printMultiplexerValues("[multiplexer1]: ", sensorValues2);
  printMultiplexerValues("[multiplexer2]: ", sensorValues3);
  printMultiplexerValues("[multiplexer3]: ", sensorValues4);

  //change the soundfiles that are playing
  if (lastESNumber != ESNumber) {

    // stopping the sound playing
    playSdWav1.stop();
    playSdWav2.stop();

    if (ESNumber != -1) {
      // if there is no sound playing, start playing a new sound based on the ESstate
      if (playSdWav1.isPlaying() == false) {
        const char* fileNameRithem = filelist[ESNumber];
        int leadESNumber = ESNumber + 1;
        const char* fileNameLead = filelist[leadESNumber];

        //playing the two files
        playSdWav1.play(fileNameRithem);
        playSdWav2.play(fileNameLead);
      }
    }
  }

  // frequency is mapped to coverage with a min of 100 and max of 5000
  int covFreq = map(coverage, 0, 1, 100, 5000);

  //safety feature to prevent screeching sounds
  if (covFreq <= 10000) {
    filter1.frequency(covFreq);
  }

  // Timer logic to track how long a case is active
  if (activeMultiplexer != lastActiveMultiplexer) {
    // Update for the new active case
    lastActiveMultiplexer = activeMultiplexer;
    caseStartTime = millis();
  }

  currentCaseTime = millis() - caseStartTime;

  lastESNumber = ESNumber;

  // Switch statement with active multiplexer
  switch (activeMultiplexer) {
    case -1:
      {
        deactivateAllPumps();
        Serial.println("No multiplexer activated.");
        idleLED();
        ESNumber = -1;
      }
      break;
    case 0:
      {
        ColorAllLeds(255, 200, 0);
        volumeMap();
        activatePump(0, enA1, in2_1, 255, 2000, 2000);
        printActiveMulti(currentCaseTime, "0", coverage);
        ESNumber = 0;
      }
      break;
    case 1:
      {
        ColorAllLeds(50, 255, 0);
        volumeMap();
        activatePump(1, enA2, in1_2, 255, 5000, 2000);
        printActiveMulti(currentCaseTime, "1", coverage);
        ESNumber = 2;
      }
      break;
    case 2:
      {
        ColorAllLeds(173, 216, 230);
        volumeMap();
        activatePump(2, enA3, in1_3, 255, 1500, 4000);
        printActiveMulti(currentCaseTime, "2", coverage);
        ESNumber = 4;
      }
      break;
    case 3:
      {
        ColorAllLeds(210, 0, 0);
        volumeMap();
        int inf = random(100, 2500);
        int def = random(inf, inf + 500);
        activatePump(3, enA4, in1_4, 255, inf, def);
        printActiveMulti(currentCaseTime, "3", coverage);
        ESNumber = 6;
      }
      break;
  }
  delay(100);
  Serial.println(brightness);
  FastLED.setBrightness(brightness);
  FastLED.show();
}

void printActiveMulti(unsigned long time, char const* Nr, float cov) {
  Serial.print("Multiplexer");
  Serial.print(Nr);
  Serial.print(" is activated for: ");
  Serial.print(time);
  Serial.print(" ms. With a coverage of: ");
  Serial.print(cov);
  Serial.println();
}

// Setting the channels on the multiplexer
void setMuxChannel(int channel) {
  digitalWrite(S0, channel & 0x01);  // Least significant bit
  digitalWrite(S1, channel & 0x02);
  digitalWrite(S2, channel & 0x04);
  digitalWrite(S3, channel & 0x08);  // Most significant bit
}

void printMultiplexerValues(const char* label, int* sensorValues) {
  for (int i = 0; i < numChannels; i++) {
    Serial.print(label);
    Serial.print("LDR[");
    Serial.print(i);
    Serial.print("]: ");
    Serial.print(sensorValues[i]);
    Serial.print("\t");
  }
  Serial.println();
}

int writeMux(int channel, int multiplexer) {
  int controlPin[] = { S0, S1, S2, S3 };

  //loop through the 4 sig
  for (int i = 0; i < 4; i++) {
    digitalWrite(controlPin[i], muxChannel[channel][i]);
  }

  //read the value at the SIG pin
  int value = analogRead(muxPins[multiplexer]);

  return value;
}

//Mechanism that controls duration of pumps turning on and off, this was coded to prevent pads from blowing up with too much air
void activatePump(int pumpIndex, int enPin, int inPin1, int strength, int inflateTime, int deflateTime) {
  deactivateAllPumps();
  // Toggle the pump's state
  unsigned long currentMillis = millis();
  PumpState& pumpState = pumpStates[pumpIndex];  // Get the state for the specific pump

  // Determine the current phase duration
  int currentPhaseTime = pumpState.pumpState ? inflateTime : deflateTime;

  // Check if the current phase duration has elapsed
  if (currentMillis - pumpState.lastSwitchTime >= currentPhaseTime) {
    pumpState.lastSwitchTime = currentMillis;    // Reset the switch time
    pumpState.pumpState = !pumpState.pumpState;  // Toggle the pump's state
  }

  if (pumpState.pumpState) {
    digitalWrite(inPin1, HIGH);
    analogWrite(enPin, strength);  // Turn the pump on with the specified strength
  } else {
    digitalWrite(inPin1, LOW);
    analogWrite(enPin, 0);  // Turn the pump off
  }
}

// Turn off all pumps
void deactivateAllPumps() {
  analogWrite(enA1, 0);
  digitalWrite(in2_1, LOW);
  analogWrite(enA2, 0);
  digitalWrite(in1_2, LOW);
  analogWrite(enA3, 0);
  digitalWrite(in1_3, LOW);
  analogWrite(enA4, 0);
  digitalWrite(in1_4, LOW);
}

int arrayCount(int arr[], int low, int lowLDR[]) {
  int lowerCount2 = 0;

  for (int i = 0; i < 16; i++) {
    if (arr[i] <= lowLDR[i] && arr[i] > 10) {
      lowerCount2++;
    }
  }

  String myString2 = String(lowerCount2);
  int num2 = myString2.toInt();

  return num2;
}

int getMax(int arr[], int size) {
  if (size <= 0) {
    return -1;  // Return -1 if array size is invalid
  }

  int maxIndex = 0;      // Start with the first element as the maximum
  bool allZeros = true;  // Flag to check if all elements are 0

  for (int i = 0; i < size; i++) {
    if (arr[i] != 0) {
      allZeros = false;  // If any element is non-zero, update the flag
    }
    if (arr[i] > arr[maxIndex]) {
      maxIndex = i;  // Update maxIndex if a larger value is found
    }
  }

  // If all elements are 0, return -1
  if (allZeros) {
    return -1;
  }

  return maxIndex;
}

float percentage(float part, float total) {
  float p = 0;
  p = part / total;
  return p;
}

void idleLED() {
  // happy pad - gold
  for (int i = 0; i < 21; i++) {
    leds[i] = CRGB(255, 200, 0);
  }

  // calm pad - light green
  for (int j = 21; j < 44; j++) {
    leds[j] = CRGB(50, 255, 0);
  }

  // sad pad - light blue
  for (int k = 44; k < 65; k++) {
    leds[k] = CRGB(173, 216, 230);
  }

  //stresfull pad - bright red
  for (int l = 65; l < 87; l++) {
    leds[l] = CRGB(210, 0, 0);
  }
}

void ColorAllLeds(int R, int G, int B) {
  for (int j = 0; j < NUM_LEDS; j++) {
    leds[j] = CRGB(R, G, B);
  }
}

void volumeMap() {
  int volume = 0;
  volume = map(currentCaseTime, 0, 5000, 100, 500);
  float vol2 = (float)volume / 1000;
  if (vol2 < 0.5) {
    sgtl5000_1.volume(vol2);
  }
}