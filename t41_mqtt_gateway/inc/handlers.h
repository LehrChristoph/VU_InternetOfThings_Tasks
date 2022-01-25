#ifndef APP_HANDLERS_H
#define APP_HANDLERS_H

#include <stdio.h>
#include <zephyr.h>

typedef struct temp_value
{
    double temperature;
    uint64_t timestamp;
}temp_value_t;

// Bluetooth handler
int bt_handler_init(void);
int bt_handler_fetch_data(void);
int bt_handler_set_sampling_interval(unsigned int ms);
temp_value_t* bt_get_temp();

// WIFI handler
void wifi_handler_connect(bool wait_for_connection);

// MQTT Handler
int mqtt_handler_connect(void);
int mqtt_handler_publish(temp_value_t*);
int mqtt_handler_disconnect(void);
int mqtt_handler_keep_alive(void);
void mqtt_set_send(bool *send);

#endif
