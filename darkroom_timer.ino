
// Rewritten for the TM1638 without rotary encoder by Gavin Lyons.
// Based on Monkito darkroom f-stop timer software written and © by Elia Cottier//
#include <TM1638plus.h>
#include <Math.h> //Math library for "round" function
#include <EEPROM.h> //EEPROM library to save set-up values

// GPIO I/O pins on the Arduino or ESP 12 Relay
//pick on any I/O you want.
#define  STROBE_TM 10 //ESPino 14 //Arduino 10
#define  CLOCK_TM 9 //ESPino 13 //Arduino 9
#define  DIO_TM 11 //ESPino 12 //Arduino 11
bool high_freq = false; //default false Arduino Uno,, If using a high freq CPU > ~100 MHZ set to true, i.e ESP32
bool focusLight=false;

#define TONE_PIN 16 //buzzer pin
#define RELAY_PIN 5 //relay board pin
#define FOCUS_LED_PIN 0 //Focus button led pin
#define START_LED_PIN 7 //Start button led pin
#define CORR_LED_PIN 6 //Start button led pin

//TM1638 Buttons
#define BRIGHTNESS_BUTTON 8 
#define STRIPTEST_BUTTON 4
#define STRIPTEST_MODE_BUTTON 5
#define INCREMENT_BUTTON 16
#define SHIFT_BACK_BUTTON 1
#define MINUS_BUTTON 32
#define PLUS_BUTTON 64
#define SHIFT_MINUS_BUTTON 33
#define SHIFT_PLUS_BUTTON 65

//EPROM default valuse storage
  const byte eeBrightness = 0; //eeprom brightness value address
  const byte eeIncrement = 1; //eeprom f-stop selector increment value address
  const byte eeLastFStopValue = 3; //eeprom step program input mode value address
  const byte eeStepIdx = 6; //eeprom step program input mode value address

  byte brightnessValue; 
  uint8_t tmButtons;
  const long intervalButton = 300; // interval to read button (milliseconds)
  byte debounce(10); //general debounce delay, ok for buttons and encoder
  byte uiMode; //mode for main loop mode management
  int displayRefreshTracker; //track encoder change used by all selection functions
  byte bip(100); //bip duration
  int shortTone (500); //short tone duration
  int longTone (1000); //long tone duration
  int toneLow(880); //lower frequency tone (A 4th)
  int toneHigh(1780); //higher frequency tone (A 5th)
  byte shortPause(150); //short tone pause
  int readingDelay = 1000;//1 sec. human reading delay
  char tempString[9]; //TM1638 Display digits with two decimals.

  volatile unsigned int buttonPlueMinusVal; //+/- exposure functions
  int increment; 
  int plusminus; // plusminus button value
  bool loadDefault;
  
//Timer
  byte timerIncrement []= {8,16,33,50,100}; //Preset f-stop fractions increments in hundreds
  char* timerIncrementText []= {"1-12","1-6 ","1-3 ","1-2 "}; //Preset f-stop fractions increments in hundreds
  byte timerIncrementSize = 5;
  byte timerInc; //fstop increment value

  unsigned int FStop; //F-stop value
  unsigned int tensSeconds; //time value in 1/10th of seconds
  unsigned int resumeTime; //Resume time value
  double deltaFStop; //scaling f-stop delta value
  double lengthRatio = 0;
  unsigned long timeMillis;//time in millis
  unsigned long startMillis;//start time
  unsigned long elapsedMillis;//elapsed time
  unsigned long millisInterval = 100;//10 Hz timer display update frequency
  unsigned long time_passed;//elapsed button time 
  int stepIdx;

//Constructor object (GPIO STB , GPIO CLOCK , GPIO DIO, use high freq MCU)
TM1638plus tm(STROBE_TM, CLOCK_TM , DIO_TM, high_freq);
void setup()
{
  Serialinit();
  tm.displayBegin();
//EEPROM
  timerInc = EEPROM.read(eeIncrement);//f-stop buttonPlueMinus increment
  stepIdx = EEPROM.read(eeStepIdx);
  if (stepIdx==0) stepIdx++;
  tm.setLED(stepIdx, 1);
  brightnessValue = EEPROM.read(eeBrightness);
//Values
  FStop = 0;
  deltaFStop = 0;
  timeMillis = 0;
  elapsedMillis = 0;
  buttonPlueMinusVal = 0;
  plusminus=0;
  increment = 0;
  displayRefreshTracker = 10000; //value for displays refresh
  time_passed = millis();
  loadDefault=true;
 //Pin modes
  pinMode(TONE_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT); 
//Display
  brightnessInit();
  bipHigh();//set-up end signal
}

void loop()
{
  uiModes();
}

byte uiModes() //timer mode and related functions
{
  tmButtons = buttonsRead(); //check buttons events
  if (tmButtons != 0) displayRefreshTracker = 10000;//Value to get any display update
 
  if (uiMode==0)
  {
    plusminus=0;
    switch(tmButtons)
    {
      case 0x16:
        uiMode = 1; //Clear buttonPlueMinus before timer mode (uiMode 0)
      break;
      case 0x02:
        uiMode = 4; //Focus light on/off
      break;
      case STRIPTEST_BUTTON:
         uiMode = 2; //Strip Test Mode
      break;
      case BRIGHTNESS_BUTTON:
        uiMode = 99; //Brightness set-up
        break;
      case INCREMENT_BUTTON://F-stop increment set-up
        uiMode = 14; 
      break;
      case STRIPTEST_MODE_BUTTON:
        uiMode=16;
      break;
      case 65:
        uiMode = 18; //Correction set-up
      break;
      case 129:
        uiMode = 19; //Correction reset
      break;
    }
  }
  else
  {
    if(millis()-time_passed > 5000) {
      time_passed = millis(); 
      uiMode=0;
    }
  }
 
  switch (uiMode)
    {
    case 1: //
      buttonPlueMinusVal = 0;
      uiMode = 0; //default mode
    break;
    case 2:
      stripTest();
    break;
    case 3:
      uiMode = 22;
    break;
    case 4:
      focusOnOff(); // Focus Lamp on/off
    break;
    case 12:
      focusOnOff();
    break;
    case 14:
      fstopIncrementSetUp();
    break;
    case 18:
      scaleCalculator();
    break;
    case 19:
      clearCorrection();
    break;
    case 99:
      brightnessSelector();
    break;
    default:
     fstopSelector(); //default mode
    } 
}
// Display Text with decimal points
void displayText(const char *text, int p, int p2) {
  char c, pos=0;
  while ((c = (*text++)) && pos < TM_DISPLAY_SIZE)  {
    if ((p==pos ||p2==pos) && p>0) {
      tm.displayASCIIwDot(pos++, c);
    }  else {
      tm.displayASCII(pos++, c);
    }
  }
}
// Reset LED on the TM1638
void clearStripLEDs() {
  for (uint8_t LEDposition = 1; LEDposition < 7; LEDposition++) {
    tm.setLED(LEDposition, 0);
  }
}

// Read and debounce the buttons from TM1638  every interval 
uint8_t buttonsRead(void)
{
  uint8_t buttons = 0;
  static unsigned long previousMillisButton = 0;  // executed once 
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillisButton >= intervalButton) {
    previousMillisButton = currentMillis;
    buttons = tm.readButtons();
  }
  return buttons;
}

//Function to setup serial called from setup FOR debug
void Serialinit()
{
  Serial.begin(9600);
  Serial.println("F-STOP Timer");
}
