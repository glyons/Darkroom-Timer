int fstop2TensSeconds(float FStop) //fStop to time conversion, return time value
{
  float tensValue = 0;
  tensValue = round(10*pow(2,FStop/100)); //f-stop to time (rounded): t=2^(fstop), here time in 1/10th of sec, f-stop in 1/100th
  return tensValue;
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
      EEPROM.write(eeIncrement, timerInc);
      EEPROM.write(eeStepIdx, i+1);
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
        buttonPlusMinusValue += increment;
      break;
      case MINUS_BUTTON:
        buttonPlusMinusValue-= increment;
      break;
    }
  if (buttonPlusMinusValue <= 0) buttonPlusMinusValue = 0;
  if (buttonPlusMinusValue > 1000) buttonPlusMinusValue = 1000;//f-stop upper limit of 10.00 corresponding to ca. 17.07min
  
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
    if (steps > 29) buttonPlusMinusValue = 1000; //F-stop max 10.00
    if (steps <= 0) buttonPlusMinusValue = 0; //F-stop  min 0.00x
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
    if (deltaFStop == 0) //condition if no scale correction
    { 
      if (FStop > 996) //9.96 last f-stop value corresponding to time under 1000 sec
      {
           //Set no comma for times over 999.9 sec
      }
      else
      {
           //Set comma position for times from 0 up to 999.9 sec
      }
    }
    if (deltaFStop != 0) //scale correction on
    {
      if (FStop > 996) //9.96 last f-stop value corresponding to time under 1000 sec
      { 
          //apostrophe, no comma for times over 999.9 sec
      }
      else
      {
          //Set apostrophe and comma position for times from 0 up to 999.9 sec
      }
    }
    if (resumeTime != 0) tensDisplay = resumeTime; //display resume time
    if (FStop < 1000) sprintf(tempString, " %03d%4d", FStop, tensDisplay);
    if (FStop >= 1000) FStop=10; //sprintf(tempString, "%4d%4d", FStop,  tensDisplay); //fstop format rule
    displayText(tempString,1,6);
    // Store the FStop value for next time, the timer starts up from cold.
    EEPROM.write(eeLastFStopValue, FStop & 0xFF); 
    EEPROM.write(eeLastFStopValue + 1, (FStop >> 8) & 0xFF); // 16 bit
  }
  displayRefreshTracker = buttonPlusMinusValue; //value for display update check
}
