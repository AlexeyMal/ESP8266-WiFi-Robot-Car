# ESP8266 WiFi Robot Car
 ESP8266 WiFi Robot Car

 The car can be controlled by the App "ESP8266 WiFi Robot Car":
 
 https://play.google.com/store/apps/details?id=com.bluino.esp8266wifirobotcar
 
 Download this app to see the modules and example wiring diagrams inside the app.
 
 My script combines 4 modi in a single firmware:
 - remote control
 - obstacle avoidance
 - follow
 - line tracking
 
 The mode switching is done in a loop by pressing "beep" button in the app.
 
 ATTENTION: the pins differ from the original wiring diagram in the app.
 Check .ino for the pin numbers!
 
 If you flash via USB, first disconnect IRCenterPin (central line tracking sensor) from RX pin, otherwise you get no target connection in Arduino IDE!
 
 My Hardware:
 - NodeMcu v2 ESP8266
 - esp-12e-motor-shield
 - HC-04 ultrasonic module (powered from Vin, Echo line connected via voltage divider 5V->3.3V to ESP)
 - Active buzzer + resistor (ESP gives max 12 mA!)
 - LED + resistor (ESP gives max 12 mA!)
 - 3 x line tracking sensors (3.3 V powerded)
 - 4 x AA 1.2V rechargable batteries
 - 2WD smart car chassis
 