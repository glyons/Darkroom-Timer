
void focusOnOff()
{
  // Toggle relay pin state
  digitalWrite(RELAY_PIN, !digitalRead(RELAY_PIN));

  // Toggle focus light and corresponding LED
  focusLight = !focusLight;
  tm.setLED(FOCUS_LED_PIN, focusLight ? 1 : 0);

  delay(2*debounce);
  uiMode = 0; // Switch back to timer mode
}

void resetFocus()
{
  // If RELAY_PIN is HIGH, reset it to LOW and turn off focus light
  if (digitalRead(RELAY_PIN) == HIGH)
  {
    digitalWrite(RELAY_PIN, LOW);
    tm.setLED(FOCUS_LED_PIN, 0);
    focusLight = false;
  }
}

void bipLow() { tone(TONE_PIN, toneLow, bip); } // Low tone bip
void bipHigh() { tone(TONE_PIN, toneHigh, bip); } // High tone bip

void endTone() // End acoustic signal
{
  bipHigh();
  delay(shortPause);
  bipHigh();
}

void errorTone() // Error acoustic signal
{
  for(int i = 0; i < 3; i++)
  {
    bipLow();
    delay(shortPause);
  }
}

void brightnessInit()
{
    // Set display brightness from EEPROM
    tm.brightness(EEPROM.read(eeBrightness));
}

void brightnessSelector() // uiMode 6, long hold focus button
{
  if (tmButtons == BRIGHTNESS_BUTTON)
  {
    // Cycle through brightness levels
    brightnessValue =  ++plusminus % 8;

    // If brightness value has changed, update display and EEPROM
    if(brightnessValue != displayRefreshTracker)
    {
      tm.brightness(brightnessValue);
      displayRefreshTracker = brightnessValue;
    }
    delay(intervalButton);
  }
}
