
// Strip Test
void stripValueTest(float t, int mode, int values[],int stops[], int exposeValues[6])
{
  int full []= {0,100,200,300,400,500};
  int halves []= {0,50,100,150,200,250};
  int thirds []= {0,33,66,100,133,166};
  int sixths []={0,16,33,50,66,83};
  int twelveths [] {0,8,16,25,33,42};
  int *selectedArray;
  float increment;

  values[0]=t;
  switch (mode)
  {
     case 0: ///Twelveths strip test calculation
       selectedArray = twelveths;
       increment = t/12;
       break;
     case 1: ///Sixths strip test calculation
       selectedArray = sixths;
       increment = t/6;
       break;
     case 2: ///Thirds strip test calculation
       selectedArray = thirds;
       increment = t/3;
       break;
     case 3: ///Halves strip test calculation
       selectedArray = halves;
       increment = t/2;
       break;
     case 4: ///Full Stop strip test calculation
       selectedArray = full;
       increment = t;
       break;
  }

  for(int n=1; n<=5; n++)
  {
    values[n] = values[n-1] + increment;
    stops[n] = selectedArray[n];
    exposeValues[n] = increment;
  }

  exposeValues[0] = t; // Base time
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
    sprintf(tempString,(n==0) ? "BASE%4d" : (values[n] < 10000) ? " %03d%4d" : "%4d",values[n]);
    displayText(tempString, (n==0 || values[n] >= 10000) ? 99 : 1, d);
    baseExposure = (n==0);
    if (!focusLight)
    {
      FStop=fstops[n];
      timerCountdown(exposeValues[n]);//timer with f-stop converted in time + enlargment correction time
      sprintf(tempString, (FStop < 1000) ? " %03d End" : "%4d End", FStop); //fstop format rule
      displayText(tempString,1,99);
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
