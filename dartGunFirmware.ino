#include <SevenSeg.h>

#define SOFTWARE_VERSION 1

//------------ Features
//#define USE_FIRING_MODE_SELECT
//#define USE_KILL_COUNTER
#define DEBUG
#define USE_VOLTAGE_METER

//------------ Pin Assignments

//Ammo Counter
#define DART_SENSOR_PIN 2  // select the input pin for the phototransistor (pin 2 or 3 -> int.0 or int.1)
#define DART_SENSOR_INTERRUPT 0 //either 0 or 1 based on the selected DART_SENSOR_PIN
#define MAG_DETECT_PIN 12 // select the pin to detect a new magazine
#define UP  1
#define DOWN 0
#define COUNT_DIRECTION UP //1: count up, 0: count down

//Display
SevenSeg ssd(10,11,4,5,6,7,8); // in order: A,B,C,D,E,F,G
#define NUM_SSD_DIGITS 4 // use all 4 digits together to prevent line contention
int digitPins[NUM_SSD_DIGITS] = {A0,A1,A2,A3}; // in order: D1,D2,D3,D4
#define DECIMAL_POINT_PIN 9 // select the decimal point pin
int ssdTop, ssdBottom;

//Firing Mode
#ifdef USE_FIRING_MODE_SELECT
  #define TRIGGER_PIN 25
#endif

//Kill Counter
#ifdef USE_KILL_COUNTER
  #define KILL_COUNTER_PIN 3  // select the pin to detect a new kill (pin 2 or 3 -> int.0 or int.1)  
  #define KILL_COUNTER_INTERRUPT 1 //either 0 or 1 based on the selected KILL_COUNTER_PIN
#endif

// Voltage Meter
#ifdef USE_VOLTAGE_METER
#define VOLTAGE_MONITOR_PIN A7
#define BATTERY_RATIO 0.00977// map a value from (0, 1024) into (0.0, 9.0)
#endif

//------------  Variables

//Ammo Counter
volatile int ammo = 0;  //variable to count ammo remaning
volatile int magSize = 18;  //variable to hold clip size
volatile int ammoCountDidChange = false;  //variable to detect change in dartSensor
#define MAG_IN  LOW
#define MAG_OUT HIGH
volatile int prevMagState; //variable to track previous magazine state
volatile int currentMagState; //variable to track current magazine state

//Firing Mode
#ifdef USE_FIRING_MODE_SELECT
#endif

//Kill Counter
int killCount = 0;  // variable to count number of kills;
#ifdef USE_KILL_COUNTER  
  volatile int killCountDidChange = false; //variable to detect change in killCount
  long lastKillTime = millis(); 
#endif

// Voltage Meter
#ifdef USE_VOLTAGE_METER
int voltageValue = 0; // adc value
float voltage = 0.0f; // value in volts
#endif

//------------ Timers
#define SSD_TIMER 2 //WARNING: Must manually relabel TIMERx ISR based on value of SSD_TIMER 

//------------ ISRs
//Called when dartSensor sees a dart fly past
void dartSensorISR()
{
    ammoCountDidChange = true;
    Serial.println("dart ISR");
}

#ifdef USE_KILL_COUNTER
//Called when a new kill is detected
void killCounterISR()
{
  killCountDidChange = true;
}
#endif

//WARNING: Must manually relabel TIMERx based on value of SSD_TIMER
ISR(TIMER2_COMPA_vect)
{
  ssd.interruptAction();
}

//------- Helper Functions
/**
 * debounce a pullup input pin (check to see if it stays low)
 * returns TRUE if the pin is actually low, and FALSE if it is not
 */

boolean isLow(int pin)
{
  boolean reading = digitalRead(pin);
  int repetitions = 1; // repeat measurement multiple times to be sure that there's a dart actually for real
  while(repetitions > 0)
  {
    delay(1);
    reading |= digitalRead(pin);
    
    repetitions--;
  }
  
  return !reading; // true if low, false if high
}

/**
 * updtae SSD with current values
 */
void ssdUpdate()
{
  int displayVal = 100 * (killCount % 100) + (ammo % 100);
  
//  char buff[NUM_SSD_DIGITS+1];
//  combinedSSD.toCharArray(buff,sizeof(buff));
//  buff[sizeof(buff)] = '\0';  //null terminate the string;
//  ssd.write(buff);
  ssd.write(displayVal);
  #ifdef USE_VOLTAGE_METER
  ssd.write(displayVal, 3); // write the decimal point
  #endif
  
//  #ifdef DEBUG
//    Serial.print("Set SSD to: ");
//    Serial.println(displayVal);
//  #endif
}

/**
 * A Dart was detected
 */
void dartDetected()
{      
    if( UP == COUNT_DIRECTION )
    {
      ammo++;
    }
    else
    {
      ammo--;
    }
    
    ammo = constrain(ammo,0,magSize);
    
    ssdUpdate();
    #ifdef DEBUG
      Serial.print("Ammo: ");
      Serial.println(ammo);
    #endif
}

/**
 * A new magazine was inserted
 */
void newMagazine()
{
    if( UP == COUNT_DIRECTION )
    {
      ammo = 0;
    }
    else
    {
      ammo = magSize;
    }
    ssdUpdate();
}

/**
 * read the battery voltage
 */
void voltageMeter()
{
  voltageValue = analogRead(VOLTAGE_MONITOR_PIN);
  voltage = (float)voltageValue * BATTERY_RATIO;


  int highDigit = ((int)voltage) % 10;
  int lowDigit = ((int)(10.0 * voltage)) % 10;
  
  // decide which SSD to use:
  #ifndef USE_KILL_COUNTER // kill counter is disabled so use the 'killCount' value digit
  killCount = (10 * highDigit) + lowDigit;
  #endif
  
  
  ssdUpdate();
}


//--------------- Setup/Loop
void setup()
{
  //Configure General
  #ifdef DEBUG
    Serial.begin(115200);
    Serial.print("Starting DartGun Ammo Monitor Mark ");
    Serial.print(SOFTWARE_VERSION);
    Serial.print("...");
    
    #ifdef USE_FIRING_MODE_SELECT
      Serial.println("Using Firing Mode Select...");
    #endif
    
    if (COUNT_DIRECTION == DOWN)
    {
      ammo = magSize;
    }    
    #ifdef USE_KILL_COUNTER
      Serial.println("Using Kill Counter...");
    #endif
    
    Serial.print("Ammo Count Direction: ");
    if( UP == COUNT_DIRECTION )
    {
      Serial.println("INVERTED/UP");
    }
    else
    {
      Serial.println("NORMAL/DOWN");
    }
  #endif
  
  //Configure Ammo Counter Pins
  pinMode(DART_SENSOR_PIN,INPUT);
  pinMode(MAG_DETECT_PIN,INPUT_PULLUP);
  attachInterrupt(DART_SENSOR_INTERRUPT,dartSensorISR,FALLING); // detect the dart entering the barrel
  currentMagState = digitalRead(MAG_DETECT_PIN);
  prevMagState = currentMagState;
  
  
  //Setup display
  ssd.setDigitPins(NUM_SSD_DIGITS,digitPins);
  ssd.setCommonCathode();
  ssd.setDPPin(DECIMAL_POINT_PIN);
  ssd.setTimer(SSD_TIMER);    //WARNING: Must manually relabel TIMERx ISR based on value of SSD_TIMER 
  ssd.startTimer();
  ssdUpdate();
  
  //Setup Firing Mode Selector
  #ifdef USE_FIRING_MODE_SELECT
    pinMode(TRIGGER_PIN,INPUT);
  #endif
  
  //Setup Kill Counter Selector
  #ifdef USE_KILL_COUNTER
    pinMode(KILL_COUNTER_PIN,INPUT_PULLUP);
   // attachInterrupt(KILL_COUNTER_INTERRUPT,killCounterISR,FALLING);
  #endif
  
  // setup voltage monitor
  #ifdef USE_VOLTAGE_METER
  pinMode(VOLTAGE_MONITOR_PIN,INPUT);
  #endif
}

void loop()
{
  //Poll Magazine Detect Pin (For Ammo Count Reset)
  currentMagState = digitalRead(MAG_DETECT_PIN);
  if((prevMagState != currentMagState) && (MAG_IN == currentMagState)) //if magazine is in but was out, reset the ammo count
  {
    newMagazine();
  }
  prevMagState = currentMagState; // remember whether there is a magazine or not

  // count shots fired
  if(true == ammoCountDidChange)
  {
    ammoCountDidChange = false;
    Serial.println("call isLow()");
    if(isLow(DART_SENSOR_PIN))
    {
       Serial.println("Dart"); 
      dartDetected();      
    }
  }

  // kill count
  #ifdef USE_KILL_COUNTER
  if(true == killCountDidChange)
  {    
    if(isLow(KILL_COUNTER_PIN))
    {
      killCount ++;
      ssdUpdate();
      #ifdef DEBUG
        Serial.print("KillCount: " + killCount);
      #endif
    }
//    lastKillTime = currentKill;
    killCountDidChange = false;
  }
  
  #endif
  
  
  #ifdef USE_VOLTAGE_METER
  voltageMeter();
  #endif  
}
