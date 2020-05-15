# chicken-safe
Automated Chicken door opener for Arduino

This project provides automation for chicken gate. It uses the sunset and sunrise times to provide accurate time to open and close the door all the year long.
User shall configure its position using coordinates. Delay between door action and sunset/sunrise time is configurable.

Hardware:
* Arduino Nano
* RTC DS1307 (as a first version)
* OLED screen SSD1036 I2C : Lowest power consumption for battery powered application
* 3 buttons
* 6V Motor with gearbox and rotation encoder
* A waterproof box

Software dependencies:
* Dusk2Dawn for sunset/sunrise information
* Arduino-menu-system
* RTCLib
* U8x8 display library


TODO:
* Handle motor rotation encoder
  * Up and down position should be configurable and stored in eeprom
  * Manual up and down steps through menu
  * First version uses timer 
* Measure battery level to detect low battery using ADC
* Measure current through the motor to detect overload or blocked gate
* Add flashing led for warnings
* Add schematics
* Add BOM
