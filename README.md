The project repository contains the source files used in building a multi-node wireless sensor network built using the ESP32 devkits.

The sensor nodes A and B acquire the environmental sensor readings and transmit the same to a cluster head node. The cluster head node is connected over a wired network with the base station and publishes all the acquired data over serial COM port.

![image](https://github.com/anr2311/Multi-Node-Wireless-Sensor-Network/assets/72514473/9e2793e5-ce3b-44f4-ae68-af1d3dff8c55)

The sensor node A comprises an ESP32, a BMP280 and DHT11 sensors:

![image](https://github.com/anr2311/Multi-Node-Wireless-Sensor-Network/assets/72514473/c838fae6-eee0-47aa-bf6b-1d202825f100)

The sensor node B comprises an ESP32, an HC-SR04 ultrasonic sensor, a MH-series light sensor and a KY-038 sound sensor:

![image](https://github.com/anr2311/Multi-Node-Wireless-Sensor-Network/assets/72514473/29259159-eb0b-473c-bad7-20aef6d9e581)

The cluster head node comprises an ESP32, a passive infrared sensor (PIR) and a OLED display:

![image](https://github.com/anr2311/Multi-Node-Wireless-Sensor-Network/assets/72514473/ca29e1c4-7cdd-46f3-a7d0-3c3386d84e8c)

The cluster head node communicates with the base station over a wired network (serial COM port) and the base station further transmits the data to a ThingSpeak server (data logging) and a flask web application:

![image](https://github.com/anr2311/Multi-Node-Wireless-Sensor-Network/assets/72514473/a5b01fb5-c8cd-44fd-90d0-115d769868cf)
