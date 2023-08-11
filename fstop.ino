
int fstop2TensSeconds(float FStop) //fStop to time conversion, return time value
{
  return round(10*pow(2,FStop/100)); //f-stop to time (rounded): t=2^(fstop), here time in 1/10th of sec, f-stop in 1/100th
}

void fstopIncrementSetUp() //F-stop increment selector, long hold timer button
{
    if (tmButtons==INCREMENT_BUTTON) plusminus++;
    byte i = plusminus % timerIncrementSize; //Limit values to those available in the presets array
    timerInc = timerIncrement[i]; //increment selection
    stepIdx = i;

    if (i != displayRefreshTracker) {
      clearStripLEDs();
      tm.setLED(i+1, 1);
      sprintf(tempString, "StEP %03d", timerInc);//set right align 0.00 format
      displayText(tempString,5,99);
     }

    displayRefreshTracker = i; //display trigger update
    delay(intervalButton);
}

void fstopSelector()//f-stop and time setting function, single click button 1
{
  int steps; //steps counter
  int tensDisplay; //time value for display
  increment = timerInc; //pre-set increment value
  switch(tmButtons)
  {
    case PLUS_BUTTON:
      if ((buttonPlusMinusValue+increment)<999)
        buttonPlusMinusValue += increment;
    break;
    case MINUS_BUTTON:
      if (( buttonPlusMinusValue-increment)>0)
      buttonPlusMinusValue-= increment;
    break;
  }
  buttonPlusMinusValue = constrain(buttonPlusMinusValue, 0, 1000); //f-stop upper limit of 10.00 corresponding to ca. 17.07min

  if (loadDefault)
  {
    FStop = (int)EEPROM.read(eeLastFStopValue) | ((int)EEPROM.read(eeLastFStopValue + 1) << 8);
    loadDefault=false;
    if (FStop==0) FStop=10;
    buttonPlusMinusValue=FStop;
  }
  else
  {
    FStop = buttonPlusMinusValue % 1001;//Fstop value
  }

  if (increment == 33){ // 1/3rd correction to avoid bad rounding of 3 * 1/3rd = 0.99
    steps = buttonPlusMinusValue / increment;//count how many steps of 33
    int multipliedVal = steps * increment; //F-stop value before correction
    if (steps % 3 == 0 && steps > 0) FStop = multipliedVal + (steps / 3);//full f-stops correction
    if (steps % 3 != 0 && steps <= 29) FStop = multipliedVal + ((steps - steps % 3 ) / 3); //fractions f-stops correction, up to 9.90
    buttonPlusMinusValue = (steps > 29) ? 1000 : ((steps <= 0) ? 0 : buttonPlusMinusValue); //F-stop  min 0.00x
  }

  tensSeconds = fstop2TensSeconds(FStop + deltaFStop); //final tens calculation including scaling factor and lighting delay
  tensDisplay = tensSeconds; //time value for display
  if (resumeTime != 0) tensSeconds = resumeTime; //resume time from timerCountdown() function
  timeMillis = millis() + (tensSeconds * 100); //f-stop conversion to millis for timer countdown

  if (tmButtons == 128 )
  {
    timerCountdown(tensSeconds);//timer with f-stop converted in time + enlargment correction time
    displayRefreshTracker += 1;
  }

  if (tmButtons == 1 ) //cancel after stop
  {
    tm.displayText(" CANCEL  ");//cancel message
    delayTimer(readingDelay);//reading delay
    resumeTime = 0; //canceled, no time to resume
    displayRefreshTracker += 1;
  }

  if (buttonPlusMinusValue != displayRefreshTracker) //check for display refresh
  {
    if (resumeTime != 0) tensDisplay = resumeTime; //display resume time
    sprintf(tempString, " %03d%4d", FStop, tensDisplay);
    displayText(tempString,1,6);
  }
  displayRefreshTracker = buttonPlusMinusValue; //value for display update check
}
