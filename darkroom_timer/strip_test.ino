
// Strip Test
void stripValueTest(float t, int mode, int values[],int stops[], int exposeValues[6])
{
  int full []= {0,100,200,300,400,500};
  int halves []= {0,50,100,150,200,250};
  int thirds []= {0,33,66,100,133,166};
  int sixths []= {0,16,33,50,66,83};
  int twelveths [] {0,8,16,25,33,42};
  int *selectedArray;
  double increment;
  const double baseTime = t;

  double fraction;
  values[0]=baseTime;
  switch (mode)
  {
     case 0: ///Twelveths strip test calculation
       selectedArray = twelveths;
       fraction = 12;
       break;
     case 1: ///Sixths strip test calculation
       selectedArray = sixths;
       fraction = 6;
       break;
     case 2: ///Thirds strip test calculation
       selectedArray = thirds;
       fraction = 3;
       break;
     case 3: ///Halves strip test calculation
       selectedArray = halves;
       fraction = 2;
       break;
     case 4: ///Full Stop strip test calculation
       selectedArray = full;
       fraction = 1;
       break;
  }

  for(int n=0; n<=5; n++)
  {
    values[n] = t;
    stops[n] = selectedArray[n];
    exposeValues[n] = increment ;
    increment = (t*pow(2, (1/fraction))) - t;
    t = t*pow(2, (1/fraction));
  }

  exposeValues[0] = baseTime; // Base time
}

void stripTest()//select strip number for stripBuilder(), simple click button 2
{
  clearStripLEDs();
  int values[6]; int fstops[6]; int exposeValues[6];
  stripValueTest(tensSeconds,stepIdx, values, fstops,exposeValues);   
  stripTestMode=true;
  int d=6;     
  for(int n=0;n<=5;n++)
  {
    tm.setLED(n+1,1);
    if (n==0)
    {
       sprintf(tempString,"BASE%4d",values[n]);
       displayText(tempString,99,6);
       baseExposure=true;
    }
    else
    {
      if (values[n] < 10000) d=6;
      if (values[n] >= 10000) d=99;
      sprintf(tempString,  " %03d%4d",fstops[n], values[n]);
      displayText(tempString,1,d);
      baseExposure=false;
    }
    
    if (!focusLight)
    {
      FStop=fstops[n];
      timerCountdown(exposeValues[n]);//timer with f-stop converted in time + enlargment correction time
      if (FStop < 1000) 
      {
        sprintf(tempString, " %03d End", FStop); //fstop format rule 
        if (n==0) displayText("BASE End",99,99); 
          else displayText(tempString,1,99);
      }
      if (FStop >= 1000) 
      {
        sprintf(tempString, "%4d End", FStop); //fstop format rule
        displayText(tempString,99,99);
      }
      bipHigh();
    }
    displayRefreshTracker += 1;
    if (!stripTestMode) break;
    delay(1000);
  }
  stripTestMode=false;
  FStop=fstops[0];
  clearStripLEDs();
  tm.setLED(stepIdx, 1); // restore LED to step state
}
