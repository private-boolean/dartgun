#dartgun
#=======

Software for the ZombieLARP dart counter mod

Please see http://imgur.com/a/vOcmn for hardware setup.

This code is provided for convenience only and no warranty is provided for it whatsoever. You may use it however you like.

This code has support for 3 monitoring options. Choose between 1 or 2 (3 is always displayed)

1. #define USE_VOLTAGE_METER - display battery voltage
2. #define USE_KILL_COUNTER - increment kill count with a push button
3. (default) display shots fired 
      #define COUNT_DIRECTION UP|DOWN - either count shots up (useful if you have multiple magazines of different sizes) or count down if you are only using magazines of a fixed size.

Make sure that you have the SevenSeg library installed in your Arduino/Libraries folder. (http://playground.arduino.cc/Main/SevenSeg)
