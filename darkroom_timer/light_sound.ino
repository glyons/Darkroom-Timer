
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

// Handler for uiMode 4: manage focus toggle and a stopwatch that displays while focus is ON
void focusModeLoop()
{
  // Toggle focus / stopwatch when user presses the Focus button
  if (tmButtons == FOCUS_BUTTON)
  {
    if (!focusLight)
    {
      // Turn focus light ON and start stopwatch at 0.0
      digitalWrite(RELAY_PIN, HIGH);
      focusLight = true;
      tm.setLED(FOCUS_LED_PIN, 1);
      stopwatchActive = true;
      stopwatchStartMillis = millis();
    }
    else
    {
      // Turn focus light OFF, stop and reset stopwatch, return to normal UI
      digitalWrite(RELAY_PIN, LOW);
      focusLight = false;
      tm.setLED(FOCUS_LED_PIN, 0);
      stopwatchActive = false;
      stopwatchStartMillis = 0;
      uiMode = 0; // return to default UI
    }
    delay(2*debounce);
  }

  // Update stopwatch display when active (show seconds with one decimal, same placement as other timers)
  if (stopwatchActive)
  {
    unsigned long now = millis();
    unsigned long elapsed = now - stopwatchStartMillis;
    unsigned int tenths = elapsed / 100; // tenths of a second
    if (tenths > 9999) tenths = 9999; // cap to four digits
    // Align tenths into the right-most 4 digits, decimal point at position 6 (same as timers)
    sprintf(tempString, "    %4d", tenths);
    displayText(tempString, 99, 6);
  }
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
