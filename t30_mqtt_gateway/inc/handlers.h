#ifndef APP_HANDLERS_H
#define APP_HANDLERS_H

// Bluetooth handler
int bt_handler_init(void);

// WIFI handler 
void wifi_handler_connect(bool wait_for_connection);

#endif