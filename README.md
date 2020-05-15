# chicken-safe
Automated Chicken door opener for Arduino

This projects provides automation for chicken gate. It uses the sunset and sunrise times to provide accurate time to open and close the door all the year long.
User shall configure its position using coordinates. Delay between door action and sunset/sunrise time is configurable.

Hardware:
* Arduino Nano
* RTC DS1307 (as a first version)
* OLED screen SSD1036 : Lowest power consumption for battery powered application
* 3 buttons
* 6V Motor with gearbox and rotation encoder

Software dependencies:
* Dusk2Dawn 
* Arduino-menu-system
* RTCLib
