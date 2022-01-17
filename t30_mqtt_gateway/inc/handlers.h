#ifndef APP_HANDLERS_H
#define APP_HANDLERS_H

// Bluetooth handler
int bt_handler_init(void);
int bt_handler_fetch_data(void);
int bt_handler_set_sampling_interval(unsigned int ms);
double bt_get_temp();

// WIFI handler
void wifi_handler_connect(bool wait_for_connection);

// MQTT Handler
int mqtt_handler_connect(void);
int mqtt_handler_publish(double);
int mqtt_handler_disconnect(void);
void mqtt_set_send(bool *send);

#endif
