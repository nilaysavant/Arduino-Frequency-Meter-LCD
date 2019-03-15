# Arduino Frequency Meter w/ LCD
####  (Using Platform IO and VS-Code)

* A simple **frequency** meter designed using `Arduino Nano (atmega328p)`.
* Can measure frequencies upto `2.6 MHz` of other **Arduinos**( running at *16MHz*).
* Uses two timers `TC1` and `TC2` for measuring the frequencies with a *loop()* sample frequency up to `1000 Hz`.
* Displays the Output on a `LCD` connected via a *i2c Serial to parallel converter module*. 
* **LCD i2c address** : `0x27` .
* Uses code referenced from : *Nick Gammons's Timer/Counter examples*..