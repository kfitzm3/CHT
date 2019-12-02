# CHT
City Health Tech Code and Library Repo
Nano Data Collection Setup Guide

Make sure to use the library package in the GitHub. Code is also there.
https://github.com/kfitzm3/CHT/settings/collaboration

Materials:
Battery, DC booster
SD breakout (Adafruit or off brand - different code depending)
Sensor
Arduino Nano

Sensor wired to 5V (red), GND (black), and A4 (white)
SD breakout:
	3V, GND, CS->D10, DI->D11, DO->D12, SCK->D13
Battery -> DC booster -> VIN, GND. MAKE SURE THEY AREN’t BACKWARDS. Red wire from battery must go to IN+ and black wire from battery must go to IN - . OUT+ goes to Vin and OUT- goes to GND.

If ADAFRUIT sd breakout: dataLogger1.ino
If NOT adafruit: datalogSD2

That’s it - lemme know if it doesn’t work
