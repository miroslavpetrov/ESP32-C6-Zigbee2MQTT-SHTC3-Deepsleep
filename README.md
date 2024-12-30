# ESP32-C6-Zigbee2MQTT-SHTC3-Deepsleep
Simple low-power deep sleep example for reading Sensirion SHTC3 sensor and sending the data over Zigbee to Home Assistant using XIAO-C6.

The sketch is tested on Xiao-C6 and Zigbee2MQTT.
Arduino-ESP32 core 3.1.0 was used.
The sensor is powered from D3 pin to achieve low power consumption.
Average consumption during deep sleep is 14uA @ 4.2V.
Code execution takes around 2.6s and consumes on average around 60mA @ 4.2V.
