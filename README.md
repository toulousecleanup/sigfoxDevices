sigfoxDevices
=============

This is the source code for all the sigfox conencted devices.

These devices are composed of:
* an Arduino MKRFox1200 
* 4 Ultrasonic sensors (HC-SR04)

Pinout
------

* ultrasonic1:  trigPin: **6**  /  echoPin: **10**
* ultrasonic2:  trigPin: **7**  /  echoPin: **11**
* ultrasonic3:  trigPin: **8**  /  echoPin: **12**
* ultrasonic4:  trigPin: **9**  /  echoPin: **13**


About Ultrasonic Sensors placement
----------------------------------
Each Ultrasonic sensor (US in the schema) must be placed above one corner of the tank whose level is monitored. 

The system is designed so that the distance between a sensor and the bottom of the empty tank is about 60cm.

Schema:

```
                            US-4                          US-3
                              /----------------------------/  ^
                             /                            /   |
                       US-1 /                       US-2 /    | 
                           /----------------------------/     |  DIST_US_BOTTOM (~60cm):
                                                              |      distance between US and 
                                                              |      the bottom of the tank
                              |____________________________|  |
                             /|                           /|  | 
                            / |                          / |  |
                       ^  |/___________________________|/  |  |
  FULL_TANK_DIST       |  |   |                        |   |  |
        (~30cm):       |  |   |________________________|__ |  v
  distance between     |  |  /                         |  /
  the bottom of the    |  | /      TANK                | /
  tank and full tank   v  |/___________________________|/
```

What if you need to change some parameters to meet your plateform design ?
--------------------------------------------------------------------------
But if your plateform design requires :
  * a smaller / bigger distance between US and the bottom of the tank (DIST_US_BOTTOM on the schema)
  * a smaller / bigger distance between US and full tank (FULL_TANK_DIST on the schema)

You can change the values of the variables in sigfox_device/sigfox_device.ino:
  * DIST_US_BOTTOM (in cm in the code)
  * FULL_TANK_DIST (in cm in the code)

To change it, edit:
```
#define DIST_US_BOTTOM 60
#define FULL_TANK_DIST DIST_US_BOTTOM / 2
```
For example DIST_US_BOTTOM=100cm and FULL_TANK_DIST=40cm needs:
```
#define DIST_US_BOTTOM 100
#define FULL_TANK_DIST 40
```
Then, just recompile and upload the code to the card using the Arduino software.

Code compilation and upload
---------------------------
  1- You need the software Arduino >= 1.8.3 (https://www.arduino.cc/en/Main/Software).

  2- Plug the arduino via an USB cable to your computer.

  3- Open the file sigfox_device/sigfox_device.ino with Arduino 1.8.x.

  4- Follow the document https://www.arduino.cc/en/Guide/MKRFox1200 -> Section *Use your Arduino MKRFOX1200 on the Arduino Desktop IDE* to be able to reprog the Arduino. It explains everything you need to know for communicating / uploading a code to the Arduino.

  5- Once the Arduino is reprog, you can unplug it from the PC and power it via the battery.

**Note**: More information on the board Arduino MKRFOX1200 can be found at https://www.arduino.cc/en/Main.ArduinoBoardMKRFox1200

Code Explanation
----------------

The devices awakes every 20mn and measures the distance between each sensor and the tank and computes the level based on this information.

**Note**: SigFox allows to send < 120 messages a day so 20mn between each message is a security

The level is a percentage. It represents how filled is the tank to monitor.

The result is sent to the Sigfox backend and on this backend is configured to send the data to our backend:
  * backend source code : https://github.com/toulousecleanup/backend  
  * backend hosted at : http://toulousecleanup.pythonanywhere.com/

**More details on the level computation**

Pseudo-code:
```
Every 20mn, on wakeup, 
  * For every US:
    * Make a distance measure
    * Compute the measure 'weight_factor':
      * The smallest the distance is (the more the tank is full), the biggest is the 'weight_factor'
      * 'weight_factor' is 1 when tank is empty
      * 'weight_factor' is bounded to 60
      * This 'weight_factor' is to make sure that an US saying 'tank is full' has more weight in the final decision that an US saying 'tank is empty'.
    * Compute the sensor 'weighed measured level' that is the sensor 'measured level' multiplied by 'weight_factor'
  * Then make the average of *sensor weighed measured level* of all 4 US and apply the saturation: level is always between 0% and 100%.
  * Send this information to the SigFox backend.
  * Go back to sleep for 20mn.
```
If you want more information about  *sensor weighed measured level* and *weight_factor* computation, read the code.

Device management
-----------------

The devices are managable on the SigFox backend: https://backend.sigfox.com/device/list
Ask Alexis Eskenazi for the login / password.

The frontend: https://toulousecleanup.github.io/toulousecleanup/data is here to see the latest data.
