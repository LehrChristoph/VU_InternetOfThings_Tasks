#ifndef APP_SENSORS_H
#define APP_SENSORS_H

// init temperature sensor
void sensors_temperature_init(int lower, int upper);

// set trigger threshold
void sensors_temperature_set_window(int lower, int upper);
void sensors_temperature_set_lower_threshold(int lower, int upper);
void sensors_temperature_set_upper_threshold(int lower, int upper);

#endif