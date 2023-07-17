void focusOnOff() //Focus light on and off
{
  //tm.reset();
  digitalWrite(RELAY_PIN, !digitalRead(RELAY_PIN));
  if (focusLight)
  {
   tm.setLED(FOCUS_LED_PIN,0);
  }
  else
  {
   tm.setLED(FOCUS_LED_PIN,1);
  }
   delay(2*debounce); 
  focusLight=!focusLight;
  
  uiMode = 0; //back to timer mode
}
void resetFocus()
{ 
 if (digitalRead(RELAY_PIN == HIGH))
  {
    digitalWrite(RELAY_PIN, LOW);
    tm.setLED(FOCUS_LED_PIN, 0);
  }
}
void bipLow()//low tone bip
{tone(TONE_PIN,toneLow,bip);}
void bipHigh()//high tone bip
{tone(TONE_PIN,toneHigh,bip);}
void endTone()//bip bip end acoustic signal
{
  bipHigh();
  delay(shortPause);
  bipHigh();
}
void errorTone()//bip bip bip error acoustic signal
{
  bipLow();
  delay(shortPause);
  bipLow();
  delay(shortPause);
  bipLow();
}

void brightnessInit()
{
    byte n = EEPROM.read(eeBrightness);//displays brightness
    tm.brightness(n);
}
void brightnessSelector()//uiMode 6, long hold focus button
{
  if (tmButtons==BRIGHTNESS_BUTTON) plusminus++;
  brightnessValue =  plusminus % 8;//read 2 revolving encoder values
  if(brightnessValue != displayRefreshTracker) //check for display update
  { 
    tm.brightness(brightnessValue); //set displays brightness function
    EEPROM.write(eeBrightness, brightnessValue);
  }
  displayRefreshTracker = brightnessValue; //update of display update trigger
  delay(intervalButton);  
}
