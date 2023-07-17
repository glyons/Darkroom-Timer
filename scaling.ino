void scaleCalculator()//determine f-stop scaling factor from length change proportion, based on 10cm
{
  switch(tmButtons)
    {
      case SHIFT_PLUS_BUTTON:
        lengthRatio ++;
      break;
      case SHIFT_MINUS_BUTTON:
        lengthRatio --;
      break;
      case SHIFT_BACK_BUTTON:
       uiMode=0;
      break;
    }

  deltaFStop = 100*(log10(pow(lengthRatio/100,2))/log10(2)); //scaling factor calculation
  int deltaFStopDisplay = deltaFStop; //factor value for display
  
  tm.setLED(CORR_LED_PIN, 1);
  sprintf(tempString, "SCALE%4d", deltaFStopDisplay);
  displayText(tempString,6,99);
  delay(100);
  Serial.println(deltaFStop);
}
void clearCorrection() //Clear scaling correction value
{
  deltaFStop = 0; //zero correction value
  plusminus = 0;
  tm.setLED(CORR_LED_PIN, 0);
  tm.displayText("SCALE.OFF");
  delayTimer(readingDelay); //delay for message reading
  uiMode = 1; //back to fstopSelector
}
