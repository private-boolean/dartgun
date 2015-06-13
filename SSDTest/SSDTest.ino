#include <SevenSeg.h>

// in order: A,B,C,D,E,F,G
SevenSeg disp(2,3,4,5,6,7,8);

// use all 4 digits together to prevent line contention
const int numOfDigits = 4;

// in order: D1,D2,D3,D4
int digitPins[numOfDigits] = {13,12,11,10};

void setup()
{
  disp.setDigitPins(numOfDigits,digitPins);
  disp.setCommonCathode();
  
  //Using interrupts
  disp.setTimer(2);
  disp.startTimer();
  
}

void loop()
{
  int count = 0;
  
  for(int count = 0; count < 9999; count++)
  {
    disp.write(count);
    delay(1000);
  }
}

ISR(TIMER2_COMPA_vect)
{
  disp.interruptAction();
}
