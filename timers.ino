////////////////////////////////////////***********-START OF TIMERS-************////////////////////////////////////////
void delayTimer(int delayTime) //replace "delay()" function
{
  unsigned long m = millis(); //local millis reference for delay between zones
  while (millis() < m + delayTime) {}; //time to allow mask manipulation between strip zones
}
void timerCountdown(int timeCounter)//Main timer countdown, time in 1/10th displayed with 3rd digit decimal
{ 
  tm.brightness(0);
  elapsedMillis = 0;//reset elapsed time
  unsigned long currentMillis = 0;//start timestamp
  resumeTime = 0;//clear resume time
  resetFocus();//Check for focus button light off
  digitalWrite(RELAY_PIN, HIGH);//enlarger light ON
  tm.setLED(START_LED_PIN, 1);//Start button led ON, direct voltage from Arduino
  startMillis = millis();//start timestamp
  for (int x = timeCounter;x >= 0; x--) //1/10th display countdown update loop
  {
    while (millis() - currentMillis < millisInterval) {} //do nothing until reach interval length
    unsigned long rawMillis = millis(); //timestamp
    elapsedMillis = rawMillis - startMillis; //track elapsed time
    currentMillis = rawMillis - elapsedMillis % 10; //substract time deviation from current timestamp

    if (x % 10 == 0) bipHigh(); //tone every second
    tmButtons = buttonsRead(); //check buttons events

    if(tmButtons==128 ) //stop to resume
    {
      resumeTime = x;//set resume time
      x=0;//end timer loop
    }
    if (tmButtons==1) //Cancel countdown
    {
       resumeTime = 0;//time reset
       x=0;//end of timer loop
       tm.displayText(" CANCEL  ");
       delayTimer(readingDelay);//reading delay
       uiMode = 0;//back to fstop selector mode without reset
    }
    timeCounter--; //set elapsed tens
    if (FStop < 1000) sprintf(tempString, " %03d%4d", FStop, timeCounter ); //fstop format rule 
    if (FStop >= 1000) sprintf(tempString, "%4d%4d", FStop,  timeCounter); //fstop format rule
    displayText(tempString,1,6);
    yield(); // ESP32 
  } //Time elapsed, end of loop
  digitalWrite(RELAY_PIN, LOW);//enlarger light OFF
  tm.setLED(START_LED_PIN, 0);//Start button led OFF 
  tm.brightness(brightnessValue);
}
