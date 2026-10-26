This readme is a overview on the Program intruder_detection.c

### PIN OUT
    GPIOx       - Pin Mapped                       Artix7_35t
    	0       -Keypad Top Line-4   	(Output)    CKIO
		1       -Keypad Top Line-3		(Output)    CKI1
		2       -Keypad Top Line-2		(Output)    CKI2
		3       -Keypad Top Line-1		(Output)    CKI3
		4       -Keypad Bottom Line-4	(Output)    CKI4
		5       -Keypad Bottom Line-3	(Output)    CKI5
		6       -Keypad Bottom Line-2	(Output)    CKI6
		7       -Keypad Bottom Line-1	(Output)    CKI7
		Gnd		-Keypad Bottom Line-5	
		Vcc		-Keypad Bottom Line-6	
 

		8		-PIR Sensor Output Pin	(Input)     CKI8
		16		-LED P1					(Output)    (RGB0- Blue LED)
		19		-LED P2					(Output)    (RGB1- Blue LED)
		11		-Buzzer					(Output)    CKI11
		18		-LED P3					(Output)    (RGB0- Red LED)
		21		-LED P4					(Output)    (RGB0- Red LED)
		28		-Panic Button			(Input)     (Button0)
		24		-Power Switch   		(Input)     (Switch0)
		25		-Deactivation Switch	(Input)     (Switch1)

* CKI - Chip Kit Outer Digital Pin (Arduino Header) 

###  Peripherals Used
* PIR Sensor
    - PIR Sensor detects the presence of humans when activated and raises a trigger for alarm. 
* Panic Button 
    - Panic Button is a manual button to trigger alarm.
* Keypad
    - PmodKYPD keypad is being used in this system for input. (https://reference.digilentinc.com/reference/pmod/pmodkypd/start)
* Buzzer
    - Buzzer is used as a device to sound the alarm incase of intrusion. 
* Power Switch 
    - Power Switch is used to activate the intruder system and the sensors start detecting only after Power Switch is activated.
* Deactivation Switch 
    - When the alarm is triggered, the deactivation switch can be used to deactivate the alarm and the buzzer. 
* LED
    - LED is used for visualization of the system.

### Working of the Program 
#### Setting Up 
* Power Switch needs to be switched to activate the Intruder Detection system.
* Once the Power Switch is set, wait for couple of seconds since PIR sensor needs time initially to calibrate and the LED_P2 is also activated, indicating the status of Power Switch.
* "Intruder System Activated" is displayed when the system is up and running, the PIR Sensor is monitoring for any movement of humans. 
* As soon as PIR picks up any movement, it triggers an Alarm through the buzzer and LED_P1.
* The Alarm keeps ringing till the Deactivation Switch is switched ON. 
* The Panic Button also manually triggers the alarm even if there is no human movement detected.

#### Reset the System
* The system can be reset by deactivated the Deactivation Switch and Power Switch. 
* Then just activating the Power Switch and the system starts detecting for intruders after sensor initialization is complete.



