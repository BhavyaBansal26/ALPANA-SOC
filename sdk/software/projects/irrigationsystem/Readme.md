Sample project to build a irrigation system.
Mositure sensor checks for the water content in the soil or medium.
Based on the sensor value, a motor is turned on or off.

In this sample project Artix7 35t board is used. Shakti E class is programmed
in Artix7 35t. 
Moisture sensor is connected to GPIO pin 0 and 3.3v source on the board.
Motor is connected to GPIO pin 7. GPIO provides less than 3.3V on high,
which is not sufficient to drive a motor, hence a level convertor has 
to be used to convert to 5V and also provide higher amps to the motor 
from a power source independent of the power from the board.
When there is sufficient moisture in soil or medium, sensor sends high signal.
Based on the signal state, motor is turned off. When there is no moisture
in the soil, there is no signal from mositure sensor and motor will be turned on.



