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


Ultrasonic placement
--------------------
Each Ultrasonic sensor (US in the schema) must be placed above one corner of the tank whose level is monitored. 
The distance between a sensor and the bottom of the empty tank must be 60cm.

Schema:

```
             US-4                         US-3
              |----------------------------|  ^
             /|                           /|  |  
      US-1  / |                   US-2   / |  | 60cm
          |/---------------------------|/  |  |
          |   |                        |   |  |
          |   |________________________|__ |  v
          |  /                         |  /
          | /                          | /
          |/___________________________|/
```

Recap
-----

The devices awakes every 3mn and measures the distance between each sensor and the tank and computes the level based on this information.
The level is a percentage. It represents how filled is the tank to monitor.
The result is sent to the Sigfox backend and on this backend is configured to send the data to our backend (source: https://github.com/toulousecleanup/backend  /  hosted at: http://toulousecleanup.pythonanywhere.com/) 


Device management
-----------------

The devices are managable on the SigFox backend: https://backend.sigfox.com/device/list
Ask Alexis Eskenazi for the login / password.

The frontend: https://toulousecleanup.github.io/toulousecleanup/data is here to see the latest data.
