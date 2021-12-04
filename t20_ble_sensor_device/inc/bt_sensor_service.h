#ifndef APP_BT_SERVICE_H
#define APP_BT_SERVICE_H

// init temperature sensor
int bt_service_init(void);

// send temperature out of range notification
int bt_service_char_temp_notify(void);

#endif