
## sensor_node

Hardware:
* Arduino Uno
* Arduino Ethernet Shield 2
* MQ-5 gas sensor
* MQ-7 gas sensor
* DS18B20 temperature sensor

Firmware:
* Implements functionality to read data from sensors and in case the previous and current data are different, 
they are published to MQTT server and visualized by this [app](https://github.com/t0m4uk1991/HADashboard/blob/master/README.md)
![arduino-image](http://i.imgur.com/rKYyu5K.jpg)
