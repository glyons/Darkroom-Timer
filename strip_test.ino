// Strip Test
void stripValueTest(float t, int mode, int values[],int stops[], int exposeValues[6])
{
  int full []= {0,100,200,300,400,500};
  int halves []= {0,50,100,150,200,250};
  int thirds []= {0,33,66,100,133,166};
  int sixths []={0,16,33,50,66,83};
  int twelveths [] {0,8,16,25,33,42};

  values[0]=t;
  switch (mode)
  {
     case 0:///Sixths strip test calculation
       values[1]=values[0]+t/12;
       values[2]=values[1]+t/12;
       values[3]=values[2]+t/12;
       values[4]=values[3]+t/12;
       values[5]=values[4]+t/12;
       for(int n=0;n<=5;n++)
       {
        stops[n]=thirds[n];
        exposeValues[n]=t/12;
       }
       break;
     case 1:///Sixths strip test calculation
       values[1]=values[0]+t/6;
       values[2]=values[1]+t/6;
       values[3]=values[2]+t/6;
       values[4]=values[3]+t/6;
       values[5]=values[4]+t/6;
       for(int n=0;n<=5;n++)
       {
        stops[n]=thirds[n];
        exposeValues[n]=t/6;
       }
       break;
      case 2:///Thirds strip test calculation
       values[1]=values[0]+t/3;
       values[2]=values[1]+t/3;
       values[3]=values[2]+t/3;
       values[4]=values[3]+t/3*2;
       values[5]=values[4]+t/3*2;
       for(int n=0;n<=5;n++)
      {
        stops[n]=thirds[n];
        exposeValues[n]=t/3;
       }
       break;
      case 3://Halves strip test calculation
       values[1]=values[0]+t/2;
       values[2]=values[1]+t/2;
       values[3]=values[2]+t;
       values[4]=values[3]+t;
       values[5]=values[4]+t*2;
       for(int n=0;n<=5;n++)
       {
        stops[n]=halves[n];
        exposeValues[n]=t/2;
       }
       break;
       case 4://Full Stop strip test calculation
         values[1]=values[0]+t;
         values[2]=values[1]+t*2;
         values[3]=values[2]+t*4;
         values[4]=values[3]+t*8;
         values[5]=values[4]+t*16;
         for(int n=0;n<=5;n++)
         {
            stops[n]=full[n];
            exposeValues[n]=t;
         }
       break;
       exposeValues[0]=t; // Base time
  }
}

void stripTest()//select strip number for stripBuilder(), simple click button 2
{
  clearStripLEDs();
  int values[6]; int fstops[6]; int exposeValues[6];
  stripValueTest(tensSeconds,stepIdx, values, fstops,exposeValues);   
  stripTestMode=true;     
  for(int n=0;n<=5;n++)
  {
    tm.setLED(n+1,1);
    if (n==0)
    {
       sprintf(tempString,"BASE%4d",values[n]);
       displayText(tempString,99,6);
    }
    else
    {
       sprintf(tempString,  " %03d%4d",fstops[n], values[n]);
       displayText(tempString,1,6);
    }
    
    if (!focusLight)
    {
      FStop=fstops[n];
      timerCountdown(exposeValues[n]);//timer with f-stop converted in time + enlargment correction time
      if (FStop < 1000) sprintf(tempString, " %03d End", FStop); //fstop format rule 
      if (FStop >= 1000) sprintf(tempString, "%4d End", FStop); //fstop format rule
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
