// Strip Test
float *stripValueTest(float t, byte mode, float values[],int stops[])
{
  int fullstops []= {0,100,200,300,400,500};
  int halfstops []= {0,50,100,150,200,250};
  int thirdstops []= {0,33,66,100,133,166};
  
  switch (mode)
  {
      case 0: //Off
       break;
      case 1://Half Stops strip test calculation
       values[0]=t;
       values[1]=values[0]+t/2;
       values[2]=values[1]+t/2;
       values[3]=values[2]+t;
       values[4]=values[3]+t;
       values[5]=values[4]+t*2;
       for(int n=0;n++;n<6)
       {
        stops[n]=halfstops[n];
       }
       break;
      case 2:///Third Stops strip test calculation
       values[0]=t;
       values[1]=values[0]+t/3;
       values[2]=values[1]+t/3;
       values[3]=values[2]+t/3;
       values[4]=values[3]+t/3*2;
       values[5]=values[4]+t/3*2;
       for(int n=0;n++;n<6)
       {
        stops[n]=thirdstops[n];
       }
       break;
      case 3://Full Stop strip test calculation
       values[0]=t;
       values[1]=values[0]+t;
       values[2]=values[1]+t*2;
       values[3]=values[2]+t*4;
       values[4]=values[3]+t*8;
       values[5]=values[4]+t*16;
       for(int n=0;n++;n<6)
       {
        stops[n]=fullstops[n];
       }
       break;
  }
  Serial.println(values[0]);
  Serial.println(values[1]);
  Serial.println(values[2]);
  Serial.println(values[3]);
  Serial.println(values[4]);
  Serial.println(values[5]);
  Serial.println("------");
  return values;
}
void stripTestModeSetUp()//Program steps input mode set-up, long hold program button
{
  if (tmButtons==STRIPTEST_MODE_BUTTON) plusminus++;
  stripTestMode = plusminus % 4;//read 2 revolving encoder values
  if(stripTestMode != displayRefreshTracker) //check for display update
  { 
    switch (stripTestMode)
    {
      case 0:
        tm.displayText("   Off  ");
      break;
      case 1:
        tm.displayText("  HALF  ");
      break;
      case 2:
        tm.displayText("  third ");
      break;
      case 3:
        tm.displayText("  Full  ");
       break;
    }
    EEPROM.write(eeStripTestMode, stripTestMode);
  }
  displayRefreshTracker = stripTestMode; //update of display update trigger
  delay(intervalButton); 
}
void stripTest()//select strip number for stripBuilder(), simple click button 2
{
    switch(tmButtons)
    {
      case STRIPTEST_BUTTON:
        plusminus++;
      break;
      case 1:
       uiMode=0;
      break;
    }
  delay(intervalButton);  
  stripID =  plusminus % 6; 
  clearStripLEDs();
  tm.setLED(stripID+1, 1);
  if (stripID != displayRefreshTracker) // refresh display
  {
    if (stripID==0)
    {
      sprintf(tempString, "bASE%4d", tensSeconds);
      displayText(tempString,99,6);
    }
    else
    {
      float values[6]; int stops[6];
      stripValueTest(tensSeconds, stripTestMode, values, stops);
      sprintf(tempString," %03d    ", stops[stripID]);//values[stripID]
      displayText(tempString,1,6);
      Serial.println(stops[stripID]);
       Serial.println(values[stripID]);
    }
  }
 displayRefreshTracker = stripID; //value for display update check
}
