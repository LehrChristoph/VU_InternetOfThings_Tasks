For this assignment the following was implemented (both in zephyr):

- t20_ble_sensor_device: This is flashed onto the Thingy52. It advertises the temperature over BLE.
- t30_mqtt_gateway: This is flased onto the ESP32. It connects itself with the Thingy52 over BLE and a MQTT broker over WIFI. If a message is published to the MQTT_SUB_TOPIC, it reads the temperature from the Thingy52 and publishes it to the topic MQTT_TOPIC.

The topics are defined in t30_mqtt_gateway/inc/mqtt_config.h

The WIFI credentials should be defined in t30_mqtt_gateway/prj.conf
